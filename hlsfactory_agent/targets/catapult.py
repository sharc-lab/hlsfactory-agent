from __future__ import annotations

import json
import math
import re
from pathlib import Path

from hlsfactory_agent.spec import PragmaRule, SynthResult, TargetSpec

DIR_PKG = Path(__file__).resolve().parent.parent
DIR_AC_TYPES_INCLUDE = DIR_PKG / "ac_types_include"
DIR_TARGETS = DIR_PKG / "targets"


def load_directive_rules(target_name: str) -> dict[str, str]:
    f = DIR_TARGETS / f"{target_name}_directives.json"
    return json.loads(f.read_text(encoding="utf-8")) if f.exists() else {}


def directive_template(resource: dict, rules: dict[str, str]) -> str | None:
    """Most specific matching rule: kind.type.scope, then kind.type, then kind."""
    args = resource.get("args", {})
    keys = []
    if args.get("type"):
        keys.append(f"{resource['kind']}.{args['type']}.{resource.get('scope', 'unknown')}")
        keys.append(f"{resource['kind']}.{args['type']}")
    keys.append(resource["kind"])
    return next((rules[k] for k in keys if k in rules), None)


REGISTER_THRESHOLD = 256  # Catapult default; a larger array cannot map to [Register] (MEM-8)


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


def render_directives(resources: list[dict], top: str, rules: dict[str, str]) -> tuple[list[str], list[dict]]:
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
        kind, factor, block_size, reason = _partition_form(r)
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
                impl=args.get("impl", ""),
                mode=args.get("mode", ""),
                port=args.get("port", ""),
            )
        )
    return lines, unrendered


