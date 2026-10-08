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
from collections import Counter
from pathlib import Path

from hlsfactory_agent.scan import collect_defines, resolve_int
from hlsfactory_agent.translate import SOURCE_SUFFIXES, TargetSpec

HEADER_MAPS: dict[str, dict[str, str]] = {
    "catapult": {"ap_int.h": "ac_int.h", "ap_fixed.h": "ac_fixed.h", "hls_stream.h": "ac_channel.h"},
    "xlscc": {"ap_int.h": "ac_int.h", "ap_fixed.h": "ac_fixed.h", "hls_stream.h": "xls_emu.h"},
}
STREAM_MAPS: dict[str, str] = {"catapult": "ac_channel<", "xlscc": "__xls_channel<"}
MODE_MAP = {
    "AP_TRN": "AC_TRN",
    "AP_TRN_ZERO": "AC_TRN_ZERO",
    "AP_RND": "AC_RND",
    "AP_RND_ZERO": "AC_RND_ZERO",
    "AP_RND_MIN_INF": "AC_RND_MIN_INF",
    "AP_RND_INF": "AC_RND_INF",
    "AP_RND_CONV": "AC_RND_CONV",
    "AP_WRAP": "AC_WRAP",
    "AP_SAT": "AC_SAT",
    "AP_SAT_ZERO": "AC_SAT_ZERO",
    "AP_SAT_SYM": "AC_SAT_SYM",
}

RE_INCLUDE = re.compile(r'^(\s*#\s*include\s*)([<"])(ap_int\.h|ap_fixed\.h|hls_stream\.h)([>"])')
RE_AP_INT = re.compile(r"\bap_int\s*<\s*([^<>,]+?)\s*>")
RE_AP_UINT = re.compile(r"\bap_uint\s*<\s*([^<>,]+?)\s*>")
RE_AP_FIXED = re.compile(
    r"\b(ap_fixed|ap_ufixed)\s*<\s*([^<>,]+?)\s*,\s*([^<>,]+?)\s*(?:,\s*([A-Z_]+)\s*)?(?:,\s*([A-Z_]+)\s*)?>"
)
RE_STREAM = re.compile(r"\bhls::stream\s*<")
RE_CHANNEL_DEPTH = re.compile(r"\bac_channel\s*<\s*([^,<>]*(?:<[^<>]*>)?[^,<>]*)\s*,\s*[^<>]+>")  # hls::stream<T, depth>: the depth is a directive
RE_PRAGMA = re.compile(r"^(\s*)#\s*pragma\s+HLS\s+([A-Za-z_]+)(.*)$", re.IGNORECASE)
RE_LOOP = re.compile(r"^\s*(?:[A-Za-z_]\w*\s*:\s*)?(for|while)\b")
RE_FN_HEADER_END = re.compile(r"\)\s*(const\s*)?\{?\s*$")
# Methods of ap_int/ap_fixed with no same-named ac_int/ac_fixed equivalent. to_int, to_uint, to_double and the
# *_reduce family exist on both families and are left alone.
RE_VENDOR_METHOD = re.compile(r"\.(range|reverse|length|set_bit|get_bit|bit|test|invert|rrotate|lrotate|concat)\s*\(")
RE_INT_LITERAL = re.compile(r"^\d+$")

