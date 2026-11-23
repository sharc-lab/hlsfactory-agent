#!/usr/bin/env python3
"""HLS marker scanner for a GitHub repo. Standard library only.

Runs on the host and, copied to tools/hls_scan.py, inside the agent container:

    hls_scan.py owner/name [--cap-mb 300] [--dir PATH]

Prints one JSON object. The GitHub API base and proxy headers come from the environment
(GH_API_BASE, PROXY_SECRET, PAPER_ID) so the same script works through the host proxy.
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import urllib.error
import urllib.request
from pathlib import Path

DEFAULT_CAP_KB = 300 * 1024
SOURCE_EXTS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".cl"}
TCL_EXT = ".tcl"
# Archives may hold the designs (e.g. UCLA-DM/HLSyn ships its kernels in a .tar.gz).
ARCHIVE_SUFFIXES = (".tar.gz", ".tgz", ".tar", ".zip", ".tar.xz", ".tar.bz2", ".7z")
SKIP_DIRS = {".git", "node_modules", ".svn", "__pycache__"}
MAX_FILE_BYTES = 2_000_000
MAX_FILES = 50_000

# Dialects HLSFactory converts to Vitis. TAPA designs are skipped by the extractor.
HANDLED = ("vitis", "intel", "catapult", "legup", "dynamatic")
MARKERS = {
    "tapa": [r'#\s*include\s*[<"]tapa\.h[>"]', r"\btapa::"],
    # Strong Vitis markers. Plain ap_int.h/ap_fixed.h includes are NOT enough: host code and
    # testbenches include them for data types (see vitis_weak).
    "vitis": [
        r"#\s*pragma\s+HLS\b",
        r'#\s*include\s*[<"](?:hls_stream|hls_math|hls_vector)\.h[>"]',
    ],
    "vitis_weak": [
        r'#\s*include\s*[<"](?:ap_int|ap_fixed|ap_float|ap_axi_sdata|ap_shift_reg)\.h[>"]',
    ],
    "intel": [r'#\s*include\s*[<"]HLS/', r"\bihc::", r"\bhls_avalon_"],
    "catapult": [
        r'#\s*include\s*[<"]ac_(?:int|fixed|float|channel|sync)\.h[>"]',
        r"\bac_channel\b",
        r"#\s*pragma\s+hls_(?:design|unroll|pipeline_init_interval)",
    ],
    "legup": [r'#\s*include\s*[<"]legup/', r"\blegup_[a-z_]+\s*\("],
    "dynamatic": [r"dynamatic/Integration\.h", r'#\s*include\s*[<"]dynamatic'],
}
COMPILED = {d: [re.compile(p) for p in ps] for d, ps in MARKERS.items()}
PRAGMA_HLS = re.compile(r"#\s*pragma\s+HLS\b")
SET_TOP = re.compile(r"\bset_top\s+(\S+)")
CSYNTH = re.compile(r"\bcsynth_design\b")


def classify_text(text: str) -> set[str]:
    """Dialects whose markers appear in one source file's text."""
    return {d for d, pats in COMPILED.items() if any(p.search(text) for p in pats)}


