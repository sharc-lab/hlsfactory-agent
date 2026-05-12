#!/usr/bin/env python3
"""Convert CUDA kernels into HLS-compatible C++ source files.

Accepts any of:
  - A single .cu file
  - A directory containing .cu files (treated as one project)
  - A directory whose subdirectories contain .cu files (each = one project)
  - HeCBench-style *-cuda/ layout (auto-detected)

Usage:
    python cuda2hls.py kernel.cu -o ./hls_out
    python cuda2hls.py my_project/ -o ./hls_out
    python cuda2hls.py /path/to/HeCBench/src -o ./hls_out [--limit 50] [--filter stencil,fft]
"""

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Optional

# Axes we care about
_AXES = ('x', 'y', 'z')


# ---------------------------------------------------------------------------
# Libraries that indicate a benchmark cannot be mechanically converted
# ---------------------------------------------------------------------------
SKIP_LIBRARIES = [
    "cublas", "cufft", "cusparse", "curand", "nccl", "thrust", "cub/",
    "cuBLAS", "cuFFT", "cuSPARSE", "cuRAND", "NCCL",
]

SKIP_INCLUDE_RE = re.compile(
    r'#\s*include\s*[<"](?:cublas|cufft|cusparse|curand|nccl|thrust|cub/)',
    re.IGNORECASE,
)


# ---------------------------------------------------------------------------
# Detection helpers (run on original CUDA source before transforms)
# ---------------------------------------------------------------------------

def _detect_used_dimensions(source: str) -> dict[str, set[str]]:
    """Scan *source* for which threadIdx/blockIdx axes are actually used.

    Returns e.g. ``{'thread': {'x', 'y'}, 'block': {'x'}}``.
    """
    dims: dict[str, set[str]] = {'thread': set(), 'block': set()}
    for axis in _AXES:
        if re.search(rf'\bthreadIdx\.{axis}\b', source):
            dims['thread'].add(axis)
        if re.search(rf'\bblockIdx\.{axis}\b', source):
            dims['block'].add(axis)
    return dims


def _parse_kernel_params(source: str, kernel_name: str) -> list[tuple[str, str, bool]]:
    """Parse a kernel's parameter list from the *original* CUDA source.

    Returns list of ``(type_str, param_name, is_pointer)`` tuples.
    """
    # Match the full signature (possibly multi-line)
    pat = re.compile(
        rf'(?:__global__\s+)[\w\s\*]*\b{re.escape(kernel_name)}\s*\(([^)]*)\)',
        re.DOTALL,
    )
    m = pat.search(source)
    if not m:
        return []

    param_str = m.group(1)
    params: list[tuple[str, str, bool]] = []
    for p in param_str.split(','):
        p = p.strip()
        if not p:
            continue
        # Remove __restrict__ and const for parsing
        p_clean = re.sub(r'\b(__restrict__|__restrict)\b', '', p).strip()
        # Identify pointer: contains '*'
        is_ptr = '*' in p_clean
        # Split into type and name
        p_clean = re.sub(r'\s+', ' ', p_clean)
        # The name is the last token (after removing *)
        tokens = p_clean.replace('*', ' * ').split()
        if not tokens:
            continue
        name = tokens[-1]
        type_str = ' '.join(tokens[:-1])
        params.append((type_str, name, is_ptr))
    return params


def _detect_shared_arrays(source: str) -> list[str]:
    """Find variable names declared with ``__shared__``."""
    # Matches: __shared__ type name[...] or extern __shared__ type name[]
    pat = re.compile(r'\b__shared__\s+[\w\s\*]+\s+(\w+)\s*\[')
    return pat.findall(source)


# ---------------------------------------------------------------------------
# Regex-based CUDA → HLS transformation rules
# ---------------------------------------------------------------------------

