"""Mechanical Vitis-to-target rewrite: everything the mapping table can do without judgment.

Runs on the host before the agent starts. It copies the design, swaps headers and types, moves
loop pragmas to the target's placement, drops pragmas with no equivalent, inserts the top-function
marker, and writes ``rewrite_log.json``. Anything it cannot do with certainty goes into the log's
``residue`` list for the agent. It never guesses at numerics.
"""

from __future__ import annotations

import json
import re
import shutil
from pathlib import Path

from hlsfactory_agent.translate import SOURCE_SUFFIXES, TargetSpec

HEADER_MAPS: dict[str, dict[str, str]] = {
    "catapult": {"ap_int.h": "ac_int.h", "ap_fixed.h": "ac_fixed.h", "hls_stream.h": "ac_channel.h"},
    "xlscc": {"ap_int.h": "ac_int.h", "ap_fixed.h": "ac_fixed.h", "hls_stream.h": "xls_emu.h"},
}
STREAM_MAPS: dict[str, str] = {"catapult": "ac_channel<", "xlscc": "__xls_channel<"}
MODE_MAP = {"AP_TRN": "AC_TRN", "AP_RND": "AC_RND", "AP_WRAP": "AC_WRAP", "AP_SAT": "AC_SAT"}

RE_INCLUDE = re.compile(r'^(\s*#\s*include\s*)([<"])(ap_int\.h|ap_fixed\.h|hls_stream\.h)([>"])')
RE_AP_INT = re.compile(r"\bap_int\s*<\s*([^<>,]+?)\s*>")
RE_AP_UINT = re.compile(r"\bap_uint\s*<\s*([^<>,]+?)\s*>")
RE_AP_FIXED = re.compile(
    r"\b(ap_fixed|ap_ufixed)\s*<\s*([^<>,]+?)\s*,\s*([^<>,]+?)\s*(?:,\s*([A-Z_]+)\s*)?(?:,\s*([A-Z_]+)\s*)?>"
)
RE_STREAM = re.compile(r"\bhls::stream\s*<")
RE_PRAGMA = re.compile(r"^(\s*)#\s*pragma\s+HLS\s+([A-Za-z_]+)(.*)$", re.IGNORECASE)
RE_LOOP = re.compile(r"^\s*(for|while)\b")
RE_FN_HEADER_END = re.compile(r"\)\s*(const\s*)?\{?\s*$")
# Methods of ap_int/ap_fixed with no same-named ac_int/ac_fixed equivalent. to_int, to_uint, to_double and the
# *_reduce family exist on both families and are left alone.
RE_VENDOR_METHOD = re.compile(r"\.(range|reverse|length|set_bit|get_bit|bit|test|invert|rrotate|lrotate|concat)\s*\(")
RE_INT_LITERAL = re.compile(r"^\d+$")

LOOKBACK_LINES = 3


def _fn_def_regex(top: str) -> re.Pattern:
    # a definition line: return type, the top name, an argument list, optionally the opening brace; never ends with ';'
    return re.compile(r"^\s*[A-Za-z_][\w:<>,\s\*&]*?\b" + re.escape(top) + r"\s*\([^;]*\)\s*\{?\s*$")


def _rewrite_types(line: str, target: TargetSpec, log: dict, rel: str, i: int) -> str:
    before = line
    line = RE_AP_INT.sub(r"ac_int<\1, true>", line)
    line = RE_AP_UINT.sub(r"ac_int<\1, false>", line)

    def fixed(m: re.Match) -> str:
        signed = "true" if m.group(1) == "ap_fixed" else "false"
        parts = [m.group(2), m.group(3), signed]
        for mode in (m.group(4), m.group(5)):
            if not mode:
                continue
            if mode in MODE_MAP:
                parts.append(MODE_MAP[mode])
            else:
                log["residue"].append(
                    {"file": rel, "line": i, "reason": f"no {target.display_name} mapping for fixed-point mode {mode}; numerics must be preserved"}
                )
                parts.append(mode)
        return f"ac_fixed<{', '.join(parts)}>"

    line = RE_AP_FIXED.sub(fixed, line)
    line = RE_STREAM.sub(STREAM_MAPS.get(target.name, "ac_channel<"), line)
    if line != before:
        log["types"].append({"file": rel, "line": i, "before": before.strip(), "after": line.strip()})
    m = RE_VENDOR_METHOD.search(line)
    if m and not line.lstrip().startswith("//"):
        log["residue"].append({"file": rel, "line": i, "reason": f"vendor method needs manual translation: {m.group(0)}"})
    return line


def _find_back(out: list[str], predicate) -> int | None:
    j = len(out) - 1
    while j >= 0 and j >= len(out) - LOOKBACK_LINES:
        if predicate(out[j]):
            return j
        j -= 1
    return None