# Catapult cannot synthesize std::complex (CIN-15), native float arithmetic (CIN-290) or float math calls
# (CIN-16); its front end does not include <stdint.h> transitively (CRD-20); local channels must be static (HIER-6).
DIALECT_TYPES: dict[str, dict[str, str]] = {
    "catapult": {"std::complex": "ac_complex", "complex": "ac_complex", "float": "ac_ieee_float32", "double": "ac_ieee_float64"},
}
DIALECT_HEADERS: dict[str, dict[str, str]] = {"catapult": {"<complex>": "<ac_complex.h>"}}
# Vendor headers with no target equivalent, written next to the design as a local synthesizable stand-in when
# the source includes them. hls::vector is a plain aggregate; hls_fft.h/hls_streamofblocks.h are empty, so
# any use of their APIs still fails the compile check and reaches the agent as residue.
SHIM_HEADERS: dict[str, dict[str, str]] = {
    "catapult": {
        "hls_vector.h": (
            "#pragma once\n"
            "namespace hls {\n"
            "template <typename T, int N> struct vector {\n"
            "    T data[N];\n"
            "    T &operator[](int i) { return data[i]; }\n"
            "    const T &operator[](int i) const { return data[i]; }\n"
            "};\n"
            "}\n"
        ),
        "hls_fft.h": "#pragma once\n",
        "hls_streamofblocks.h": "#pragma once\n",
        "cat_abs.h": "#pragma once\n// Catapult has no synthesizable body for the C library abs(); the pre-pass rewrites abs( to cat_abs(.\ntemplate <typename T> inline T cat_abs(const T &x) { return x < 0 ? T(-x) : x; }\n",
        "ac_float_trig.h": '#pragma once\n// Catapult synthesizes no trig on its float types: fixed-point CORDIC from ac_math, converted at the boundary.\n// Angle in radians, |x| < 2^15; about 1e-9 absolute error. Written next to the design by the translation pre-pass.\n#include <ac_std_float.h>\n#include <ac_math/ac_sincos_cordic.h>\n#include <ac_math/ac_sqrt.h>\nnamespace ac_float_trig {\ntypedef ac_fixed<48, 16, true, AC_RND, AC_SAT> angle_t;\ntypedef ac_fixed<40, 2, true> unit_t;\nstatic const ac_fixed<48, 2, true> INV_PI = 0.31830988618379067154;\ntemplate <typename F> inline F sin_of(const F &x) {\n    angle_t a = x.template convert_to_ac_fixed<48, 16, true, AC_RND, AC_SAT>() * INV_PI;\n    unit_t s;\n    ac_math::ac_sin_cordic(a, s);\n    return F(s);\n}\ntemplate <typename F> inline F cos_of(const F &x) {\n    angle_t a = x.template convert_to_ac_fixed<48, 16, true, AC_RND, AC_SAT>() * INV_PI;\n    unit_t c;\n    ac_math::ac_cos_cordic(a, c);\n    return F(c);\n}\n}\ninline ac_ieee_float32 sin(const ac_ieee_float32 &x) { return ac_float_trig::sin_of(x); }\ninline ac_ieee_float32 cos(const ac_ieee_float32 &x) { return ac_float_trig::cos_of(x); }\ninline ac_ieee_float64 sin(const ac_ieee_float64 &x) { return ac_float_trig::sin_of(x); }\ninline ac_ieee_float64 cos(const ac_ieee_float64 &x) { return ac_float_trig::cos_of(x); }\ninline ac_ieee_float32 sinf(const ac_ieee_float32 &x) { return sin(x); }\ninline ac_ieee_float32 cosf(const ac_ieee_float32 &x) { return cos(x); }\nnamespace ac_float_trig {\ntemplate <typename F> inline F sqrt_of(const F &x) {\n    ac_fixed<48, 16, false, AC_RND, AC_SAT> a = x.template convert_to_ac_fixed<48, 16, true, AC_RND, AC_SAT>();\n    ac_fixed<40, 8, false> r;\n    ac_math::ac_sqrt(a, r);\n    return F(r);\n}\n}\ninline ac_ieee_float32 sqrt(const ac_ieee_float32 &x) { return ac_float_trig::sqrt_of(x); }\ninline ac_ieee_float64 sqrt(const ac_ieee_float64 &x) { return ac_float_trig::sqrt_of(x); }\ninline ac_ieee_float32 sqrtf(const ac_ieee_float32 &x) { return sqrt(x); }\n',
    }
}
RE_ANY_INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')
RE_DIALECT_TYPE = re.compile(r"\b(std::complex|complex|float|double)\b")
RE_CONST_TABLE = re.compile(r"\b(?:const\s+)?(float|double)\b[^=;]*\[[^=;]*=\s*\{|\bconst\s+(float|double)\b[^=;]*\[|\bconstexpr\s+(float|double)\b")  # a literal table or constexpr stays native
RE_MAIN = re.compile(r"\bint\s+main\s*\(")
RE_SDS_PRAGMA = re.compile(r"^\s*#\s*pragma\s+SDS\b", re.I)  # SDSoC pragmas have no Catapult meaning
RE_FLOAT_TYPEDEF = re.compile(r"\btypedef\s+(?:float|double)\s+([A-Za-z_]\w*)\s*;")
RE_FLOAT_MATH = re.compile(r"\b(tan|exp|log|pow|atan2?)f?\s*\(")  # cos/sin come from the ac_float_trig shim
RE_STD_TRIG = re.compile(r"\bstd::((?:cos|sin|sqrt)f?)\s*\(")
RE_ABS = re.compile(r"\b(?:std::)?abs\s*\(")  # the C library abs has no synthesizable body (CIN-16); cat_abs.h replaces it
RE_FLOAT_LITERAL_DECL = re.compile(r"(?<![\w.])(ac_ieee_float(?:32|64))\s+(\w+)\s*=\s*(-?\d+(?:\.\d*)?(?:[eE][-+]?\d+)?[fF]?)\s*;")
RE_UNSIZED_PARAM = re.compile(r"\b([A-Za-z_]\w*)\s*\[\s*(?:/\*[^*]*\*/)?\s*\]")  # `a[]` or `a[/*n*/]`
RE_BIT_RANGE = re.compile(r"\.range\(\s*([^,()]+?)\s*,\s*\1\s*\)")  # x.range(i, i) is the single bit x[i]
# `ac_ieee_floatNN v = <native arithmetic>;` — Catapult cannot convert a folded constant to double (CIN-291),
# so each operand is wrapped in the float type and the arithmetic happens on it. Binary64 only: it keeps the C
# double semantics; a float32 declaration would change precision and is left as residue.
RE_FLOAT_DECL = re.compile(r"^(\s*)(ac_ieee_float(?:32|64))(\s+\w+\s*=\s*)([^;(){}\[\],\"']*[-+*/][^;(){}\[\],\"']*);(\s*)$")
RE_OPERAND = re.compile(r"\b(\d+\.?\d*(?:[eE][-+]?\d+)?|[A-Za-z_]\w*)\b")
RE_TYPED_LITERAL = re.compile(r"\d\.?\d*[fFuUlL]\b")
# a local channel, possibly an array, possibly with the hls::stream name argument ac_channel has no constructor for
RE_LOCAL_CHANNEL = re.compile(r"^(\s+)(ac_channel\s*<[^;&()]*>\s+[A-Za-z_]\w*(?:\s*\[[^\]]+\])*)\s*(?:\(\s*\"[^\"]*\"\s*\))?\s*;")
LOOKBACK_LINES = 3
SIGNATURE_LINES = 8