def _build_transforms() -> list[tuple[re.Pattern, str]]:
    """Return (compiled_regex, replacement) pairs applied in order."""
    return [
        # Remove CUDA includes
        (re.compile(r'#\s*include\s*[<"]cuda[^>"]*[>"].*\n?'), ''),
        (re.compile(r'#\s*include\s*[<"]cooperative_groups[^>"]*[>"].*\n?'), ''),
        (re.compile(r'#\s*include\s*[<"]device_launch_parameters\.h[>"].*\n?'), ''),
        (re.compile(r'#\s*include\s*[<"]helper_cuda[^>"]*[>"].*\n?'), ''),
        (re.compile(r'#\s*include\s*[<"]helper_functions[^>"]*[>"].*\n?'), ''),
        (re.compile(r'#\s*include\s*[<"]helper_timer[^>"]*[>"].*\n?'), ''),
        # Remove unavailable external library includes
        (re.compile(r'#\s*include\s*[<"]omp\.h[>"].*\n?'), ''),
        (re.compile(r'#\s*include\s*[<"]avilib\.h[>"].*\n?'), ''),
        (re.compile(r'#\s*include\s*[<"]avimod\.h[>"].*\n?'), ''),

        # Remove #include "*.cu" (Rodinia-style cross-includes already merged)
        (re.compile(r'#\s*include\s*"[^"]*\.cu".*\n?'), ''),

        # Remove cooperative_groups usage
        (re.compile(r'^.*\bcg::.*$', re.MULTILINE), ''),
        (re.compile(r'^.*cooperative_groups.*$', re.MULTILINE), ''),

        # __forceinline__ → inline
        (re.compile(r'\b__forceinline__\b'), 'inline'),

        # Remove qualifiers (keep surrounding code intact)
        (re.compile(r'\b__global__\s*'), ''),
        (re.compile(r'\b__device__\s*'), ''),
        (re.compile(r'\b__host__\s*'), ''),
        (re.compile(r'\b__shared__\s+'), ''),
        (re.compile(r'\b__constant__\s+'), ''),
        (re.compile(r'\b__restrict__\b'), ''),

        # extern __shared__ TYPE name[] → TYPE name[4096]
        # (already stripped __shared__ above, so match extern ... name[])
        (re.compile(r'\bextern\s+([\w\s]+)\s+(\w+)\s*\[\s*\]'), r'\1 \2[4096]'),

        # __syncthreads() → remove entirely (sequential loop-nest doesn't need barriers)
        (re.compile(r'\s*\b__syncthreads\s*\(\s*\)\s*;[^\n]*'), ''),

        # atomicAdd(&x, val) → x += val
        (re.compile(r'\batomicAdd\s*\(\s*&(\w+(?:\[[\w\+\-\*\s]+\])*)\s*,\s*([^)]+)\)'),
         r'(\1 += \2)'),
        # atomicAdd(ptr, val) → *ptr += val
        (re.compile(r'\batomicAdd\s*\(\s*(\w+(?:\[[\w\+\-\*\s]+\])*)\s*,\s*([^)]+)\)'),
         r'(*\1 += \2)'),

        # atomicMin(&x, val) → x = min(x, val)
        (re.compile(r'\batomicMin\s*\(\s*&(\w+(?:\[[\w\+\-\*\s]+\])*)\s*,\s*([^)]+)\)'),
         r'(\1 = min(\1, \2))'),
        (re.compile(r'\batomicMin\s*\(\s*(\w+(?:\[[\w\+\-\*\s]+\])*)\s*,\s*([^)]+)\)'),
         r'(*\1 = min(*\1, \2))'),

        # atomicMax(&x, val) → x = max(x, val)
        (re.compile(r'\batomicMax\s*\(\s*&(\w+(?:\[[\w\+\-\*\s]+\])*)\s*,\s*([^)]+)\)'),
         r'(\1 = max(\1, \2))'),
        (re.compile(r'\batomicMax\s*\(\s*(\w+(?:\[[\w\+\-\*\s]+\])*)\s*,\s*([^)]+)\)'),
         r'(*\1 = max(*\1, \2))'),

        # atomicExch, atomicCAS – best-effort removal
        (re.compile(r'\batomicExch\s*\(\s*&(\w+(?:\[[\w\+\-\*\s]+\])*)\s*,\s*([^)]+)\)'),
         r'(\1 = \2)'),
        (re.compile(r'\batomicExch\s*\(\s*(\w+(?:\[[\w\+\-\*\s]+\])*)\s*,\s*([^)]+)\)'),
         r'(*\1 = \2)'),

        # Warp intrinsics
        (re.compile(r'\b__ballot_sync\s*\([^)]*\)'), '0'),
        (re.compile(r'\b__popc\s*\('), '__builtin_popcount('),
        (re.compile(r'\b__shfl_down_sync\s*\([^)]*\)'), '0'),
        (re.compile(r'\b__shfl_sync\s*\([^)]*\)'), '0'),
        (re.compile(r'\b__shfl_xor_sync\s*\([^)]*\)'), '0'),

        # threadIdx / blockIdx → loop variable names
        (re.compile(r'\bthreadIdx\.x\b'), '_tid_x'),
        (re.compile(r'\bthreadIdx\.y\b'), '_tid_y'),
        (re.compile(r'\bthreadIdx\.z\b'), '_tid_z'),
        (re.compile(r'\bblockIdx\.x\b'), '_bid_x'),
        (re.compile(r'\bblockIdx\.y\b'), '_bid_y'),
        (re.compile(r'\bblockIdx\.z\b'), '_bid_z'),
        # blockDim / gridDim → compile-time constants
        (re.compile(r'\bblockDim\.x\b'), 'BLOCK_DIM_X'),
        (re.compile(r'\bblockDim\.y\b'), 'BLOCK_DIM_Y'),
        (re.compile(r'\bblockDim\.z\b'), 'BLOCK_DIM_Z'),
        (re.compile(r'\bgridDim\.x\b'), 'GRID_DIM_X'),
        (re.compile(r'\bgridDim\.y\b'), 'GRID_DIM_Y'),
        (re.compile(r'\bgridDim\.z\b'), 'GRID_DIM_Z'),

        # CUDA math type helpers
        (re.compile(r'\bmake_float2\b'), 'float2'),
        (re.compile(r'\bmake_float3\b'), 'float3'),
        (re.compile(r'\bmake_float4\b'), 'float4'),
        (re.compile(r'\bmake_int2\b'), 'int2'),
        (re.compile(r'\bmake_int3\b'), 'int3'),
        (re.compile(r'\bmake_double2\b'), 'double2'),

        # Remove leftover <<<...>>> kernel launch syntax (whole line)
        (re.compile(r'^.*<<<[^>]*>>>.*$', re.MULTILINE), ''),

        # Remove cudaMalloc / cudaFree / cudaMemcpy lines
        (re.compile(r'^.*\bcudaMalloc\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaFree\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaMemcpy\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaMemset\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaDeviceSynchronize\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaGetLastError\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaCheckError\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaEventCreate\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaEventRecord\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaEventSynchronize\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaEventElapsedTime\b.*$', re.MULTILINE), ''),
        (re.compile(r'^.*\bcudaEventDestroy\b.*$', re.MULTILINE), ''),

        # dim3 declarations are fine in C++; leave them
    ]


