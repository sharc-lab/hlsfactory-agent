from __future__ import annotations

import re
import shutil
from pathlib import Path

from hlsfactory_agent.spec import PragmaRule, SynthResult, TargetSpec
from hlsfactory_agent.targets.catapult import DIR_AC_TYPES_INCLUDE, CATAPULT

# clang 16 front end, interfaces inferred from the parameter types (ac_channel -> FIFO, array -> memory),
# the bundled soft libm linked so float math synthesizes
BAMBU_FLAGS = ("--compiler=I386_CLANG16", "--generate-interface=INFER", "--clock-period=10", "-I.", "-lm", "-v2")


def render_bambu_driver(top: str, sources: list[str]) -> str:
    return (
        "#!/bin/sh\n"
        "# Bambu takes the top from --top-fname; ac_types and the soft libm come with the tool\n"
        f'exec "${{BAMBU:-bambu}}" {" ".join(sources)} --top-fname={top} {" ".join(BAMBU_FLAGS)}\n'
    )


# first match wins; keys are message fragments from the Bambu log
FAILURE_CLASSES = (
    ("Missing interface mode attribute", "missing-interface"),
    ("pragma not supported", "unsupported-pragma"),
    ("expected a for, while, or do-while loop", "pragma-placement"),
    ("Front-end compiler returns an error", "compile-error"),
    ("not found in IR", "top-not-found"),
    ("Vertices not completely allocated", "no-library-component"),
    ("unexpected pattern", "interface-inference"),
    ("Bad Parameters format", "bad-option"),
    ("clock constraint too tight", "clock-too-tight"),
    ("Please report bugs", "tool-crash"),
    ("Segmentation fault", "tool-crash"),
    ("error in backend: Plugin Error", "plugin-error"),
)
RE_ERROR = re.compile(r"^(?:.*: error: .*|error -> .*)$", re.M)


def classify_bambu_error(log: str) -> str:
    return next((name for key, name in FAILURE_CLASSES if key in log), "other")


def parse_bambu_report(dir_run: Path) -> SynthResult:
    """No simulation is run, so latency is unknown; area, DSPs and flip-flops come from the -v2 log."""
    f_log = dir_run / "synth_stdout.log"
    log = f_log.read_text(encoding="utf-8", errors="replace") if f_log.exists() else ""
    errors = RE_ERROR.findall(log)
    if errors or not list(dir_run.glob("*.v")):
        return SynthResult("FAILED", first_error=errors[0][:300] if errors else "", failure_class=classify_bambu_error(log))
    area = re.findall(r"Total estimated area: ([\d.]+)", log)
    dsp = re.findall(r"Estimated number of DSPs: (\d+)", log)
    ff = re.findall(r"Total number of flip-flops in function \S+: (\d+)", log)
    return SynthResult(
        "PASS",
        area=float(area[-1]) if area else None,
        resources={"DSP": int(dsp[-1]) if dsp else None, "FF": int(ff[-1]) if ff else None},
    )


SYNTH_OUTPUTS = ("HLS_output", "*.v", "*.mem", "synth_stdout.log", "synthesize_*.sh", "synth.json")


def bambu_degrade_step(dir_failed: Path, dir_next: Path) -> str | None:
    """Interface inference gives up on a pointer the top passes to memcpy (`unexpected pattern`); the MINIMAL
    interface mode synthesizes it with plain memory ports. Nothing else is touched."""
    log = (dir_failed / "synth_stdout.log").read_text(encoding="utf-8", errors="replace") if (dir_failed / "synth_stdout.log").exists() else ""
    driver = dir_failed / "run_bambu.sh"
    if "unexpected pattern" not in log or not driver.exists() or "--generate-interface=INFER" not in driver.read_text(encoding="utf-8"):
        return None
    shutil.copytree(dir_failed, dir_next, ignore=shutil.ignore_patterns(*SYNTH_OUTPUTS))
    f = dir_next / "run_bambu.sh"
    f.write_text(f.read_text(encoding="utf-8").replace("--generate-interface=INFER", "--generate-interface=MINIMAL"), encoding="utf-8")
    return "interface:minimal"