# first match wins; keys are message fragments from catapult.log
FAILURE_CLASSES = (
    ("License request failed", "license"),
    ("ASM-35", "shared-array"),
    ("ASM-34", "unsupported-construct"),
    ("Logic mixed with interconnect", "logic-in-interconnect"),
    ("SCHD-20", "schedule-delay"),
    ("insufficient resources", "insufficient-memory-ports"),
    ("HIER-47", "channel-fifo"),
    ("CIN-290", "float-op"),
    ("CIN-15", "unsupported-type"),
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


CATAPULT = TargetSpec(
    name="catapult",
    display_name="Siemens Catapult HLS",
    include_dir=DIR_AC_TYPES_INCLUDE,
    include_dir_name="ac_types_include",
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
            placement="on the line directly BEFORE the sub-function definition; the top keeps `#pragma hls_design top`",
            note="Lossy: Catapult hierarchical blocks are set per function, not per region. Record it in the report.",
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
            source="#pragma HLS BIND_STORAGE / RESOURCE / BIND_OP ...",
            target=None,
            placement="removed from source",
            note="Resource binding is TCL in Catapult. Remove and list as DROPPED.",
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
        r"\bhls::vector\b",
        r"#\s*pragma\s+HLS\b",
        r"[\"<]ap_int\.h[\">]",
        r"[\"<]ap_fixed\.h[\">]",
        r"[\"<]hls_stream\.h[\">]",
        r"[\"<]hls_(vector|fft|streamofblocks|math)\.h[\">]",
    ),
    dut_forbidden_patterns=(
        r"\bcomplex\s*<",  # std::complex is not synthesizable (CIN-15); the pre-pass already made it ac_complex
        r"\b(cos|sin|tan|exp|log|sqrt|pow|atan2?)f?\s*\(",  # no synthesizable float math (CIN-16)
        r"^\s+ac_channel\s*<[^;&]*>\s+[A-Za-z_]\w*\s*;",  # a local channel must be static (HIER-6)
    ),
    required_includes=(
        (r"\bu?int(8|16|32|64)_t\b", "#include <stdint.h>"),  # EDG does not include it transitively (CRD-20)
        (r"\b(ac_ieee_float32|ac_ieee_float64)\b", "#include <ac_std_float.h>"),
    ),
    required_files=("testbench.cpp", "run.tcl", "translation_report.md"),
    driver_file="run.tcl",
    driver_required_tokens=("solution file add", "go analyze", "go extract"),
    top_marker="#pragma hls_design top",
    extra_rules=(
        "Keep the code C++11 compatible. Catapult's default C++ standard is c++11.",
        "ac_int and ac_fixed require an explicit signedness template parameter; ap_int is signed, ap_uint is unsigned.",
        "ac_fixed<W, I, S> takes the total width W and the integer width I, the same convention as ap_fixed, so W and I are copied as-is.",
        "The Vitis default rounding and overflow modes (AP_TRN, AP_WRAP) equal the Catapult defaults (AC_TRN, AC_WRAP). Only translate the modes when the original code names them explicitly.",
        "Do not use `ac_int<N, false>` for a value that was `ap_int<N>`. Signedness must be preserved exactly.",
        "Streams: ac_channel<T> has read() and write(). It does not have empty(); use `!ch.available(1)` only if the original code used empty().",
        "`hls::vector<T, N>` has no Catapult equivalent: replace it with a plain struct holding `T data[N]` and an `operator[]`, and drop `hls_vector.h`, `hls_fft.h` and `hls_streamofblocks.h` (Catapult's front end cannot see them).",
        "Catapult does not synthesize `std::complex`, native `float`/`double` arithmetic, or `cos`/`sin`/`sqrt` on float. The pre-pass already rewrote the DUT to `ac_complex` and `ac_ieee_float32`; keep those. Compute float trig tables at compile time (a `static const float` literal table, values computed offline) instead of calling the functions in the DUT.",
        "The testbench is compiled, never synthesized: it may keep `std::complex<double>` and native float for the golden model, converting to and from the DUT's types at the boundary with `.real()`/`.imag()` and casts.",
        "`#pragma hls_design top|block|inline` and `#pragma hls_pipeline_init_interval` go on the line BEFORE the function or loop. Catapult silently ignores a pragma placed inside the body (CIN-319), so a pragma in Vitis position does nothing.",
        "Every function called from a `DATAFLOW` region that receives an array (not a channel) must NOT be a `hls_design block`: Catapult blocks may only be connected by channels (ASM-35).",
    ),
    directive_rules=load_directive_rules("catapult"),
    synth_command=("$MGC_HOME/bin/catapult", "-shell", "-file", "synth.tcl", "-logfile", "catapult.log"),
    synth_env={
        "MGC_HOME": "/tools/software/siemens/catapult/latest/Mgc_home",
        "MGLS_LICENSE_FILE": "1717@ece-winlic.ece.gatech.edu",
        "SALT_LICENSE_FILE": "1717@ece-winlic.ece.gatech.edu",
    },
    synth_files={"synth.tcl": 'set rc [catch {source run.tcl} err]\nif {$rc} { puts "SYNTH_ERROR: $err" }\nexit\n'},
    synth_success_marker="Catapult/*/rtl.rpt",
    parse_report=parse_catapult_report,
)


# Resource paths only exist after `go assembly`, are nested under every hls_design block, and a struct-typed
# array is split into one resource per field (`v._r.d`, `v._i.d`), so they are looked up, not predicted.
RESOURCE_DIRECTIVE_PROC = """proc resource_directive {var args} {
    set hits 0
    set pat /{top}
    for {set depth 1} {$depth <= 8} {incr depth} {
        append pat /*
        if {[catch {directive get "$pat:rsc" -match glob -return path} paths]} continue
        foreach p $paths {
            regsub {:rsc$} [lindex [split $p /] end] {} leaf
            regsub {^[^:]*:} $leaf {} leaf
            if {[string match "*.rom" $leaf] && [string match "*Register*" $args]} continue
            if {$leaf eq $var || [string match "$var.*" $leaf]} {
                if {[catch {directive set $p {*}$args} err]} { puts "DIRECTIVE_ERROR $p $err" } else { incr hits }
            }
        }
    }
    puts "DIRECTIVE $var hits=$hits"
}"""


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
        "solution options set /Input/CppStandard c++11",
        "solution options set /Input/CompilerFlags {-I. -DCATAPULT}",
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