TRANSFORMS = _build_transforms()


# ---------------------------------------------------------------------------
# Kernel detection / extraction helpers
# ---------------------------------------------------------------------------

_GLOBAL_KERNEL_RE = re.compile(
    r'__global__\s+\w[\w\s\*]*\s+(\w+)\s*\(',
)


def _find_kernel_names(source: str) -> list[str]:
    """Return names of __global__ kernel functions in *source*."""
    return _GLOBAL_KERNEL_RE.findall(source)


_DEVICE_FUNC_RE = re.compile(
    r'__device__\s+[\w\s\*&:]+\s+(\w+)\s*\(',
)


def _find_device_function_names(source: str) -> list[str]:
    """Return names of __device__ helper functions in *source*."""
    return _DEVICE_FUNC_RE.findall(source)


def _has_skip_library(source: str) -> Optional[str]:
    """Return the library name if *source* uses a library we cannot convert."""
    lower = source.lower()
    for lib in SKIP_LIBRARIES:
        if lib.lower() in lower:
            return lib
    if SKIP_INCLUDE_RE.search(source):
        return "CUDA library include"
    return None


def _apply_transforms(source: str) -> str:
    """Apply all regex transformations to *source*."""
    for pattern, repl in TRANSFORMS:
        source = pattern.sub(repl, source)
    # Collapse multiple blank lines
    source = re.sub(r'\n{3,}', '\n\n', source)
    return source


# ---------------------------------------------------------------------------
# Host / kernel separation
# ---------------------------------------------------------------------------

# Pattern to detect a bare return type on its own line, with the function name
# on the next line.  Joining them lets the function-matching regex work.
_SPLIT_SIG_RE = re.compile(
    r'^(\s*'
    r'(?:static\s+|inline\s+|extern\s+|const\s+)*'
    r'(?:void|int|float|double|unsigned|long|short|char|bool|auto|size_t'
    r'|unsigned\s+int|unsigned\s+long|long\s+long)'
    r'(?:\s*\*)*'       # optional pointer stars
    r')\s*\n'           # bare return type, then newline
    r'(\s*\w+\s*\()',   # next line starts with funcname(
    re.MULTILINE,
)


def _normalize_split_signatures(source: str) -> str:
    """Join function signatures where the return type and name are on separate lines."""
    return _SPLIT_SIG_RE.sub(r'\1 \2', source)