RE_DATAFLOW = re.compile(r"#\s*pragma\s+HLS\s+dataflow\b", re.I)
RE_INLINE_ON = re.compile(r"#\s*pragma\s+HLS\s+inline\s*(?://.*)?$", re.I | re.M)
RE_FN_DEF = re.compile(r"(?m)^[ \t]*(?:template\s*<[^>]*>\s*)?[A-Za-z_][\w:<>,\s\*&]*?\b([A-Za-z_]\w*)\s*\(([^;{}]*)\)\s*\{")
BLOCK_MARKER = "#pragma hls_design block"
KEYWORDS = {"if", "for", "while", "switch", "else", "return", "catch", "sizeof"}


def _has_memory_param(params: str) -> bool:
    """An array or pointer parameter, with template argument lists removed so `UF*2` is not a pointer."""
    while (stripped := re.sub(r"<[^<>]*>", "", params)) != params:
        params = stripped
    return re.search(r"[\[*]", params) is not None


def _body_end(text: str, start: int) -> int:
    depth = 0
    for j in range(text.index("{", start), len(text)):
        depth += text[j] == "{"
        depth -= text[j] == "}"
        if depth == 0:
            return j
    return len(text)


def dataflow_blocks(dir_in: Path, top: str | None) -> set[str]:
    """Functions called from a `#pragma HLS dataflow` region that take only channels and scalars. Catapult
    blocks may share nothing but channels (ASM-35), so a stage that receives an array stays inline; a stage
    with its own `#pragma HLS inline` stays inline too."""
    blocks: set[str] = set()
    for p in sorted(Path(dir_in).rglob("*")):
        if not p.is_file() or p.suffix.lower() not in SOURCE_SUFFIXES or p.name == "testbench.cpp":
            continue
        text = p.read_text(encoding="utf-8", errors="replace")
        defs = {m.group(1): (m.start(), m.group(2)) for m in RE_FN_DEF.finditer(text) if m.group(1) not in KEYWORDS}
        bodies = {n: text[s : _body_end(text, s) + 1] for n, (s, _) in defs.items()}
        for m in RE_DATAFLOW.finditer(text):
            inside = [n for n, (s, _) in defs.items() if s < m.start() < s + len(bodies[n])]
            if not inside:
                continue
            region = max(inside, key=lambda n: defs[n][0])
            for c in re.findall(r"\b([A-Za-z_]\w*)\s*(?:<[^;()]*>)?\s*\(", bodies[region]):
                if c in defs and c not in (region, top) and not _has_memory_param(defs[c][1]) and not RE_INLINE_ON.search(bodies[c]):
                    blocks.add(c)
    return blocks