def scan_dir(root: Path) -> dict:
    """Walk a checked-out repo and count HLS markers per dialect."""
    root = Path(root)
    by_dialect = {d: 0 for d in MARKERS}
    hls_files = tapa_files = weak_files = pragma_count = scanned = 0
    tcl_tops: list[str] = []
    tcl_synth = 0
    source_files = 0
    archives: list[str] = []
    examples: list[str] = []
    truncated = False

    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
        for name in filenames:
            ext = Path(name).suffix.lower()
            if name.lower().endswith(ARCHIVE_SUFFIXES):
                if len(archives) < 10:
                    archives.append(str((Path(dirpath) / name).relative_to(root)))
                continue
            if ext not in SOURCE_EXTS and ext != TCL_EXT:
                continue
            path = Path(dirpath) / name
            try:
                if path.stat().st_size > MAX_FILE_BYTES or path.is_symlink():
                    continue
                text = path.read_text(errors="ignore")
            except OSError:
                continue
            scanned += 1
            if scanned > MAX_FILES:
                truncated = True
                break
            if ext == TCL_EXT:
                tcl_tops += SET_TOP.findall(text)
                tcl_synth += len(CSYNTH.findall(text))
                continue
            source_files += 1
            dialects = classify_text(text)
            for d in dialects:
                by_dialect[d] += 1
            pragma_count += len(PRAGMA_HLS.findall(text))
            if "tapa" in dialects:
                tapa_files += 1
            elif dialects & set(HANDLED):
                hls_files += 1
                if len(examples) < 10:
                    examples.append(str(path.relative_to(root)))
            elif "vitis_weak" in dialects:
                weak_files += 1
        if truncated:
            break

    if hls_files:
        classification = "hls_found"
    elif tapa_files:
        classification = "tapa_only"
    elif tcl_tops and tcl_synth and source_files:
        # Vitis/Vivado HLS project whose directives live in TCL (set_directive_*), not in
        # #pragma HLS lines, e.g. breagen/MachSuite. Needs C/C++ source next to the script.
        classification = "hls_tcl_only"
    elif weak_files:
        classification = "weak_hls_only"  # only ap_int-style includes: needs a human look
    elif archives and not source_files:
        classification = "archive_only"  # no C/C++ at all, but designs may be in an archive
    else:
        classification = "repo_no_hls"
    return {
        "classification": classification,
        "mode": "clone",
        "files_scanned": scanned,
        "source_files": source_files,
        "archives": archives,
        "hls_files": hls_files,
        "tapa_files": tapa_files,
        "weak_files": weak_files,
        "mixed_tapa": bool(hls_files and tapa_files),
        "tapa_dominant": bool(hls_files and tapa_files > hls_files),
        "by_dialect": by_dialect,
        "pragma_hls_count": pragma_count,
        "tcl_set_top": sorted(set(tcl_tops))[:20],
        "tcl_csynth_count": tcl_synth,
        "examples": examples,
        "truncated": truncated,
    }


def scan_paths(paths: list[str], truncated: bool = False) -> dict:
    """Path-only fallback for repos over the size cap (git tree API, no file contents).

    Cannot see markers, so it never claims hls_found: classification is 'unscanned' and
    the repo goes to human review with these hints.
    """
    srcs = [p for p in paths if Path(p).suffix.lower() in SOURCE_EXTS]
    tcls = [p for p in paths if p.lower().endswith(TCL_EXT)]
    hls_dirs = sorted({str(Path(p).parent) for p in paths if "hls" in p.lower()})[:10]
    return {
        "classification": "unscanned",
        "mode": "tree",
        "partial_scan": True,
        "tree_truncated": truncated,
        "source_files": len(srcs),
        "tcl_files": len(tcls),
        "hls_named_paths": hls_dirs,
    }


SPARSE_PATTERNS = [f"*{e}" for e in sorted(SOURCE_EXTS | {TCL_EXT})]


def _git(args: list[str], cwd: Path | None = None, timeout: int = 600):
    return subprocess.run(
        ["git", *args], cwd=cwd, capture_output=True, text=True, timeout=timeout,
        env={**os.environ, "GIT_TERMINAL_PROMPT": "0"},
    )  # fmt: skip


def clone_source_only(url: str, dest: Path) -> str | None:
    """Partial clone that downloads only source-file blobs (no data files, bitstreams...).

    Returns an error string, or None on success. Works for repos far over the size cap
    because the cap applies to source bytes, not the whole repo.
    """
    p = _git(["clone", "--depth", "1", "--filter=blob:none", "--no-checkout", "--quiet", url, str(dest)])
    if p.returncode != 0:
        return f"git clone failed: {p.stderr.strip()[:300]}"
    _git(["config", "core.sparseCheckout", "true"], cwd=dest)
    (dest / ".git" / "info").mkdir(parents=True, exist_ok=True)
    (dest / ".git" / "info" / "sparse-checkout").write_text("\n".join(SPARSE_PATTERNS) + "\n")
    p = _git(["read-tree", "-mu", "HEAD"], cwd=dest)
    if p.returncode != 0:
        return f"sparse checkout failed: {p.stderr.strip()[:300]}"
    return None


def source_bytes(tree: list[dict]) -> int:
    exts = SOURCE_EXTS | {TCL_EXT}
    return sum(
        t.get("size", 0) for t in tree
        if t.get("type") == "blob" and Path(t["path"]).suffix.lower() in exts
    )  # fmt: skip


# --- GitHub access (stdlib, proxy aware) ----------------------------------------------


def _api_base() -> str:
    return os.environ.get("GH_API_BASE", "https://api.github.com").rstrip("/")