def _rewrite_pragmas(lines: list[str], target: TargetSpec, log: dict, rel: str) -> list[str]:
    out: list[str] = []
    for i, line in enumerate(lines, start=1):
        m = RE_PRAGMA.match(line)
        if not m:
            out.append(line)
            continue
        kind, rest = m.group(2).lower(), m.group(3)
        entry = {"file": rel, "line": i, "kind": kind, "before": line.strip()}

        if kind == "pipeline":
            if re.search(r"\boff\b", rest, re.IGNORECASE):
                entry["reason"] = "pipeline off has no target equivalent"
                log["pragmas_dropped"].append(entry)
                continue
            ii = re.search(r"II\s*=\s*([A-Za-z_0-9]+)", rest, re.IGNORECASE)
            value = ii.group(1) if ii else "1"
            if not RE_INT_LITERAL.match(value):
                log["residue"].append({"file": rel, "line": i, "reason": f"pipeline II `{value}` is not an integer literal; target pragmas are not macro-expanded, substitute the numeric value"})
            new = f"#pragma hls_pipeline_init_interval {value}"
            anchor = _find_back(out, RE_LOOP.match)
        elif kind == "unroll":
            f = re.search(r"factor\s*=\s*([A-Za-z_0-9]+)", rest, re.IGNORECASE)
            if f:
                value = f.group(1)
                if not RE_INT_LITERAL.match(value):
                    log["residue"].append({"file": rel, "line": i, "reason": f"unroll factor `{value}` is not an integer literal; target pragmas are not macro-expanded, substitute the numeric value"})
                new = f"#pragma hls_unroll {value}"
            else:
                new = "#pragma hls_unroll yes"
            anchor = _find_back(out, RE_LOOP.match)
        elif kind == "inline":
            new = "#pragma hls_design inline"
            anchor = _find_back(out, lambda s: bool(RE_FN_HEADER_END.search(s)) and not RE_LOOP.match(s) and not s.lstrip().startswith("if"))
        else:
            entry["reason"] = "no source-level equivalent in target"
            log["pragmas_dropped"].append(entry)
            continue

        if anchor is None:
            log["residue"].append({"file": rel, "line": i, "reason": f"could not find where to place `{new}` for `{line.strip()}`; place it manually"})
            continue
        indent = out[anchor][: len(out[anchor]) - len(out[anchor].lstrip())]
        out.insert(anchor, f"{indent}{new}")
        entry["after"] = new
        log["pragmas_moved"].append(entry)
    return out


def _insert_top_marker(lines: list[str], top: str, target: TargetSpec, log: dict, rel: str) -> list[str]:
    rx = _fn_def_regex(top)
    for idx, line in enumerate(lines):
        if rx.match(line) and not line.rstrip().endswith(";"):
            lines.insert(idx, target.top_marker)
            log["top_marker"] = {"file": rel, "line": idx + 1, "inserted": True}
            return lines
    return lines


def rewrite_design(dir_in: Path, dir_out: Path, target: TargetSpec, top: str | None) -> dict:
    dir_in, dir_out = Path(dir_in), Path(dir_out)
    dir_out.mkdir(parents=True, exist_ok=True)
    log: dict = {
        "target": target.name,
        "headers": [],
        "types": [],
        "pragmas_moved": [],
        "pragmas_dropped": [],
        "top_marker": {"inserted": False},
        "residue": [],
    }
    header_map = HEADER_MAPS.get(target.name, HEADER_MAPS["catapult"])

    for p in sorted(dir_in.rglob("*")):
        if not p.is_file() or p.name == "synth.tcl":
            continue
        dst = dir_out / p.relative_to(dir_in)
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(p, dst)

    for p in sorted(dir_out.rglob("*")):
        if not p.is_file() or p.suffix.lower() not in SOURCE_SUFFIXES:
            continue
        rel = str(p.relative_to(dir_out))
        lines = p.read_text(encoding="utf-8", errors="replace").splitlines()
        new_lines: list[str] = []
        for i, line in enumerate(lines, start=1):
            m = RE_INCLUDE.match(line)
            if m:
                new = f"{m.group(1)}{m.group(2)}{header_map[m.group(3)]}{m.group(4)}"
                log["headers"].append({"file": rel, "line": i, "before": line.strip(), "after": new.strip()})
                new_lines.append(new)
                continue
            new_lines.append(_rewrite_types(line, target, log, rel, i))
        new_lines = _rewrite_pragmas(new_lines, target, log, rel)
        if top and p.suffix.lower() in (".cpp", ".cc", ".c") and not log["top_marker"]["inserted"]:
            new_lines = _insert_top_marker(new_lines, top, target, log, rel)
        text = "\n".join(new_lines) + "\n"
        for k, l in enumerate(text.splitlines(), start=1):
            if l.lstrip().startswith("//"):
                continue
            for pat in target.forbidden_patterns:
                if re.search(pat, l):
                    log["residue"].append({"file": rel, "line": k, "reason": f"still matches `{pat}`: {l.strip()}"})
                    break
        p.write_text(text, encoding="utf-8")

    (dir_out / "rewrite_log.json").write_text(json.dumps(log, indent=2), encoding="utf-8")
    return log