def _extract_kernel_and_helpers(
    source: str,
    kernel_names: list[str],
    device_func_names: list[str] | None = None,
) -> tuple[str, str]:
    """Split *source* into (kernel_code, header_code).

    kernel_code: kernel functions + __device__ helpers only
    header_code: structs, typedefs, constants, macros

    Only functions in *kernel_names* or *device_func_names* are included in
    kernel_code.  All other functions (host code) are discarded.
    """
    # Normalize multi-line function signatures so the regex can match them
    source = _normalize_split_signatures(source)

    allowed = set(kernel_names)
    if device_func_names:
        allowed.update(device_func_names)

    lines = source.split('\n')

    header_lines: list[str] = []
    body_lines: list[str] = []

    # Simple brace-counting state machine
    in_function = False
    brace_depth = 0
    current_func_lines: list[str] = []
    current_func_is_allowed = False
    skip_function = False

    i = 0
    while i < len(lines):
        line = lines[i]

        # Detect function start (simplified heuristic: line with '(' and '{' or
        # next line has '{')
        if not in_function:
            # Check if this looks like a function definition
            # (has a return type, name, and opening paren)
            func_match = re.match(
                r'\s*(?:static\s+|inline\s+|void\s+|int\s+|float\s+|double\s+|'
                r'unsigned\s+|long\s+|short\s+|char\s+|bool\s+|auto\s+|'
                r'template\s*<[^>]*>\s*|__attribute__\s*\([^)]*\)\s*)*'
                r'[\w\*\s:&]+\s+(\w+)\s*\(',
                line,
            )

            if func_match and '{' in line:
                fname = func_match.group(1)
                in_function = True
                brace_depth = line.count('{') - line.count('}')
                current_func_is_allowed = fname in allowed
                skip_function = not current_func_is_allowed
                current_func_lines = [line] if current_func_is_allowed else []
                if brace_depth <= 0:
                    in_function = False
                    if current_func_is_allowed:
                        body_lines.extend(current_func_lines)
                        body_lines.append('')
                    skip_function = False
                i += 1
                continue
            elif func_match:
                # Might be a multi-line signature; look ahead for '{'
                peek = i + 1
                sig_lines = [line]
                found_brace = False
                while peek < len(lines) and peek < i + 20:
                    sig_lines.append(lines[peek])
                    if '{' in lines[peek]:
                        found_brace = True
                        break
                    if ';' in lines[peek]:
                        break
                    peek += 1

                if found_brace:
                    fname = func_match.group(1)
                    current_func_is_allowed = fname in allowed
                    skip_function = not current_func_is_allowed
                    in_function = True
                    brace_depth = 0
                    current_func_lines = list(sig_lines) if current_func_is_allowed else []
                    for sl in sig_lines:
                        brace_depth += sl.count('{') - sl.count('}')
                    if brace_depth <= 0:
                        in_function = False
                        if current_func_is_allowed:
                            body_lines.extend(current_func_lines)
                            body_lines.append('')
                        skip_function = False
                    i = peek + 1
                    continue

            # Not inside a function — classify as header material
            # (structs, typedefs, macros, constants, includes, blank lines)
            header_lines.append(line)
            i += 1
            continue

        # Inside a function body
        brace_depth += line.count('{') - line.count('}')
        if not skip_function:
            current_func_lines.append(line)

        if brace_depth <= 0:
            in_function = False
            if not skip_function:
                body_lines.extend(current_func_lines)
                body_lines.append('')
            skip_function = False
            current_func_lines = []
        i += 1

    return '\n'.join(body_lines), '\n'.join(header_lines)


# ---------------------------------------------------------------------------
# Loop-nest generation
# ---------------------------------------------------------------------------

def _find_kernel_body_range(source: str, kernel_name: str) -> tuple[int, int] | None:
    """Return (open_brace_pos, close_brace_pos) for *kernel_name*'s function body."""
    # Find function definition (already transformed: no __global__)
    pat = re.compile(
        rf'(?:^|\n)([^\n]*\b{re.escape(kernel_name)}\s*\([^)]*\)\s*\{{)',
        re.DOTALL,
    )
    m = pat.search(source)
    if not m:
        return None
    open_pos = source.index('{', m.start())
    depth = 0
    for i in range(open_pos, len(source)):
        if source[i] == '{':
            depth += 1
        elif source[i] == '}':
            depth -= 1
            if depth == 0:
                return (open_pos, i)
    return None