_DROP = "removed from source"
BAMBU = TargetSpec(
    name="bambu",
    display_name="Bambu (PandA) HLS",
    include_dir=DIR_AC_TYPES_INCLUDE,
    include_dir_name="ac_types_include",
    cxx_std_check="c++17",
    header_map={"ap_int.h": "ac_int.h", "ap_fixed.h": "ac_fixed.h", "hls_stream.h": "ac_channel.h", "hls_math.h": "<math.h>"},
    type_map={k: v for k, v in CATAPULT.type_map.items()},
    pragma_rules=(
        PragmaRule(
            source="#pragma HLS PIPELINE ...  (inside the loop body)",
            target=None,
            placement=_DROP,
            note="Bambu rejects its pipeline pragma on loops (`Loop pipelining pragma not supported`). Remove and list as DROPPED with the original text.",
            category="absent",
        ),
        PragmaRule(
            source="#pragma HLS UNROLL  (inside the loop body)",
            target="#pragma HLS unroll",
            placement="on the line directly BEFORE the `for` statement; inside the body it is a compile error",
        ),
        PragmaRule(
            source="#pragma HLS UNROLL factor=n  (inside the loop body)",
            target="#pragma HLS unroll factor=n",
            placement="on the line directly BEFORE the `for` statement; inside the body it is a compile error",
        ),
        PragmaRule(
            source="#pragma HLS INLINE",
            target="#pragma HLS inline",
            placement="on the line directly BEFORE the function definition",
        ),
        PragmaRule(
            source="#pragma HLS DATAFLOW",
            target=None,
            placement=_DROP,
            note="Lossy: the region is synthesized sequentially. Remove and list as DROPPED.",
            category="lossy",
        ),
        PragmaRule(
            source="#pragma HLS INTERFACE ...",
            target=None,
            placement=_DROP,
            note="Interfaces are inferred from the parameter types (`--generate-interface=INFER`). Remove and list as DROPPED.",
            category="directive",
        ),
        PragmaRule(
            source="any other #pragma HLS ... (ARRAY_PARTITION, LOOP_TRIPCOUNT, BIND_STORAGE, STREAM, DEPENDENCE, ...)",
            target=None,
            placement=_DROP,
            note="Remove and list as DROPPED with the original text.",
            category="absent",
        ),
    ),
    forbidden_patterns=tuple(p for p in CATAPULT.forbidden_patterns if "pragma" not in p) + (r"#\s*pragma\s+HLS\s+(?!unroll\b|inline\b)",),
    dut_forbidden_patterns=(r"\bcomplex\s*<",),  # std::complex is not synthesizable; the pre-pass already made it ac_complex
    required_includes=(
        (r"\bu?int(8|16|32|64)_t\b", "#include <stdint.h>"),
        (r"\b(cos|sin|tan|sqrt|exp|log|pow|atan2?|fabs)f?\s*\(", "#include <math.h>"),
    ),
    required_files=("testbench.cpp", "run_bambu.sh", "translation_report.md"),
    driver_file="run_bambu.sh",
    driver_required_tokens=("BAMBU", "--top-fname"),
    top_marker="",  # the driver names the top (--top-fname); no source marker exists
    extra_rules=(
        "ac_int and ac_fixed require an explicit signedness template parameter; ap_int is signed, ap_uint is unsigned.",
        "ac_fixed<W, I, S> takes the total width W and the integer width I, the same convention as ap_fixed, so W and I are copied as-is.",
        "The Vitis default rounding and overflow modes (AP_TRN, AP_WRAP) equal the ac_fixed defaults (AC_TRN, AC_WRAP). Only translate the modes when the original code names them explicitly.",
        "Do not use `ac_int<N, false>` for a value that was `ap_int<N>`. Signedness must be preserved exactly.",
        "Streams: ac_channel<T> has read() and write(). It does not have empty(); use `!ch.available(1)` only if the original code used empty().",
        "Native `float` and `double` and the <math.h> functions synthesize (the driver links Bambu's soft libm); keep them. `std::complex` does not: the pre-pass already rewrote it to `ac_complex`; keep that.",
        "`#pragma HLS unroll` and `#pragma HLS inline` are the only Bambu pragmas used; both go on the line BEFORE the loop or function. There is no top marker: the driver's `--top-fname` names the top.",
        "The testbench is compiled, never synthesized: it may keep `std::complex` and host types, converting at the DUT boundary.",
        "A top that returned an ac_int/ac_fixed/ac_complex by value now returns void with a trailing `ret_out` reference (Bambu aborts on a class returned by value); the pre-pass did the DUT side, update the testbench call sites.",
    ),
    tool="bambu",
    synth_command=("sh", "run_bambu.sh"),
    synth_success_marker="*.v",
    parse_report=parse_bambu_report,
    degrade_step=bambu_degrade_step,
)