def _insert_block_markers(lines: list[str], blocks: set[str], log: dict, rel: str) -> list[str]:
    for name in sorted(blocks):
        rx = _fn_def_regex(name)
        for idx in range(len(lines)):
            if not rx.match("\n".join(lines[idx : idx + SIGNATURE_LINES])):
                continue
            while idx < len(lines) - 1 and not lines[idx].strip():
                idx += 1
            at = idx - 1 if idx and lines[idx - 1].lstrip().startswith("template") else idx
            if at and lines[at - 1].lstrip().startswith("#pragma hls_design"):
                break
            lines.insert(at, BLOCK_MARKER)
            log["blocks"].append({"file": rel, "line": at + 1, "function": name})
            break
    return lines


def _fn_def_regex(top: str) -> re.Pattern:
    # a definition: return type, the top name, an argument list that may span lines, then the opening brace
    return re.compile(r"^\s*[A-Za-z_][\w:<>,\s\*&]*?\b" + re.escape(top) + r"\s*\([^;{}]*\)\s*\{")


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
    line = RE_CHANNEL_DEPTH.sub(r"ac_channel<\1>", line)  # ac_channel takes one template argument; FIFO_DEPTH comes from run.tcl
    if line != before:
        log["types"].append({"file": rel, "line": i, "before": before.strip(), "after": line.strip()})
    m = RE_VENDOR_METHOD.search(line)
    if m and not line.lstrip().startswith("//") and not (m.group(0).startswith(".range") and RE_BIT_RANGE.search(line)):
        # a single-bit .range(i, i) is rewritten mechanically later and is not residue
        log["residue"].append({"file": rel, "line": i, "text": line.strip(), "reason": f"vendor method needs manual translation: {m.group(0)}"})
    return line


def float_typedefs(dir_in: Path) -> set[str]:
    """Names typedef'd to float/double in the DUT: declarations of these types need the literal rule too."""
    names: set[str] = set()
    for p in sorted(Path(dir_in).rglob("*")):
        if p.is_file() and p.suffix.lower() in SOURCE_SUFFIXES and p.name != "testbench.cpp":
            names |= set(RE_FLOAT_TYPEDEF.findall(p.read_text(encoding="utf-8", errors="replace")))
    return names


