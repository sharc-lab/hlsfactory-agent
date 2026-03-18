#!/usr/bin/env python3
"""Analyze CUDA source files and extract kernel metadata for LLM-based HLS conversion.

Scans CUDA repositories and produces a JSON report with:
  - Kernel names, parameters, and signatures
  - Thread/block dimension usage
  - Shared memory arrays
  - Algorithm pattern hints
  - Source file locations

Usage:
    python cuda_analyze.py /path/to/cuda/repo
    python cuda_analyze.py /path/to/cuda/repo -o report.json
"""

import argparse
import json
import re
import sys
from pathlib import Path

_AXES = ('x', 'y', 'z')

SKIP_LIBRARIES = [
    "cublas", "cufft", "cusparse", "curand", "nccl", "thrust", "cub/",
]

SKIP_INCLUDE_RE = re.compile(
    r'#\s*include\s*[<"](?:cublas|cufft|cusparse|curand|nccl|thrust|cub/)',
    re.IGNORECASE,
)


def _detect_used_dimensions(source: str) -> dict[str, set[str]]:
    dims: dict[str, set[str]] = {'thread': set(), 'block': set()}
    for axis in _AXES:
        if re.search(rf'\bthreadIdx\.{axis}\b', source):
            dims['thread'].add(axis)
        if re.search(rf'\bblockIdx\.{axis}\b', source):
            dims['block'].add(axis)
    return dims


def _find_kernel_names(source: str) -> list[str]:
    return re.findall(r'__global__\s+\w[\w\s\*]*\s+(\w+)\s*\(', source)


def _parse_kernel_params(source: str, kernel_name: str) -> list[dict]:
    pat = re.compile(
        rf'(?:__global__\s+)[\w\s\*]*\b{re.escape(kernel_name)}\s*\(([^)]*)\)',
        re.DOTALL,
    )
    m = pat.search(source)
    if not m:
        return []
    param_str = m.group(1)
    # Strip inline comments (// ... until end of line)
    param_str = re.sub(r'//[^\n]*', '', param_str)
    # Strip block comments (/* ... */)
    param_str = re.sub(r'/\*.*?\*/', '', param_str, flags=re.DOTALL)
    # Collapse whitespace
    param_str = re.sub(r'\s+', ' ', param_str).strip()

    params = []
    for p in param_str.split(','):
        p = p.strip()
        if not p:
            continue
        p_clean = re.sub(r'\b(__restrict__|__restrict)\b', '', p).strip()
        is_ptr = '*' in p_clean
        p_clean = re.sub(r'\s+', ' ', p_clean)
        tokens = p_clean.replace('*', ' * ').split()
        if not tokens:
            continue
        name = tokens[-1]
        type_str = ' '.join(tokens[:-1])
        params.append({'type': type_str, 'name': name, 'is_pointer': is_ptr})
    return params


def _detect_shared_arrays(source: str) -> list[str]:
    return re.findall(r'\b__shared__\s+[\w\s\*]+\s+(\w+)\s*\[', source)


def _has_skip_library(source: str) -> str | None:
    lower = source.lower()
    for lib in SKIP_LIBRARIES:
        if lib.lower() in lower:
            return lib
    if SKIP_INCLUDE_RE.search(source):
        return "CUDA library include"
    return None


def _extract_kernel_source(source: str, kernel_name: str) -> str | None:
    """Extract the full source code of a kernel function."""
    pat = re.compile(
        rf'(__global__\s+[\w\s\*]*\b{re.escape(kernel_name)}\s*\([^)]*\)\s*\{{)',
        re.DOTALL,
    )
    m = pat.search(source)
    if not m:
        return None
    brace_pos = source.index('{', m.start())
    depth = 0
    for i in range(brace_pos, len(source)):
        if source[i] == '{':
            depth += 1
        elif source[i] == '}':
            depth -= 1
            if depth == 0:
                return source[m.start():i + 1]
    return None


def _detect_algorithm_patterns(source: str) -> list[str]:
    """Detect common algorithm patterns in CUDA code."""
    patterns = []
    if re.search(r'__shared__', source):
        patterns.append('shared_memory')
    if re.search(r'atomicAdd|atomicMin|atomicMax|atomicCAS', source):
        patterns.append('atomic_operations')
    if re.search(r'__syncthreads', source):
        patterns.append('thread_synchronization')
    if re.search(r'texture|tex1D|tex2D|cudaBindTexture', source):
        patterns.append('texture_memory')
    if re.search(r'float[234]\b|double[234]\b|make_float[234]', source):
        patterns.append('vector_types')
    if re.search(r'cooperative_groups|cg::', source):
        patterns.append('cooperative_groups')
    if re.search(r'\[.*[+-]\s*1\s*\]', source):
        patterns.append('stencil_access')
    if re.search(r'for.*>>=\s*1|for.*\/=\s*2', source):
        patterns.append('reduction')
    return patterns


