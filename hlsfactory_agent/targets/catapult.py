from __future__ import annotations

import json
import math
import os
import re
import shutil
from pathlib import Path

from hlsfactory_agent.spec import PragmaRule, SynthResult, TargetSpec

DIR_PKG = Path(__file__).resolve().parent.parent
DIR_AC_TYPES_INCLUDE = DIR_PKG / "ac_types_include"
# ac_math and the rest of the tool's headers come from the Catapult install, never vendored; without it the
# trig shim does not compile and the container check says so
DIR_CATAPULT_INCLUDE = Path(os.environ.get("MGC_HOME", "/tools/software/siemens/catapult/latest/Mgc_home")) / "shared" / "include"
DIR_TARGETS = DIR_PKG / "targets"


def load_directive_rules(target_name: str) -> dict[str, str]:
    f = DIR_TARGETS / f"{target_name}_directives.json"
    return json.loads(f.read_text(encoding="utf-8")) if f.exists() else {}


def directive_template(resource: dict, rules: dict[str, str]) -> str | None:
    """Most specific matching rule: kind.type.scope, then kind.type, then kind."""
    args = resource.get("args", {})
    keys = []
    if args.get("type"):
        keys.append(f"{resource['kind']}.{args['type'].lower()}.{resource.get('scope', 'unknown')}")
        keys.append(f"{resource['kind']}.{args['type'].lower()}")
    keys.append(resource["kind"])
    return next((rules[k] for k in keys if k in rules), None)


REGISTER_THRESHOLD = 256  # Catapult default; a larger array cannot map to [Register] (MEM-8)
MEM_MAP_THRESHOLD = 32  # Catapult default; a larger array becomes a memory, mapped to the 1-read-port ccs_ram_sync_1R1W