def _generate_loop_nests(
    source: str,
    kernel_names: list[str],
    used_dims: dict[str, set[str]],
) -> str:
    """Wrap each kernel body in for-loops over the used block/thread dimensions."""
    # Process kernels from bottom to top so positions stay valid
    replacements: list[tuple[int, int, str]] = []

    for kname in kernel_names:
        rng = _find_kernel_body_range(source, kname)
        if rng is None:
            continue
        open_pos, close_pos = rng
        # Extract the body *between* the braces (exclusive)
        inner = source[open_pos + 1 : close_pos]

        # Build nested loops (block outer, thread inner), ordered z→y→x
        # so the innermost loop is x (the one that gets PIPELINE)
        axes_order = [a for a in ('z', 'y', 'x') if a in used_dims.get('block', set()) or a in used_dims.get('thread', set())]

        if not axes_order:
            continue

        indent = '    '
        loop_open = ''
        loop_close = ''
        depth = 1  # starts at 1 because we're inside the function

        for axis in axes_order:
            pad = indent * depth
            if axis in used_dims.get('block', set()):
                loop_open += f'{pad}for (int _bid_{axis} = 0; _bid_{axis} < GRID_DIM_{axis.upper()}; _bid_{axis}++) {{\n'
                loop_close = f'{pad}}}\n' + loop_close
                depth += 1
                pad = indent * depth
            if axis in used_dims.get('thread', set()):
                loop_open += f'{pad}for (int _tid_{axis} = 0; _tid_{axis} < BLOCK_DIM_{axis.upper()}; _tid_{axis}++) {{\n'
                # Add PIPELINE pragma on the innermost thread loop
                if axis == axes_order[-1]:
                    loop_open += f'{pad}#pragma HLS PIPELINE II=1\n'
                loop_close = f'{pad}}}\n' + loop_close
                depth += 1

        # Re-indent inner body
        inner_lines = inner.split('\n')
        new_inner_lines = []
        for line in inner_lines:
            stripped = line.strip()
            if stripped:
                new_inner_lines.append(indent * depth + stripped)
            else:
                new_inner_lines.append('')
        new_inner = '\n'.join(new_inner_lines)

        new_body = '\n' + loop_open + new_inner + '\n' + loop_close
        replacements.append((open_pos + 1, close_pos, new_body))

    # Apply replacements from bottom to top
    replacements.sort(key=lambda r: r[0], reverse=True)
    for start, end, new_text in replacements:
        source = source[:start] + new_text + source[end:]

    return source


# ---------------------------------------------------------------------------
# HLS pragma insertion
# ---------------------------------------------------------------------------

def _insert_hls_pragmas(
    source: str,
    kernel_names: list[str],
    kernel_params: dict[str, list[tuple[str, str, bool]]],
    shared_arrays: list[str],
) -> str:
    """Insert HLS interface pragmas after each kernel's opening brace."""
    for kname in kernel_names:
        rng = _find_kernel_body_range(source, kname)
        if rng is None:
            continue
        open_pos = rng[0]
        params = kernel_params.get(kname, [])
        pragmas = []
        gmem_idx = 0
        for _type_str, pname, is_ptr in params:
            if is_ptr:
                pragmas.append(
                    f'#pragma HLS INTERFACE m_axi port={pname} offset=slave bundle=gmem{gmem_idx}'
                )
                gmem_idx += 1
            else:
                pragmas.append(f'#pragma HLS INTERFACE s_axilite port={pname}')
        pragmas.append('#pragma HLS INTERFACE s_axilite port=return')

        # Add ARRAY_PARTITION for former __shared__ arrays
        for arr_name in shared_arrays:
            # Only add if the array is referenced in this kernel's body
            body_end = rng[1]
            body_text = source[open_pos:body_end]
            if arr_name in body_text:
                pragmas.append(
                    f'#pragma HLS ARRAY_PARTITION variable={arr_name} complete dim=1'
                )

        pragma_block = '\n'.join('    ' + p for p in pragmas) + '\n'
        # Insert right after the opening brace
        insert_pos = open_pos + 1
        source = source[:insert_pos] + '\n' + pragma_block + source[insert_pos:]

    return source


# ---------------------------------------------------------------------------
# extern "C" wrapper
# ---------------------------------------------------------------------------

def _add_extern_c(source: str, kernel_names: list[str]) -> str:
    """Prepend ``extern "C"`` to each kernel function definition."""
    for kname in kernel_names:
        # Match the function definition line (already no __global__)
        pat = re.compile(rf'^([ \t]*)([\w\s\*]+\b{re.escape(kname)}\s*\()', re.MULTILINE)
        m = pat.search(source)
        if m:
            indent = m.group(1)
            # Avoid double-wrapping
            before = source[:m.start()]
            if before.rstrip().endswith('extern "C"'):
                continue
            source = source[:m.start()] + f'{indent}extern "C"\n{m.group(0)}' + source[m.end():]
    return source


# ---------------------------------------------------------------------------
# Dimension #define generation
# ---------------------------------------------------------------------------

_DIM_DEFAULTS = {
    'BLOCK_DIM_X': 256, 'BLOCK_DIM_Y': 1, 'BLOCK_DIM_Z': 1,
    'GRID_DIM_X': 1,    'GRID_DIM_Y': 1,  'GRID_DIM_Z': 1,
}


