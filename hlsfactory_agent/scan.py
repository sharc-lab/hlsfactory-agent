"""Deterministic inventory of what a Vitis HLS design contains, so nothing can be dropped silently.

The scanner reports every `#pragma HLS` kind, every vendor type, every vendor header, and a few
constructs that no target tool accepts (dynamic memory, STL containers, recursion). Its output is
written next to the design as `scan.json`, given to the agent, and used afterwards to check that
the translation report accounts for every pragma kind.
"""

from __future__ import annotations

import json
import re
from collections import Counter
from pathlib import Path

SOURCE_SUFFIXES = (".cpp", ".cc", ".c", ".h", ".hpp")

RE_PRAGMA = re.compile(r"^\s*#\s*pragma\s+HLS\s+([A-Za-z_]+)", re.IGNORECASE)
RE_TYPES = re.compile(r"\b(ap_int|ap_uint|ap_fixed|ap_ufixed|hls::stream|hls::vector)\b")
RE_HEADER = re.compile(
    r'^\s*#\s*include\s*[<"]((?:ap_int|ap_fixed|hls_stream|hls_math|hls_vector|ap_axi_sdata)\.h)[>"]'
)
RE_CONSTRUCTS = {
    "dynamic_memory": re.compile(r"\bmalloc\s*\(|\bcalloc\s*\(|\bnew\s+[A-Za-z_]"),
    "stl_container": re.compile(r"\bstd::(vector|map|set|string|list|deque)\b"),
}
# A function definition: something, a name, an argument list, an opening brace. Not a call, not a prototype.
RE_FN_DEF = re.compile(r"^\s*[A-Za-z_][\w:<>,\s\*&]*?\b([A-Za-z_]\w*)\s*\([^;{]*\)\s*\{", re.MULTILINE)


def _is_comment(line: str) -> bool:
    return line.lstrip().startswith("//")


def _source_files(dir_design: Path) -> list[Path]:
    return sorted(p for p in dir_design.rglob("*") if p.is_file() and p.suffix.lower() in SOURCE_SUFFIXES)


def _recursion_candidates(text: str, rel: str) -> list[dict]:
    """Functions whose own name is called inside their body."""
    found: list[dict] = []
    for m in RE_FN_DEF.finditer(text):
        name = m.group(1)
        if name in ("if", "for", "while", "switch", "return"):
            continue
        body_start = m.end()
        depth, j = 1, body_start
        while j < len(text) and depth:
            if text[j] == "{":
                depth += 1
            elif text[j] == "}":
                depth -= 1
            j += 1
        if re.search(rf"\b{re.escape(name)}\s*\(", text[body_start:j]):
            line_no = text.count("\n", 0, m.start()) + 1
            found.append({"kind": "recursion_candidate", "file": rel, "line": line_no, "text": name})
    return found


def scan_design(dir_design: Path) -> dict:
    dir_design = Path(dir_design)
    pragmas: list[dict] = []
    types: list[dict] = []
    headers: list[dict] = []
    constructs: list[dict] = []
    for p in _source_files(dir_design):
        rel = str(p.relative_to(dir_design))
        text = p.read_text(encoding="utf-8", errors="replace")
        for i, line in enumerate(text.splitlines(), start=1):
            if _is_comment(line):
                continue
            m = RE_PRAGMA.match(line)
            if m:
                pragmas.append({"kind": m.group(1).lower(), "file": rel, "line": i, "text": line.strip()})
                continue
            m = RE_HEADER.match(line)
            if m:
                headers.append({"kind": m.group(1), "file": rel, "line": i, "text": line.strip()})
                continue
            for t in RE_TYPES.findall(line):
                types.append({"kind": t, "file": rel, "line": i, "text": line.strip()})
            for kind, rx in RE_CONSTRUCTS.items():
                if rx.search(line):
                    constructs.append({"kind": kind, "file": rel, "line": i, "text": line.strip()})
        constructs.extend(_recursion_candidates(text, rel))
    return {
        "pragmas": pragmas,
        "types": types,
        "headers": headers,
        "constructs": constructs,
        "counts": {
            "pragmas": dict(Counter(x["kind"] for x in pragmas)),
            "types": dict(Counter(x["kind"] for x in types)),
            "headers": dict(Counter(x["kind"] for x in headers)),
            "constructs": dict(Counter(x["kind"] for x in constructs)),
        },
    }


def write_scan(dir_design: Path, dir_out: Path) -> Path:
    out = Path(dir_out) / "scan.json"
    out.write_text(json.dumps(scan_design(dir_design), indent=2), encoding="utf-8")
    return out
