"""Deterministic inventory of what a Vitis HLS design contains, so nothing can be dropped silently.

The scanner reports every `#pragma HLS` kind, every vendor type, every vendor header, and a few
constructs that no target tool accepts (dynamic memory, STL containers, recursion). Its output is
written next to the design as `scan.json`, given to the agent, and used afterwards to check that
the translation report accounts for every pragma kind.
"""

from __future__ import annotations

import ast
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
RESOURCE_KINDS = ("array_partition", "bind_storage", "interface", "dependence", "stream")
RE_DEFINE = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)\s+(.+?)\s*$")
RE_KV = re.compile(r"([A-Za-z_]\w*)\s*=\s*(\S+)")
RE_ARRAY_DECL_T = r"\b{var}\s*\[\s*([^\]]+?)\s*\]"
BARE_PARTITION_WORDS = ("complete", "cyclic", "block")
_ALLOWED_AST = (
    ast.Expression, ast.BinOp, ast.UnaryOp, ast.Constant,
    ast.Add, ast.Sub, ast.Mult, ast.Div, ast.FloorDiv, ast.USub, ast.UAdd,
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


def collect_defines(dir_design: Path) -> dict[str, str]:
    defines: dict[str, str] = {}
    for p in _source_files(Path(dir_design)):
        for line in p.read_text(encoding="utf-8", errors="replace").splitlines():
            m = RE_DEFINE.match(line)
            if m and "(" not in m.group(1):
                defines.setdefault(m.group(1), m.group(2).split("//")[0].strip())
    return defines


def resolve_int(expr: str, defines: dict[str, str], _depth: int = 0) -> int | None:
    if _depth > 10:
        return None
    text = expr.strip()
    for name in sorted(defines, key=len, reverse=True):
        if re.search(rf"\b{re.escape(name)}\b", text):
            text = re.sub(rf"\b{re.escape(name)}\b", f"({defines[name]})", text)
    if re.search(r"[A-Za-z_]", text):
        return resolve_int(text, defines, _depth + 1) if text != expr.strip() else None
    try:
        tree = ast.parse(text, mode="eval")
    except SyntaxError:
        return None
    if not all(isinstance(n, _ALLOWED_AST) for n in ast.walk(tree)):
        return None
    try:
        value = eval(compile(tree, "<factor>", "eval"), {"__builtins__": {}}, {})
    except Exception:
        return None
    return int(value) if isinstance(value, (int, float)) and float(value).is_integer() else None


def parse_pragma_args(kind: str, rest: str) -> dict[str, str]:
    args = {k.lower(): v for k, v in RE_KV.findall(rest)}
    if kind == "array_partition":
        for word in rest.replace("=", " = ").split():
            if word.lower() in BARE_PARTITION_WORDS:
                args.setdefault("type", word.lower())
    return args


def top_signature(dir_design: Path, top: str) -> str:
    rx = re.compile(r"\b" + re.escape(top) + r"\s*\(([^;{]*)\)\s*\{", re.S)
    for p in _source_files(Path(dir_design)):
        m = rx.search(p.read_text(encoding="utf-8", errors="replace"))
        if m:
            return m.group(1)
    return ""


def array_params(signature: str) -> set[str]:
    return set(re.findall(r"([A-Za-z_]\w*)\s*(?:\[[^\]]*\])+", signature))


def array_size(dir_design: Path, var: str, defines: dict[str, str]) -> int | None:
    rx = re.compile(RE_ARRAY_DECL_T.format(var=re.escape(var)))
    for p in _source_files(Path(dir_design)):
        for m in rx.finditer(p.read_text(encoding="utf-8", errors="replace")):
            n = resolve_int(m.group(1), defines)
            if n:
                return n
    return None


def scan_design(dir_design: Path, top: str | None = None) -> dict:
    dir_design = Path(dir_design)
    defines = collect_defines(dir_design)
    params = array_params(top_signature(dir_design, top)) if top else None
    pragmas: list[dict] = []
    resources: list[dict] = []
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
                kind = m.group(1).lower()
                pragmas.append({"kind": kind, "file": rel, "line": i, "text": line.strip()})
                if kind in RESOURCE_KINDS:
                    args = parse_pragma_args(kind, line[m.end():])
                    factor = resolve_int(args["factor"], defines) if "factor" in args else None
                    unresolved = args["factor"] if "factor" in args and factor is None else None
                    var = args.get("variable", "")
                    if params is None:
                        scope = "unknown"
                    else:
                        scope = "argument" if var in params else "local"
                    resources.append(
                        {
                            "kind": kind,
                            "file": rel,
                            "line": i,
                            "text": line.strip(),
                            "args": args,
                            "factor": factor,
                            "unresolved": unresolved,
                            "scope": scope,
                            "size": array_size(dir_design, var, defines) if var else None,
                        }
                    )
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
        "defines": defines,
        "resources": resources,
        "types": types,
        "headers": headers,
        "constructs": constructs,
        "counts": {
            "pragmas": dict(Counter(x["kind"] for x in pragmas)),
            "types": dict(Counter(x["kind"] for x in types)),
            "headers": dict(Counter(x["kind"] for x in headers)),
            "constructs": dict(Counter(x["kind"] for x in constructs)),
            "resources": dict(Counter(r["kind"] for r in resources)),
        },
    }


def write_scan(dir_design: Path, dir_out: Path, top: str | None = None) -> Path:
    out = Path(dir_out) / "scan.json"
    out.write_text(json.dumps(scan_design(dir_design, top=top), indent=2), encoding="utf-8")
    return out