def _generate_dimension_defines(used_dims: dict[str, set[str]]) -> str:
    """Produce ``#define`` lines for the dimensions actually used."""
    lines: list[str] = []
    for axis in _AXES:
        if axis in used_dims.get('block', set()) or axis in used_dims.get('thread', set()):
            A = axis.upper()
            lines.append(f'#ifndef BLOCK_DIM_{A}')
            lines.append(f'#define BLOCK_DIM_{A} {_DIM_DEFAULTS[f"BLOCK_DIM_{A}"]}')
            lines.append(f'#endif')
        if axis in used_dims.get('block', set()):
            A = axis.upper()
            lines.append(f'#ifndef GRID_DIM_{A}')
            lines.append(f'#define GRID_DIM_{A} {_DIM_DEFAULTS[f"GRID_DIM_{A}"]}')
            lines.append(f'#endif')
    if lines:
        lines.insert(0, '// CUDA thread/block dimension constants (adjust for your workload)')
        lines.append('')
    return '\n'.join(lines)


# ---------------------------------------------------------------------------
# Per-benchmark conversion
# ---------------------------------------------------------------------------

def _strip_merged_includes(source: str, merged_filenames: set[str]) -> str:
    """Remove ``#include "..."`` lines that reference already-merged files."""
    def _replace(m: re.Match) -> str:
        fname = m.group(1)
        if fname in merged_filenames:
            return ''
        return m.group(0)

    return re.sub(r'#\s*include\s*"([^"]*)"[^\n]*\n?', _replace, source)


def convert_benchmark(
    bench_dir: Path,
    output_dir: Path,
    project_name: str | None = None,
) -> dict:
    """Convert a directory containing CUDA source files.

    Returns a dict with conversion metadata, or a dict with 'skip_reason'.
    """
    bench_name = project_name or bench_dir.name

    # Gather source files
    cu_files = sorted(bench_dir.glob('*.cu'))
    h_files = sorted(bench_dir.glob('*.h'))

    if not cu_files:
        return {'name': bench_name, 'skip_reason': 'no .cu files found'}

    all_source = ''
    for f in cu_files + h_files:
        try:
            all_source += f.read_text(errors='replace') + '\n'
        except Exception:
            pass

    # Check for unsupported libraries
    lib = _has_skip_library(all_source)
    if lib:
        return {'name': bench_name, 'skip_reason': f'uses {lib}'}

    # Must have at least one __global__ kernel
    kernel_names = _find_kernel_names(all_source)
    if not kernel_names:
        return {'name': bench_name, 'skip_reason': 'no __global__ kernel found'}

    # --- Detection passes on original source ---
    used_dims = _detect_used_dimensions(all_source)
    shared_arrays = _detect_shared_arrays(all_source)
    device_func_names = _find_device_function_names(all_source)
    kernel_params: dict[str, list[tuple[str, str, bool]]] = {}
    for kn in kernel_names:
        kernel_params[kn] = _parse_kernel_params(all_source, kn)

    # --- Apply transforms to each .cu file and merge ---
    kernel_parts: list[str] = []
    header_parts: list[str] = []

    for cu in cu_files:
        raw = cu.read_text(errors='replace')
        kn = _find_kernel_names(raw)
        dn = _find_device_function_names(raw)

        transformed = _apply_transforms(raw)
        kern, hdr = _extract_kernel_and_helpers(
            transformed,
            kn if kn else kernel_names,
            dn if dn else device_func_names,
        )
        if kern.strip():
            kernel_parts.append(f'// --- from {cu.name} ---\n{kern}')
        if hdr.strip():
            header_parts.append(f'// --- from {cu.name} ---\n{hdr}')

    for hf in h_files:
        if hf.name.startswith('reference'):
            continue
        raw = hf.read_text(errors='replace')
        transformed = _apply_transforms(raw)
        header_parts.append(f'// --- from {hf.name} ---\n{transformed}')

    # --- Merge kernel parts and apply new passes ---
    kernel_body = '\n\n'.join(kernel_parts)
    kernel_body = _generate_loop_nests(kernel_body, kernel_names, used_dims)
    kernel_body = _insert_hls_pragmas(kernel_body, kernel_names, kernel_params, shared_arrays)
    kernel_body = _add_extern_c(kernel_body, kernel_names)

    # --- Generate dimension defines ---
    dim_defines = _generate_dimension_defines(used_dims)

    # Collect names of all source files that were merged so we can strip
    # their #include lines (they're already inlined into kernel.h / kernel.cpp)
    merged_filenames = {f.name for f in cu_files + h_files}

    # Build kernel.h
    header_text = (
        '#pragma once\n'
        '#include <cstdint>\n'
        '#include <cmath>\n'
        '#include <algorithm>\n'
        'using std::min;\n'
        'using std::max;\n\n'
        + (dim_defines + '\n' if dim_defines else '')
        + '\n\n'.join(header_parts)
    )

    # Strip #include lines for files we already merged
    header_text = _strip_merged_includes(header_text, merged_filenames)
    kernel_body = _strip_merged_includes(kernel_body, merged_filenames)

    # Build kernel.cpp
    kernel_text = '#include "kernel.h"\n\n' + kernel_body

    # Write output
    out_dir = output_dir / bench_name
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / 'kernel.cpp').write_text(kernel_text, encoding='utf-8')
    (out_dir / 'kernel.h').write_text(header_text, encoding='utf-8')

    # Copy reference implementation if present
    for ref_name in ('reference.h', 'reference.cu', 'reference.cpp'):
        ref_path = bench_dir / ref_name
        if ref_path.exists():
            dest = out_dir / 'reference.cpp'
            raw = ref_path.read_text(errors='replace')
            dest.write_text(_apply_transforms(raw), encoding='utf-8')
            break  # only one reference file

    return {
        'name': bench_name,
        'kernels': kernel_names,
        'source_files': [f.name for f in cu_files],
    }