def _rewrite_dialect(lines: list[str], target: TargetSpec, log: dict, rel: str, testbench: bool) -> list[str]:
    """Constructs the target's synthesizer rejects but a host compiler accepts. The testbench keeps host
    types (it is compiled, never synthesized); mixing at the DUT boundary is left to the agent as residue."""
    types = DIALECT_TYPES.get(target.name, {})
    if not types:
        return lines
    out: list[str] = []
    for i, line in enumerate(lines, start=1):
        code = line.split("//", 1)[0]
        if testbench:
            if log.get("dialect_dut_changed") and RE_DIALECT_TYPE.search(code):
                log["residue"].append({"file": rel, "line": i, "reason": "testbench uses std::complex/float; the DUT now uses ac_complex/ac_ieee_float32, convert at the boundary with .real()/.imag() or a cast"})
                break
            continue
        for h, new_h in DIALECT_HEADERS.get(target.name, {}).items():
            if re.match(r"\s*#\s*include\s*" + re.escape(h), line):
                line = line.replace(h, new_h)
        directive = line.lstrip().startswith("#") and not re.match(r"\s*#\s*define\b", line)  # a #define'd type is a type
        if not RE_CONST_TABLE.search(code) and not directive:
            new = RE_STD_TRIG.sub(r"\1(", RE_DIALECT_TYPE.sub(lambda m: types[m.group(1)], line))
            if new != line:
                log["types"].append({"file": rel, "line": i, "before": line.strip(), "after": new.strip()})
                log["dialect_dut_changed"] = True
                line = new
        m = RE_LOCAL_CHANNEL.match(line)
        if m:
            line = f"{m.group(1)}static {m.group(2)};"
            log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
        if RE_BIT_RANGE.search(code):
            line = RE_BIT_RANGE.sub(r"[\1]", line)
            log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
        if RE_ABS.search(code):
            line = RE_ABS.sub("cat_abs(", line)
            log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
        lit_rx = log.get("_float_literal_rx") or RE_FLOAT_LITERAL_DECL
        if lit_rx.search(line.split("//", 1)[0]) and not line.lstrip().startswith("#"):  # after the type rewrite
            # the float types' constructors are explicit: copy-initialization from a literal does not compile (CRD-415)
            line = lit_rx.sub(r"\1 \2(\3);", line)
            log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
        if RE_SDS_PRAGMA.match(line):
            log["pragmas_dropped"].append({"file": rel, "line": i, "kind": "sds", "before": line.strip(), "category": "absent", "reason": "SDSoC pragma, no target equivalent"})
            continue
        m = RE_FLOAT_DECL.match(line)
        if m and not line.lstrip().startswith("#"):
            if m.group(2) == "ac_ieee_float64" and not RE_TYPED_LITERAL.search(m.group(4)):
                rhs = RE_OPERAND.sub(lambda o: f"{m.group(2)}({o.group(1)})", m.group(4))
                line = f"{m.group(1)}{m.group(2)}{m.group(3)}{rhs};{m.group(5)}"
                log["types"].append({"file": rel, "line": i, "before": lines[i - 1].strip(), "after": line.strip()})
            else:
                log["residue"].append({"file": rel, "line": i, "text": line.strip(), "reason": f"native float arithmetic feeding a {m.group(2)} is not synthesizable in {target.display_name} (CIN-291): wrap each operand in {m.group(2)}(...) or fold it to a table"})
        if RE_FLOAT_MATH.search(code):
            log["residue"].append({"file": rel, "line": i, "text": line.strip(), "reason": f"no synthesizable float math in {target.display_name}: {RE_FLOAT_MATH.search(code).group(0)}) — constant-fold to a table or use a fixed-point ac_math function (cos/sin are provided by ac_float_trig.h)"})
        out.append(line)
    if testbench:
        return lines
    text = "\n".join(out)
    for needed, inc in reversed(target.required_includes):
        if re.search(needed, text) and inc not in text and inc.replace("<stdint.h>", "<cstdint>") not in text:
            out.insert(0, inc)
            log["headers"].append({"file": rel, "line": 1, "before": "", "after": inc})
    return out