def _partition_form(r: dict) -> tuple[str | None, int | None, int | None, str | None]:
    """Vitis splits one dimension of a row-major array; Catapult sees it flattened, so an outer-dimension
    split is BLOCK_SIZE and an innermost-dimension split is INTERLEAVE. Returns (type, factor, block_size, reason)."""
    args = r.get("args", {})
    kind, factor = args.get("type"), r.get("factor")
    dims = r.get("dims") or ([r["size"]] if r.get("size") else [])
    dim = int(args["dim"]) if str(args.get("dim", "")).isdigit() else 0
    size = math.prod(dims) if dims else None
    if kind == "complete" and (dim == 0 or len(dims) <= 1):
        if r.get("scope") == "local" and size is None:
            return kind, None, None, f"array size unknown, cannot check Catapult REGISTER_THRESHOLD {REGISTER_THRESHOLD}"
        if r.get("scope") == "local" and size > REGISTER_THRESHOLD:
            return kind, None, None, f"{size} elements exceed Catapult REGISTER_THRESHOLD {REGISTER_THRESHOLD}"
        return kind, None, None, None
    if len(dims) <= 1 or dim == 0:
        if kind == "block":
            if not size or not factor:
                return kind, None, None, "block partition needs both array size and factor"
            return kind, factor, max(1, -(-size // factor)), None
        return kind, factor, None, None
    inner = math.prod(dims[1:])
    if dim == 1:
        if kind == "complete":
            return "block", None, inner, None
        if kind == "block" and factor:
            return "block", factor, max(1, -(-dims[0] // factor)) * inner, None
    if dim == len(dims):
        if kind == "complete":
            return "cyclic", dims[-1], None, None
        if kind == "cyclic" and factor and dims[-1] % factor == 0:
            return "cyclic", factor, None, None
    return kind, None, None, f"{kind} partition of dim {dim} of a {'x'.join(map(str, dims))} array has no flat equivalent"


def render_directives(resources: list[dict], top: str, rules: dict[str, str], arrays: list[dict] = ()) -> tuple[list[str], list[dict]]:
    """Resource pragmas become directive lines; then every array Vitis would have put in a dual-port BRAM
    (no BIND_STORAGE, not const, larger than MEM_MAP_THRESHOLD) gets the dual-port module Catapult does not
    pick by default, so a schedule that needed two ports in Vitis stays feasible."""
    lines: list[str] = []
    unrendered: list[dict] = []
    for r in resources:
        args = r.get("args", {})
        if directive_template(r, rules) is None:
            unrendered.append({"resource": r, "reason": "no directive rule"})
            continue
        if r.get("unresolved"):
            unrendered.append({"resource": r, "reason": f"unresolved factor {r['unresolved']}"})
            continue
        kind, factor, block_size, reason = _partition_form(r) if r["kind"] == "array_partition" else (args.get("type"), None, None, None)
        if r["kind"] == "stream" and not str(args.get("depth", "")).isdigit():
            reason = "stream depth unknown"
        if reason:
            unrendered.append({"resource": r, "reason": reason})
            continue
        template = directive_template({**r, "args": {**args, "type": kind}}, rules)
        if template is None:
            unrendered.append({"resource": r, "reason": f"no directive rule for {kind} form"})
            continue
        lines.append(
            template.format(
                top=top,
                var=args.get("variable") or args.get("port") or "",
                factor=factor if factor is not None else "",
                block_size=block_size if block_size is not None else "",
                dim=args.get("dim", "1"),
                depth=args.get("depth", ""),
                impl=args.get("impl", ""),
                mode=args.get("mode", ""),
                port=args.get("port", ""),
            )
        )
    bound = {r["args"].get("variable") for r in resources if r["kind"] == "bind_storage"}
    registered = {l.split()[1] for l in lines if "[Register]" in l}
    for a in arrays:
        if a["var"] in bound or a["var"] in registered or a.get("const") or not a.get("size") or a["size"] <= MEM_MAP_THRESHOLD:
            continue
        if "array.default_memory" in rules:
            lines.append(rules["array.default_memory"].format(var=a["var"]))
    return lines, unrendered


# first match wins; keys are message fragments from catapult.log
FAILURE_CLASSES = (
    ("License request failed", "license"),
    ("ASM-35", "shared-array"),
    ("ASM-34", "unsupported-construct"),
    ("Logic mixed with interconnect", "logic-in-interconnect"),
    ("ASM-59", "logic-in-interconnect"),
    ("SCHD-20", "schedule-delay"),
    ("insufficient resources", "insufficient-memory-ports"),
    ("HIER-47", "channel-fifo"),
    ("CRAAS-6", "no-library-component"),
    ("Cannot pipeline a fully unrolled loop", "pipeline-on-unrolled-loop"),
    ("CIN-71", "pointer-cast"),
    ("CIN-84", "unsized-array"),
    ("CIN-290", "float-op"),
    ("CIN-15", "unsupported-type"),
    ("CRD-2388", "constexpr-body"),
    ("CRD-20", "undefined-identifier"),
    ("CIN-16", "missing-function"),
    ("HIER-6", "non-static-channel"),
)


def classify_catapult_error(log: str) -> str:
    return next((name for key, name in FAILURE_CLASSES if key in log), "other")


def parse_catapult_report(dir_run: Path) -> SynthResult:
    f_log = dir_run / "catapult.log"
    log = f_log.read_text(encoding="utf-8", errors="replace") if f_log.exists() else ""
    lines = log.splitlines()
    errors = [l for l in lines if l.startswith("# Error")]
    rpts = sorted(dir_run.glob("Catapult/*/rtl.rpt"), key=lambda p: p.stat().st_mtime)
    hits = {m.group(1): int(m.group(2)) for m in re.finditer(r"^# DIRECTIVE (\S+) hits=(\d+)", log, re.M)}
    if not rpts or any(l.startswith("# SYNTH_ERROR") for l in lines):
        return SynthResult(
            "FAILED",
            first_error=errors[0][:300] if errors else "",
            failure_class=classify_catapult_error(log),
            directives=hits,
        )
    rpt = rpts[-1].read_text(encoding="utf-8", errors="replace")
    total = re.search(r"Design Total: +\d+ +(\d+) +(\d+)", rpt)
    area = re.search(r"Total Area Score: +[\d.]+ +[\d.]+ +([\d.]+)", rpt)
    return SynthResult(
        "PASS",
        latency=int(total.group(1)) if total else None,
        throughput=int(total.group(2)) if total else None,
        area=float(area.group(1)) if area else None,
        directives=hits,
    )


RE_MIN_II = re.compile(r"Minimum II of '(\d+)' is required")
# a feedback loop that misses its budget at the tried II: scaling the II by delay/budget is the smallest that fits
RE_FAILED_LOOP = re.compile(r"Failed Loop: II=(\d+).*?Maximum allowed budget=([\d.]+)ns\s+Sequential Delay of loop\(ns\)=([\d.]+)ns")
RE_PORTS = re.compile(r"(\d+) are needed, but only (\d+) instances are available \(SCHD-4\)")
RE_MERGE = re.compile(r"please merge accesses to '(?:[^']*:)?([A-Za-z_]\w*)[^':]*:rsc")  # the array is the last scope segment: 'A::f:buf.data:rsc' names buf
RE_II_PRAGMA = re.compile(r"(#pragma\s+hls_pipeline_init_interval\s+)(\d+)")
RE_UNROLLED_PIPE = re.compile(r"\$PROJECT_HOME/([^()\s]+)\((\d+)\): Cannot pipeline a fully unrolled loop")
SYNTH_OUTPUTS = ("Catapult", "catapult.log", "synth_stdout.log", "synth.tcl", "synth.json")


def catapult_degrade_step(dir_failed: Path, dir_next: Path) -> str | None:
    """Recoveries the log itself parameterizes: SCHD-20 names the minimum II, so every tighter pipeline
    pragma is raised to it; SCHD-4 names how many memory ports were needed and SCHD-39 the array, so that
    array is split by the shortfall, or, once it is already split, the loops reading it get a longer II.
    Nothing else is touched; a log without either yields None."""
    log = (dir_failed / "catapult.log").read_text(encoding="utf-8", errors="replace") if (dir_failed / "catapult.log").exists() else ""
    iis = [int(x) for x in RE_MIN_II.findall(log)]
    iis += [math.ceil(int(ii) * float(delay) / float(budget)) for ii, budget, delay in RE_FAILED_LOOP.findall(log) if float(delay) > float(budget)]
    ports = RE_PORTS.search(log)
    merge = RE_MERGE.search(log)
    unrolled = sorted({(f, int(n)) for f, n in RE_UNROLLED_PIPE.findall(log)})
    if not iis and not (ports and merge) and not unrolled:
        return None
    shutil.copytree(dir_failed, dir_next, ignore=shutil.ignore_patterns(*SYNTH_OUTPUTS))
    if unrolled:
        # the loop Catapult names is fully unrolled after constant propagation: drop the pipeline pragma in the
        # pragma block right above it (the source-level rule only sees `hls_unroll yes`)
        removed = 0
        for f, n in unrolled:
            p = dir_next / f
            if not p.exists():
                continue
            lines = p.read_text(encoding="utf-8", errors="replace").splitlines()
            k = n - 2
            while k >= 0 and (lines[k].lstrip().startswith("#pragma hls_") or re.match(r"\s*[A-Za-z_]\w*\s*:\s*$", lines[k])):
                if lines[k].lstrip().startswith("#pragma hls_pipeline_init_interval"):
                    del lines[k]
                    removed += 1
                    break
                k -= 1
            p.write_text("\n".join(lines) + "\n", encoding="utf-8")
        return f"unpipeline:{removed}"
    if iis:
        target_ii = max(iis)
        for p in dir_next.rglob("*"):
            if not p.is_file() or p.suffix.lower() not in (".cpp", ".cc", ".c", ".h", ".hpp"):
                continue
            text = p.read_text(encoding="utf-8", errors="replace")
            new = RE_II_PRAGMA.sub(lambda m: f"{m.group(1)}{max(int(m.group(2)), target_ii)}", text)
            if new != text:
                p.write_text(new, encoding="utf-8")
        return f"relax_ii:{target_ii}"
    var, needed, have = merge.group(1), int(ports.group(1)), int(ports.group(2))
    tcl = dir_next / "run.tcl"
    lines = tcl.read_text(encoding="utf-8").splitlines()
    if any(l.startswith(f"resource_directive {var} -") and ("-INTERLEAVE" in l or "-BLOCK_SIZE" in l) for l in lines):
        # the array is already split and still short of ports: the accesses collide in one bank (a strided or
        # data-dependent index), which no further split fixes; the loops reading it get a longer II instead
        factor = -(-needed // have)
        n = _relax_ii_on_loops_using(dir_next, var, factor)
        if n:
            return f"relax_ii_for:{var}:x{factor}:{n}"
    if not any(l.startswith("proc resource_directive") for l in lines):
        lines[lines.index("go architect"):lines.index("go architect")] = RESOURCE_DIRECTIVE_PROC.replace("{top}", _top_name(dir_next) or "*").splitlines()
    # interleaving by the port shortfall; a row-wise block split was tried on 2-D arrays and rescued none of 6 (census3)
    size, step = -(-needed // have), "interleave"
    prev = next((i for i, l in enumerate(lines) if l.startswith(f"resource_directive {var} -INTERLEAVE ")), None)
    if prev is not None:  # a repeat round compounds the split instead of adding a duplicate line
        size *= int(lines[prev].rsplit(" ", 1)[1])
        lines[prev] = f"resource_directive {var} -INTERLEAVE {size}"
    else:
        lines.insert(lines.index("go architect"), f"resource_directive {var} -INTERLEAVE {size}")
    tcl.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return f"{step}:{var}:{size}"


RE_TOP_FN = re.compile(r"#pragma\s+hls_design\s+top\s*\n[^(;{]*?\b([A-Za-z_]\w*)\s*\(")


def _top_name(d: Path) -> str | None:
    for p in sorted(d.rglob("*")):
        if p.is_file() and p.suffix.lower() in (".cpp", ".cc", ".c", ".h", ".hpp"):
            m = RE_TOP_FN.search(p.read_text(encoding="utf-8", errors="replace"))
            if m:
                return m.group(1)
    return None


def _loop_body(text: str, start: int) -> tuple[int, int] | None:
    """Span of the body of the first `for`/`while` at or after `start`: braces, or one statement."""
    m = re.compile(r"\b(for|while)\s*\(").search(text, start)
    if not m:
        return None
    k, depth = m.end(), 1
    while k < len(text) and depth:
        depth += {"(": 1, ")": -1}.get(text[k], 0)
        k += 1
    j = len(text) - len(text[k:].lstrip())
    if j < len(text) and text[j] == "{":
        e, depth = j + 1, 1
        while e < len(text) and depth:
            depth += {"{": 1, "}": -1}.get(text[e], 0)
            e += 1
        return j, e
    e = text.find(";", j)
    return (j, e + 1) if e >= 0 else None


def _relax_ii_on_loops_using(d: Path, var: str, factor: int) -> int:
    """Multiply the II of every pipelined loop whose body indexes `var`; returns how many were changed."""
    changed = 0
    use = re.compile(r"\b" + re.escape(var) + r"\s*\[")
    for p in sorted(d.rglob("*")):
        if not p.is_file() or p.suffix.lower() not in (".cpp", ".cc", ".c", ".h", ".hpp") or p.name == "testbench.cpp":
            continue
        text = p.read_text(encoding="utf-8", errors="replace")
        out, last = [], 0
        for m in RE_II_PRAGMA.finditer(text):
            span = _loop_body(text, m.end())
            if span and use.search(text, span[0], span[1]):
                out += [text[last:m.start(2)], str(int(m.group(2)) * factor)]
                last = m.end(2)
                changed += 1
        if last:
            p.write_text("".join(out) + text[last:], encoding="utf-8")
    return changed


CATAPULT = TargetSpec(
    name="catapult",
    display_name="Siemens Catapult HLS",
    include_dir=DIR_AC_TYPES_INCLUDE,
    include_dir_name="ac_types_include",
    extra_include_dirs=((DIR_CATAPULT_INCLUDE, "catapult_include"),) if DIR_CATAPULT_INCLUDE.is_dir() else (),
    cxx_std_check="c++17",
    header_map={
        "ap_int.h": "ac_int.h",
        "ap_fixed.h": "ac_fixed.h",
        "hls_stream.h": "ac_channel.h",
        "hls_math.h": "<cmath> (or ac_math if available)",
    },
    type_map={
        "ap_int<N>": "ac_int<N, true>",
        "ap_uint<N>": "ac_int<N, false>",
        "ap_fixed<W, I>": "ac_fixed<W, I, true>",
        "ap_ufixed<W, I>": "ac_fixed<W, I, false>",
        "ap_fixed<W, I, AP_RND, AP_SAT>": "ac_fixed<W, I, true, AC_RND, AC_SAT>",
        "ap_fixed<W, I, AP_TRN, AP_WRAP>": "ac_fixed<W, I, true, AC_TRN, AC_WRAP>",
        "hls::stream<T>": "ac_channel<T>",
        "stream.read()": "channel.read()",
        "stream.write(v)": "channel.write(v)",
        "stream.empty()": "!channel.available(1)",
        "x.range(hi, lo)": "x.slc<hi - lo + 1>(lo)",
        "x[i] (single bit)": "x[i]",
    },
    pragma_rules=(
        PragmaRule(
            source="#pragma HLS PIPELINE II=n  (inside the loop body)",
            target="#pragma hls_pipeline_init_interval n",
            placement="on the line directly BEFORE the `for` statement",
        ),
        PragmaRule(
            source="#pragma HLS UNROLL  (inside the loop body)",
            target="#pragma hls_unroll yes",
            placement="on the line directly BEFORE the `for` statement",
        ),
        PragmaRule(
            source="#pragma HLS UNROLL factor=n  (inside the loop body)",
            target="#pragma hls_unroll n",
            placement="on the line directly BEFORE the `for` statement",
        ),
        PragmaRule(
            source="#pragma HLS DATAFLOW",
            target="#pragma hls_design block  (on each sub-function that becomes a dataflow block)",
            placement="already placed by the pre-pass before every function called from the region whose parameters are channels or scalars; the top keeps `#pragma hls_design top`",
            note="Lossy: Catapult hierarchical blocks are set per function, not per region, and a stage that receives an array stays inline (ASM-35). List the region as TRANSLATED naming the block functions; do not add or remove `hls_design block` lines. The function holding the blocks may contain only the block calls and static channels: a loop, condition or other statement around them is rejected (ASM-59, ASM-62). If the region sits in a loop or beside other statements, move those into the stages (each stage runs the loop itself; send a runtime trip count to each stage through its own channel).",
            category="lossy",
        ),
        PragmaRule(
            source="#pragma HLS INLINE",
            target="#pragma hls_design inline",
            placement="on the line directly BEFORE the function definition",
        ),
        PragmaRule(
            source="#pragma HLS ARRAY_PARTITION ...",
            target=None,
            placement="removed from source",
            note="No source-level equivalent. Catapult sets memory mapping in TCL. Remove and list it in the report as DROPPED with the original text.",
            category="directive",
        ),
        PragmaRule(
            source="#pragma HLS INTERFACE ...",
            target=None,
            placement="removed from source",
            note="Interfaces are TCL directives in Catapult. Remove and list as DROPPED.",
            category="directive",
        ),
        PragmaRule(
            source="#pragma HLS LOOP_TRIPCOUNT ...",
            target=None,
            placement="removed from source",
            note="Analysis-only pragma. Remove and list as DROPPED.",
            category="advisory",
        ),
        PragmaRule(
            source="#pragma HLS BIND_STORAGE type=ram_1p|ram_s2p|ram_2p|ram_t2p ...",
            target=None,
            placement="removed from source; already rendered into `run.tcl` as a memory module mapping",
            note="Remove and list as TRANSLATED (directive). Other BIND_STORAGE types, RESOURCE and BIND_OP: remove and list as DROPPED.",
            category="directive",
        ),
        PragmaRule(
            source="#pragma HLS STREAM variable=x depth=n",
            target=None,
            placement="removed from source; already rendered into `run.tcl` as a FIFO_DEPTH on that channel",
            note="Remove and list as TRANSLATED (directive). Without a literal depth it is DROPPED.",
            category="directive",
        ),
        PragmaRule(
            source="#pragma HLS DEPENDENCE ...",
            target=None,
            placement="removed from source",
            note="Remove and list as DROPPED. Mention it in the report as a possible performance difference.",
            category="absent",
        ),
        PragmaRule(
            source="any other #pragma HLS ...",
            target=None,
            placement="removed from source",
            note="Remove and list as DROPPED with the original text.",
            category="absent",
        ),
    ),
    forbidden_patterns=(
        r"\bap_int\s*<",
        r"\bap_uint\s*<",
        r"\bap_fixed\s*<",
        r"\bap_ufixed\s*<",
        r"\bhls::stream\b",
        r"#\s*pragma\s+HLS\b",
        r"[\"<]ap_int\.h[\">]",
        r"[\"<]ap_fixed\.h[\">]",
        r"[\"<]hls_stream\.h[\">]",
        r"[\"<]hls_math\.h[\">]",
    ),
    dut_forbidden_patterns=(
        r"\bcomplex\s*<",  # std::complex is not synthesizable (CIN-15); the pre-pass already made it ac_complex
        r"\b(tan|atan2?)f?\s*\(",  # no synthesizable float math (CIN-16); cos/sin/sqrt and exp/log/pow have shims
        r"\.to_(float|double)\s*\(|\((float|double)\)|(?<![\w.:])(float|double)\s*\((?!\s*\))",  # back to native float in the DUT: its arithmetic fails synthesis (CIN-290)
        r"\b(?:std::)?(memcpy|memset)\s*\(|\breinterpret_cast\s*<",  # byte-level copies and casts (CIN-71); cat_bytes.h has typed ones
        r"^\s+ac_channel\s*<(?:[^;&()]|\([^()]*\))*>\s+[A-Za-z_]\w*(?:\s*\[[^\]]+\])*\s*(?:\(\s*\"[^\"]*\"\s*\))?\s*;",  # a local channel must be static (HIER-6)
    ),
    required_includes=(
        (r"\bu?int(8|16|32|64)_t\b", "#include <stdint.h>"),  # EDG does not include it transitively (CRD-20)
        (r"\b(ac_ieee_float32|ac_ieee_float64)\b", "#include <ac_std_float.h>"),
        (r"\b(cos|sin|sqrt)f?\s*\(", '#include "ac_float_trig.h"'),  # fixed-point ac_math behind float cos/sin/sqrt
        (r"\b(exp|log|pow)f?\s*\(", '#include "ac_float_exp.h"'),  # hyperbolic CORDIC behind float exp/log/pow
        (r"\bcat_abs\s*\(", '#include "cat_abs.h"'),
        (r"\bcat_(memcpy|memset0|store|load)\b", '#include "cat_bytes.h"'),
    ),
    required_files=("testbench.cpp", "run.tcl", "translation_report.md"),
    driver_file="run.tcl",
    driver_required_tokens=("solution file add", "go analyze", "go extract"),
    top_marker="#pragma hls_design top",
    extra_rules=(
        "Keep the code C++14 compatible; `run.tcl` sets Catapult's C++ standard to c++14.",
        "ac_int and ac_fixed require an explicit signedness template parameter; ap_int is signed, ap_uint is unsigned.",
        "ac_fixed<W, I, S> takes the total width W and the integer width I, the same convention as ap_fixed, so W and I are copied as-is.",
        "The Vitis default rounding and overflow modes (AP_TRN, AP_WRAP) equal the Catapult defaults (AC_TRN, AC_WRAP). Only translate the modes when the original code names them explicitly.",
        "Do not use `ac_int<N, false>` for a value that was `ap_int<N>`. Signedness must be preserved exactly.",
        "Streams: ac_channel<T> has read() and write(). It does not have empty(); use `!ch.available(1)` only if the original code used empty().",
        "`hls_vector.h`, `hls_fft.h` and `hls_streamofblocks.h` are provided as local stand-in headers next to the design (hls::vector is a plain struct there); keep the includes and the `hls::vector` type as they are. Any hls::fft or stream-of-blocks API call has no equivalent and must be rewritten.",
        "Catapult does not synthesize `std::complex` or native `float`/`double` arithmetic. The pre-pass already rewrote the DUT to `ac_complex` and `ac_ieee_float32`; keep those. `cos`/`sin` on those types come from the local `ac_float_trig.h` (fixed-point CORDIC, ~1e-9 error); keep the calls unqualified (`cos(x)`, not `std::cos(x)`). `exp`/`log`/`pow` come from the local `ac_float_exp.h` (hyperbolic CORDIC); keep those unqualified too. `tan`/`atan` have no equivalent and must be constant-folded or rewritten. Never convert back to native `float`/`double` in the DUT (`.to_float()`, `(float)` casts): Catapult rejects native float arithmetic (CIN-290).",
        "The testbench is compiled, never synthesized: it may keep `std::complex<double>` and native float for the golden model, converting to and from the DUT's types at the boundary with `.real()`/`.imag()` and casts.",
        "`#pragma hls_design top|block|inline` and `#pragma hls_pipeline_init_interval` go on the line BEFORE the function or loop. Catapult silently ignores a pragma placed inside the body (CIN-319), so a pragma in Vitis position does nothing.",
        "Catapult rejects `memcpy`, `memset` and `reinterpret_cast` between different types (CIN-71). The pre-pass maps the common forms to the typed helpers in the local `cat_bytes.h` (`cat_memcpy`, `cat_memset0`, `cat_store<V>(p, v)`, `cat_load<V>(p)`): use those, with a constant byte count, and give a `void*` parameter a template type instead.",
        "Every function called from a `DATAFLOW` region that receives an array (not a channel) must NOT be a `hls_design block`: Catapult blocks may only be connected by channels (ASM-35).",
    ),
    directive_rules=load_directive_rules("catapult"),
    tool="catapult",
    synth_command=("$MGC_HOME/bin/catapult", "-shell", "-file", "synth.tcl", "-logfile", "catapult.log"),
    synth_files={"synth.tcl": 'set rc [catch {source run.tcl} err]\nif {$rc} { puts "SYNTH_ERROR: $err" }\nexit\n'},
    synth_success_marker="Catapult/*/rtl.rpt",
    parse_report=parse_catapult_report,
    degrade_step=catapult_degrade_step,
)


# Resource paths only exist after `go assembly`, are nested under every hls_design block, and a struct-typed
# array is split into one resource per field (`v._r.d`, `v._i.d`), so they are looked up, not predicted.
RESOURCE_DIRECTIVE_PROC = """proc directive_on {suffix var args} {
    set hits 0
    set pat /{top}
    for {set depth 1} {$depth <= 8} {incr depth} {
        append pat /*
        if {[catch {directive get "$pat$suffix" -match glob -return path} paths]} continue
        foreach p $paths {
            regsub "$suffix\$" [lindex [split $p /] end] {} leaf
            regsub {^[^:]*:} $leaf {} leaf
            if {[string match "*.rom" $leaf] && [string match "*Register*" $args]} continue
            if {$leaf eq $var || [string match "$var.*" $leaf]} {
                if {[catch {directive set $p {*}$args} err]} { puts "DIRECTIVE_ERROR $p $err" } else { incr hits }
            }
        }
    }
    puts "DIRECTIVE $var hits=$hits"
}
proc resource_directive {var args} { directive_on :rsc $var {*}$args }
proc channel_directive {var args} { directive_on :cns $var {*}$args }"""


def render_catapult_run_tcl(
    top: str,
    sources: list[str],
    testbench: str | None,
    clock_period_ns: float = 5.0,
    directives: list[str] | tuple[str, ...] = (),
) -> str:
    lines = [
        "# Catapult HLS run script generated by hlsfactory_agent.translate",
        "solution new -state initial",
        # an ac_channel between two inlined functions is an error by default; the hls4ml flow downgrades it too
        "options set Message/ErrorOverride HIER-10 -remove",
        "solution options set Message/ErrorOverride HIER-10 -remove",
        # c++14 relaxes constexpr to statements (CRD-2388 under the c++11 default)
        "solution options set /Input/CppStandard c++14",
        "solution options set /Input/CompilerFlags {-I. -DCATAPULT -DNDEBUG}",  # assert() has no synthesizable body (CIN-16)
    ]
    for src in sources:
        lines.append(f"solution file add ./{src} -type C++")
    if testbench:
        lines.append(f"solution file add ./{testbench} -type C++ -exclude true")
    lines += [
        "go analyze",
        "go compile",
        "solution library add nangate-45nm_beh -- -rtlsyntool DesignCompiler -vendor Nangate -technology 045nm",
        "solution library add ccs_sample_mem",
        "go libraries",
        f"directive set -CLOCKS {{clk {{-CLOCK_PERIOD {clock_period_ns}}}}}",
        # a channel between inlined functions has no legal unbuffered module (HIER-47); STREAM depth overrides per channel
        "directive set -FIFO_DEPTH 8",
        "go assembly",
    ]
    if directives:
        lines.append("# resource directives carried from Vitis pragmas")
        lines.append(RESOURCE_DIRECTIVE_PROC.replace("{top}", top))
        lines.extend(directives)
    lines += [
        "go architect",
        "go allocate",
        "go schedule",
        "go extract",
    ]
    return "\n".join(lines) + "\n"