# ---------------------------------------------------------------------------
# Single-file conversion helper
# ---------------------------------------------------------------------------

def convert_file(cu_path: Path, output_dir: Path) -> dict:
    """Convert a single .cu file (no project directory).

    Creates a project named after the file stem.
    """
    project_name = cu_path.stem
    raw = cu_path.read_text(errors='replace')

    lib = _has_skip_library(raw)
    if lib:
        return {'name': project_name, 'skip_reason': f'uses {lib}'}

    kernel_names = _find_kernel_names(raw)
    if not kernel_names:
        return {'name': project_name, 'skip_reason': 'no __global__ kernel found'}

    # Detection passes on original source
    used_dims = _detect_used_dimensions(raw)
    shared_arrays = _detect_shared_arrays(raw)
    device_func_names = _find_device_function_names(raw)
    kernel_params: dict[str, list[tuple[str, str, bool]]] = {}
    for kn in kernel_names:
        kernel_params[kn] = _parse_kernel_params(raw, kn)

    transformed = _apply_transforms(raw)
    kern, hdr = _extract_kernel_and_helpers(transformed, kernel_names, device_func_names)

    # Apply new passes
    kern = _generate_loop_nests(kern, kernel_names, used_dims)
    kern = _insert_hls_pragmas(kern, kernel_names, kernel_params, shared_arrays)
    kern = _add_extern_c(kern, kernel_names)

    dim_defines = _generate_dimension_defines(used_dims)

    merged_filenames = {cu_path.name}

    header_text = (
        '#pragma once\n'
        '#include <cstdint>\n'
        '#include <cmath>\n'
        '#include <algorithm>\n'
        'using std::min;\n'
        'using std::max;\n\n'
        + (dim_defines + '\n' if dim_defines else '')
        + hdr
    )

    header_text = _strip_merged_includes(header_text, merged_filenames)
    kern = _strip_merged_includes(kern, merged_filenames)
    kernel_text = '#include "kernel.h"\n\n' + kern

    out_dir = output_dir / project_name
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / 'kernel.cpp').write_text(kernel_text, encoding='utf-8')
    (out_dir / 'kernel.h').write_text(header_text, encoding='utf-8')

    return {
        'name': project_name,
        'kernels': kernel_names,
        'source_files': [cu_path.name],
    }


# ---------------------------------------------------------------------------
# Discovery helpers
# ---------------------------------------------------------------------------

def _discover_projects(src: Path) -> list[tuple[Path, str]]:
    """Return list of (directory, project_name) pairs to convert.

    Supports three layouts:
      1. src is a single .cu file        → [(parent_dir, file_stem)]
      2. src is a dir with .cu files      → [(src, src.name)]
      3. src has subdirs with .cu files   → [(subdir, cleaned_name), ...]
    """
    # Case 1: single file (handled separately in main, but included for completeness)
    if src.is_file():
        return [(src.parent, src.stem)]

    # Check subdirectories for .cu files
    subdirs_with_cu: list[tuple[Path, str]] = []
    has_direct_cu = bool(list(src.glob('*.cu')))

    for child in sorted(src.iterdir()):
        if not child.is_dir():
            continue
        if list(child.glob('*.cu')):
            # Strip common CUDA directory suffixes for a cleaner name
            name = child.name
            for suffix in ('-cuda', '-sycl', '_cuda', '.cuda'):
                if name.endswith(suffix):
                    name = name[:-len(suffix)]
                    break
            subdirs_with_cu.append((child, name))

    # Case 3: subdirectories found
    if subdirs_with_cu:
        return subdirs_with_cu

    # Case 2: .cu files directly in src
    if has_direct_cu:
        return [(src, src.name)]

    # Fallback: recurse one more level to find any .cu files
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