def _discover_projects(src: Path) -> list[tuple[Path, str]]:
    """Return list of (directory, project_name) pairs."""
    if src.is_file():
        return [(src.parent, src.stem)]

    subdirs_with_cu: list[tuple[Path, str]] = []
    has_direct_cu = bool(list(src.glob('*.cu')))

    for child in sorted(src.iterdir()):
        if not child.is_dir():
            continue
        if list(child.glob('*.cu')):
            name = child.name
            for suffix in ('-cuda', '-sycl', '_cuda', '.cuda'):
                if name.endswith(suffix):
                    name = name[:-len(suffix)]
                    break
            subdirs_with_cu.append((child, name))

    if subdirs_with_cu:
        return subdirs_with_cu

    if has_direct_cu:
        return [(src, src.name)]

    # Recurse one more level
    for child in sorted(src.iterdir()):
        if child.is_dir():
            for grandchild in sorted(child.iterdir()):
                if grandchild.is_dir() and list(grandchild.glob('*.cu')):
                    name = grandchild.name
                    for suffix in ('-cuda', '-sycl', '_cuda', '.cuda'):
                        if name.endswith(suffix):
                            name = name[:-len(suffix)]
                            break
                    subdirs_with_cu.append((grandchild, name))
    return subdirs_with_cu


def analyze_project(proj_dir: Path, proj_name: str) -> dict:
    """Analyze a CUDA project directory and return metadata."""
    cu_files = sorted(proj_dir.glob('*.cu'))
    h_files = sorted(proj_dir.glob('*.h'))

    if not cu_files:
        return {'name': proj_name, 'skip_reason': 'no .cu files found'}

    all_source = ''
    for f in cu_files + h_files:
        try:
            all_source += f.read_text(errors='replace') + '\n'
        except Exception:
            pass

    lib = _has_skip_library(all_source)
    if lib:
        return {'name': proj_name, 'skip_reason': f'uses {lib}'}

    kernel_names = _find_kernel_names(all_source)
    if not kernel_names:
        return {'name': proj_name, 'skip_reason': 'no __global__ kernel found'}

    used_dims = _detect_used_dimensions(all_source)
    shared_arrays = _detect_shared_arrays(all_source)
    algo_patterns = _detect_algorithm_patterns(all_source)

    kernels = []
    for kn in kernel_names:
        params = _parse_kernel_params(all_source, kn)
        kernel_source = _extract_kernel_source(all_source, kn)
        kernels.append({
            'name': kn,
            'params': params,
            'source_lines': len(kernel_source.splitlines()) if kernel_source else 0,
        })

    return {
        'name': proj_name,
        'directory': str(proj_dir),
        'source_files': [f.name for f in cu_files],
        'header_files': [f.name for f in h_files],
        'kernels': kernels,
        'dimensions': {k: sorted(v) for k, v in used_dims.items()},
        'shared_arrays': shared_arrays,
        'algorithm_patterns': algo_patterns,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description='Analyze CUDA source files for HLS conversion',
    )
    parser.add_argument('src', type=Path, help='CUDA source directory or file')
    parser.add_argument(
        '-o', '--output', type=Path, default=None,
        help='Output JSON file (default: stdout)',
    )
    parser.add_argument('--limit', type=int, default=0, help='Max projects to analyze')
    parser.add_argument('--filter', type=str, default='', help='Comma-separated name filters')
    args = parser.parse_args(argv)

    src: Path = args.src
    projects = _discover_projects(src)

    if not projects:
        print(f"No CUDA source files found in {src}", file=sys.stderr)
        return 1

    name_filters = [f.strip() for f in args.filter.split(',') if f.strip()]
    if name_filters:
        projects = [
            (d, n) for d, n in projects
            if any(f in n or f in d.name for f in name_filters)
        ]

    if args.limit > 0:
        projects = projects[:args.limit]

    results = []
    for proj_dir, proj_name in projects:
        result = analyze_project(proj_dir, proj_name)
        results.append(result)

    report = {
        'total_projects': len(projects),
        'analyzable': len([r for r in results if 'skip_reason' not in r]),
        'skipped': len([r for r in results if 'skip_reason' in r]),
        'projects': results,
    }

    output_text = json.dumps(report, indent=2)

    if args.output:
        args.output.write_text(output_text, encoding='utf-8')
        print(f"Analysis written to {args.output}", file=sys.stderr)
    else:
        print(output_text)

    return 0


if __name__ == '__main__':
    sys.exit(main())