def _find_back(out: list[str], predicate) -> int | None:
    j = len(out) - 1
    while j >= 0 and j >= len(out) - LOOKBACK_LINES:
        if predicate(out[j]):
            return j
        j -= 1
    return None


def _category_for(kind: str, rest: str, target: TargetSpec) -> str:
    kind_l = kind.lower()
    has_factor = "factor" in rest.lower()
    for rule in target.pragma_rules:
        src = rule.source.lower()
        if f"hls {kind_l}" not in src:
            continue
        if kind_l == "unroll" and ("factor" in src) != has_factor:
            continue
        return rule.category
    return "absent"


def _rewrite_pragmas(lines: list[str], target: TargetSpec, log: dict, rel: str, defines: dict[str, str] | None = None) -> list[str]:
    out: list[str] = []
    defines = defines or {}
    for i, line in enumerate(lines, start=1):
        m = RE_PRAGMA.match(line)
        if not m:
            out.append(line)
            continue
        kind, rest = m.group(2).lower(), m.group(3)
        entry = {"file": rel, "line": i, "kind": kind, "before": line.strip(), "category": _category_for(kind, rest, target)}

        if kind == "pipeline":
            if re.search(r"\boff\b", rest, re.IGNORECASE):
                entry["reason"] = "pipeline off has no target equivalent"
                log["pragmas_dropped"].append(entry)
                continue
            ii = re.search(r"II\s*=\s*([A-Za-z_0-9]+)", rest, re.IGNORECASE)
            value = ii.group(1) if ii else "1"
            if not RE_INT_LITERAL.match(value) and resolve_int(value, defines) is not None:
                value = str(resolve_int(value, defines))  # target pragmas are not macro-expanded; a #define resolves mechanically
            new = f"#pragma hls_pipeline_init_interval {value}"
            if not RE_INT_LITERAL.match(value):
                log["residue"].append({"file": rel, "line": i, "text": new, "reason": f"pipeline II `{value}` is not an integer literal; target pragmas are not macro-expanded, substitute the numeric value"})
            anchor = _find_back(out, RE_LOOP.match)
        elif kind == "unroll":
            f = re.search(r"factor\s*=\s*([A-Za-z_0-9]+)", rest, re.IGNORECASE)
            if f:
                value = f.group(1)
                if not RE_INT_LITERAL.match(value) and resolve_int(value, defines) is not None:
                    value = str(resolve_int(value, defines))
                new = f"#pragma hls_unroll {value}"
                if not RE_INT_LITERAL.match(value):
                    log["residue"].append({"file": rel, "line": i, "text": new, "reason": f"unroll factor `{value}` is not an integer literal; target pragmas are not macro-expanded, substitute the numeric value"})
            else:
                new = "#pragma hls_unroll yes"
            anchor = _find_back(out, RE_LOOP.match)
        elif kind == "inline":
            new = "#pragma hls_design inline"
            code_of = lambda s: s.split("//", 1)[0].rstrip()  # `) {  // L90`
            anchor = _find_back(out, lambda s: bool(RE_FN_HEADER_END.search(code_of(s))) and not RE_LOOP.match(s) and not s.lstrip().startswith("if"))
            if anchor is not None:  # a signature that spans lines: the pragma goes before its first line, not inside the parameter list
                depth = 0
                for j in range(anchor, -1, -1):
                    depth += code_of(out[j]).count(")") - code_of(out[j]).count("(")
                    if depth <= 0 and "(" in code_of(out[j]):
                        anchor = j
                        break
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
    return _drop_pipeline_on_unrolled(out, log, rel)