# ---------------------------------------------------------------------------
# CLI entry point
# ---------------------------------------------------------------------------

def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description='Convert CUDA kernels to HLS-compatible C++',
    )
    parser.add_argument(
        'src',
        type=Path,
        help=(
            'CUDA source: a .cu file, a directory with .cu files, '
            'or a parent directory whose subdirectories contain .cu files'
        ),
    )
    parser.add_argument(
        '-o', '--output',
        type=Path,
        default=Path('./cuda2hls_output'),
        help='Output directory (default: ./cuda2hls_output)',
    )
    parser.add_argument(
        '--limit',
        type=int,
        default=0,
        help='Max number of projects to convert (0 = no limit)',
    )
    parser.add_argument(
        '--filter',
        type=str,
        default='',
        help='Comma-separated name substrings to include',
    )
    args = parser.parse_args(argv)

    src: Path = args.src
    output_dir: Path = args.output
    limit: int = args.limit
    name_filters: list[str] = [f.strip() for f in args.filter.split(',') if f.strip()]

    # ---- Single .cu file mode ----
    if src.is_file():
        if not src.suffix == '.cu':
            print(f"Error: {src} is not a .cu file", file=sys.stderr)
            return 1
        output_dir.mkdir(parents=True, exist_ok=True)
        result = convert_file(src, output_dir)
        if 'skip_reason' in result:
            print(f"SKIP  {result['name']}: {result['skip_reason']}")
            return 1
        kernels = ', '.join(result['kernels'])
        print(f"OK    {result['name']} ({len(result['kernels'])} kernel(s): {kernels})")
        manifest = {
            'total_found': 1, 'converted': 1, 'skipped': 0,
            'benchmarks': [result], 'skipped_benchmarks': [],
        }
        (output_dir / 'manifest.json').write_text(
            json.dumps(manifest, indent=2), encoding='utf-8',
        )
        print(f"Output: {output_dir / result['name']}")
        return 0

    if not src.is_dir():
        print(f"Error: {src} is not a file or directory", file=sys.stderr)
        return 1

    # ---- Directory mode: discover projects ----
    projects = _discover_projects(src)

    if not projects:
        print(f"No CUDA source files found in {src}", file=sys.stderr)
        return 1

    # Apply name filter
    if name_filters:
        projects = [
            (d, n) for d, n in projects
            if any(f in n or f in d.name for f in name_filters)
        ]

    # Apply limit
    if limit > 0:
        projects = projects[:limit]

    print(f"Found {len(projects)} CUDA project(s) to process")

    output_dir.mkdir(parents=True, exist_ok=True)

    converted: list[dict] = []
    skipped: list[dict] = []

    for proj_dir, proj_name in projects:
        result = convert_benchmark(proj_dir, output_dir, project_name=proj_name)
        if 'skip_reason' in result:
            skipped.append(result)
            print(f"  SKIP  {result['name']}: {result['skip_reason']}")
        else:
            converted.append(result)
            kernels = ', '.join(result['kernels'])
            print(f"  OK    {result['name']} ({len(result['kernels'])} kernel(s): {kernels})")

    # Write manifest
    manifest = {
        'total_found': len(projects),
        'converted': len(converted),
        'skipped': len(skipped),
        'benchmarks': converted,
        'skipped_benchmarks': skipped,
    }
    manifest_path = output_dir / 'manifest.json'
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding='utf-8')

    # Summary
    print(f"\n{'='*50}")
    print(f"Total projects found:    {len(projects)}")
    print(f"Converted successfully:  {len(converted)}")
    print(f"Skipped:                 {len(skipped)}")
    print(f"Output directory:        {output_dir}")
    print(f"Manifest:                {manifest_path}")

    if skipped:
        print(f"\nSkip reasons:")
        reasons: dict[str, int] = {}
        for s in skipped:
            r = s['skip_reason']
            reasons[r] = reasons.get(r, 0) + 1
        for reason, count in sorted(reasons.items(), key=lambda x: -x[1]):
            print(f"  {count:3d}  {reason}")

    return 0


if __name__ == '__main__':
    sys.exit(main())