def _api_get(path: str) -> tuple[int, dict | list | None]:
    headers = {"Accept": "application/vnd.github+json"}
    if os.environ.get("PROXY_SECRET"):
        headers["X-Proxy-Secret"] = os.environ["PROXY_SECRET"]
    if os.environ.get("PAPER_ID"):
        headers["X-Paper-Id"] = os.environ["PAPER_ID"]
    req = urllib.request.Request(f"{_api_base()}/{path.lstrip('/')}", headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=180) as r:
            return r.status, json.loads(r.read().decode())
    except urllib.error.HTTPError as e:
        try:
            return e.code, json.loads(e.read().decode())
        except ValueError:
            return e.code, None
    except (urllib.error.URLError, TimeoutError, ValueError) as e:
        return 0, {"error": str(e)}


def scan_repo(repo_id: str, cap_kb: int = DEFAULT_CAP_KB) -> dict:
    status, meta = _api_get(f"repos/{repo_id}")
    if status == 404:
        return {"classification": "link_dead", "exists": False, "repo_id": repo_id}
    if status != 200 or not isinstance(meta, dict):
        return {
            "classification": "error",
            "repo_id": repo_id,
            "error": f"repo API returned {status}: {meta}",
        }
    full_name = meta["full_name"]
    size_kb = meta.get("size") or 0
    repo_meta = {
        "full_name": full_name,
        "renamed_to": full_name if full_name.lower() != repo_id.lower() else None,
        "archived": meta.get("archived"),
        "fork": meta.get("fork"),
        "language": meta.get("language"),
        "created_at": meta.get("created_at"),
        "pushed_at": meta.get("pushed_at"),
        "description": meta.get("description"),
        "size_kb": size_kb,
    }
    url = f"https://github.com/{full_name}.git"
    if size_kb > cap_kb:
        # Big repo: size the SOURCE files via the tree API; if they fit under the cap, do a
        # source-only partial clone, else fall back to a path-only (unscanned) result.
        status, tree = _api_get(f"repos/{full_name}/git/trees/HEAD?recursive=1")
        if status != 200 or not isinstance(tree, dict):
            out = {"classification": "error", "error": f"tree API returned {status}"}
        else:
            entries = tree.get("tree", [])
            src_kb = source_bytes(entries) // 1024
            if src_kb <= cap_kb:
                tmp = tempfile.mkdtemp(prefix="hlsscan_")
                try:
                    err = clone_source_only(url, Path(tmp) / "repo")
                    if err is None:
                        out = scan_dir(Path(tmp) / "repo")
                        out["mode"] = "sparse"
                    else:
                        out = scan_paths([t["path"] for t in entries if t.get("type") == "blob"],
                                         bool(tree.get("truncated")))
                        out["error"] = err
                except subprocess.TimeoutExpired:
                    out = {"classification": "error", "error": "sparse clone timed out"}
                finally:
                    shutil.rmtree(tmp, ignore_errors=True)
            else:
                out = scan_paths([t["path"] for t in entries if t.get("type") == "blob"],
                                 bool(tree.get("truncated")))
            out["source_kb"] = src_kb
        out.update(repo_id=repo_id, exists=True, repo_meta=repo_meta, cap_kb=cap_kb)
        return out

    tmp = tempfile.mkdtemp(prefix="hlsscan_")
    try:
        p = subprocess.run(
            ["git", "clone", "--depth", "1", "--quiet", url, tmp + "/repo"],
            capture_output=True, text=True, timeout=600,
            env={**os.environ, "GIT_TERMINAL_PROMPT": "0"},
        )  # fmt: skip
        if p.returncode != 0:
            return {
                "classification": "error",
                "repo_id": repo_id,
                "exists": True,
                "repo_meta": repo_meta,
                "error": f"git clone failed: {p.stderr.strip()[:300]}",
            }
        out = scan_dir(Path(tmp) / "repo")
    except subprocess.TimeoutExpired:
        out = {"classification": "error", "error": "git clone timed out"}
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    out.update(repo_id=repo_id, exists=True, repo_meta=repo_meta, cap_kb=cap_kb)
    return out


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("repo_id", help="owner/name (ignored with --dir)")
    ap.add_argument("--cap-mb", type=int, default=DEFAULT_CAP_KB // 1024)
    ap.add_argument("--dir", type=Path, help="scan an existing checkout instead")
    args = ap.parse_args()
    if args.dir:
        out = scan_dir(args.dir)
        out["repo_id"] = args.repo_id
    else:
        out = scan_repo(args.repo_id, args.cap_mb * 1024)
    json.dump(out, sys.stdout, indent=2)
    print()


if __name__ == "__main__":
    main()