def _drop_pipeline_on_unrolled(out: list[str], log: dict, rel: str) -> list[str]:
    """Vitis accepts PIPELINE together with a complete UNROLL; Catapult refuses to pipeline a fully unrolled loop,
    so the pipeline pragma is dropped from any pragma block that also fully unrolls."""
    is_pragma = lambda s: s.lstrip().startswith("#pragma hls_")
    drop: set[int] = set()
    for i, line in enumerate(out):
        if line.strip() != "#pragma hls_unroll yes":
            continue
        j = i
        while j > 0 and is_pragma(out[j - 1]):
            j -= 1
        k = i
        while k + 1 < len(out) and is_pragma(out[k + 1]):
            k += 1
        drop.update(n for n in range(j, k + 1) if out[n].lstrip().startswith("#pragma hls_pipeline_init_interval"))
    for n in sorted(drop):
        log["pragmas_dropped"].append({"file": rel, "line": n + 1, "kind": "pipeline", "before": out[n].strip(), "category": "lossy",
                                       "reason": "a fully unrolled loop cannot be pipelined in Catapult"})
    return [l for n, l in enumerate(out) if n not in drop]


def _size_unsized_params(dir_out: Path, top: str, log: dict) -> None:
    """`T x[]` parameters of the top have no size Catapult can use (CIN-84, it assumes 1024 words); the size
    comes from the array the testbench passes at the call site, resolved through the testbench's defines."""
    from hlsfactory_agent.scan import array_dims, top_signature

    sig = top_signature(dir_out, top)
    tb = dir_out / "testbench.cpp"
    if not sig or not tb.exists():
        return
    params = [p.strip() for p in re.split(r",(?![^<]*>)", sig) if p.strip()]
    tb_text = tb.read_text(encoding="utf-8", errors="replace")
    args: list[str] = []
    for call in re.finditer(r"\b" + re.escape(top) + r"\s*\(([^;{}]*)\)\s*;", tb_text):  # the call, not a prototype
        cand = [a.strip().lstrip("&*") for a in re.split(r",(?![^()]*\))", call.group(1))]
        if not any("[" in a or re.match(r"(const|unsigned|signed|int|double|float|char|bool|short|long|void|ac_|ap_|std::)\b", a) for a in cand):
            args = cand
            break
    if not args:
        return
    defines = collect_defines(dir_out)
    # `int n = 1024;` in the testbench resolves like a #define for the heap idioms below
    locals_ = {m.group(1): m.group(2) for m in re.finditer(r"\b(?:const\s+)?(?:int|unsigned|size_t|long)\s+([A-Za-z_]\w*)\s*=\s*([^;(){}]+?)\s*;", tb_text)}
    sizes: dict[str, int] = {}
    for k, p in enumerate(params):
        m = RE_UNSIZED_PARAM.search(p)
        if not (m and k < len(args) and re.fullmatch(r"[A-Za-z_]\w*", args[k])):
            continue
        dims = array_dims(dir_out, args[k], defines)
        if dims and len(dims) == 1:
            sizes[m.group(1)] = dims[0]
            continue
        heap = re.search(r"\b" + re.escape(args[k]) + r"\s*=\s*(?:\([^()]*\)\s*)?(?:malloc\s*\(\s*([^;]*?)\s*\*\s*sizeof|new\s+[\w:<>]+\s*\[\s*([^\]]+?)\s*\])", tb_text)
        if heap:
            n = resolve_int(heap.group(1) or heap.group(2), {**defines, **locals_})
            if n:
                sizes[m.group(1)] = n
    if not sizes:
        return
    for p in sorted(dir_out.rglob("*")):
        if not p.is_file() or p.suffix.lower() not in SOURCE_SUFFIXES or p.name == "testbench.cpp":
            continue
        text = p.read_text(encoding="utf-8", errors="replace")
        new = RE_UNSIZED_PARAM.sub(lambda m: f"{m.group(1)}[{sizes[m.group(1)]}]" if m.group(1) in sizes else m.group(0), text)
        if new != text:
            p.write_text(new, encoding="utf-8")
            log["types"].append({"file": str(p.relative_to(dir_out)), "line": 0, "before": "unsized top parameters " + ", ".join(sizes), "after": ", ".join(f"{k}[{v}]" for k, v in sizes.items())})


def _relocate_residue(log: dict, rel: str, out: list[str]) -> None:
    """Insertions shift lines; a residue entry that recorded its text is re-pointed at that text's nearest copy."""
    for r in log["residue"]:
        if r.get("file") != rel or not r.get("text"):
            continue
        hits = [n for n, l in enumerate(out, start=1) if l.strip() == r["text"]]
        if hits:
            r["line"] = min(hits, key=lambda n: abs(n - int(r["line"])))


def _insert_top_marker(lines: list[str], top: str, target: TargetSpec, log: dict, rel: str) -> list[str]:
    rx = _fn_def_regex(top)
    for idx, line in enumerate(lines):
        if rx.match("\n".join(lines[idx : idx + SIGNATURE_LINES])):
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
        "blocks": [],
        "residue": [],
    }
    header_map = HEADER_MAPS.get(target.name, HEADER_MAPS["catapult"])
    blocks = dataflow_blocks(dir_in, top) if target.name == "catapult" else set()
    defines = collect_defines(dir_in)
    names = float_typedefs(dir_in) if target.name == "catapult" else set()
    if names:  # `const w_dt v = 1.0;` with `typedef float w_dt` needs direct initialization like the float types themselves
        log["_float_literal_rx"] = re.compile(r"(?<![\w.])((?:const\s+)?(?:ac_ieee_float(?:32|64)|" + "|".join(map(re.escape, sorted(names))) + r"))\s+(\w+)\s*=\s*(-?\d+(?:\.\d*)?(?:[eE][-+]?\d+)?[fF]?)\s*;")

    for p in sorted(dir_in.rglob("*")):
        if not p.is_file() or p.name == "synth.tcl":
            continue
        if p.suffix.lower() in SOURCE_SUFFIXES and p.name != "testbench.cpp" and RE_MAIN.search(p.read_text(encoding="utf-8", errors="replace")):
            log["pragmas_dropped"].append({"file": str(p.relative_to(dir_in)), "line": 0, "kind": "file", "before": "second main()", "category": "absent",
                                           "reason": "a second test program next to testbench.cpp cannot be linked; not copied"})
            continue
        dst = dir_out / p.relative_to(dir_in)
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(p, dst)

    if top and target.name == "catapult":
        _size_unsized_params(dir_out, top, log)

    for p in sorted(dir_out.rglob("*"), key=lambda q: (q.name == "testbench.cpp", str(q))):
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
        new_lines = _rewrite_dialect(new_lines, target, log, rel, testbench=p.name == "testbench.cpp")
        for line in new_lines:
            m = RE_ANY_INCLUDE.match(line)
            shim = SHIM_HEADERS.get(target.name, {}).get(m.group(1)) if m else None
            if shim is not None and not (dir_out / m.group(1)).exists():
                (dir_out / m.group(1)).write_text(shim, encoding="utf-8")
                log["headers"].append({"file": m.group(1), "line": 0, "before": "", "after": "shim written"})
        new_lines = _rewrite_pragmas(new_lines, target, log, rel, defines)
        if top and p.suffix.lower() in (".cpp", ".cc", ".c") and not log["top_marker"]["inserted"]:
            new_lines = _insert_top_marker(new_lines, top, target, log, rel)
        if blocks and p.name != "testbench.cpp":
            new_lines = _insert_block_markers(new_lines, blocks, log, rel)
        _relocate_residue(log, rel, new_lines)
        text = "\n".join(new_lines) + "\n"
        for k, l in enumerate(text.splitlines(), start=1):
            if l.lstrip().startswith("//"):
                continue
            for pat in target.forbidden_patterns + (() if p.name == "testbench.cpp" else target.dut_forbidden_patterns):
                if re.search(pat, l):
                    log["residue"].append({"file": rel, "line": k, "reason": f"still matches `{pat}`: {l.strip()}"})
                    break
        p.write_text(text, encoding="utf-8")

    log.pop("_float_literal_rx", None)
    log["category_counts"] = dict(Counter(e["category"] for e in log["pragmas_moved"] + log["pragmas_dropped"]))
    (dir_out / "rewrite_log.json").write_text(json.dumps(log, indent=2), encoding="utf-8")
    return log
