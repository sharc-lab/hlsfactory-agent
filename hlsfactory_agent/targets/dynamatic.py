"""Dynamatic (EPFL-LAP): dynamically scheduled dataflow HLS from plain C.

The kernel and its test program live in one file named after the top function (`<top>.c`); `main` drives the
kernel through `CALL_KERNEL`, which the tool reuses for profiling and C/RTL co-simulation. The tool ships
only as a from-source build, so synthesis runs in its own image (docker_images/Dockerfile.dynamatic), which
carries the CBC solver for the throughput-driven (MILP) buffer placement.
"""
from __future__ import annotations

import re
import shutil
from pathlib import Path

from hlsfactory_agent.spec import PragmaRule, SynthResult, TargetSpec
from hlsfactory_agent.tools import DYNAMATIC_ROOT

DIR_PKG = Path(__file__).resolve().parent.parent
DIR_DYNAMATIC_INCLUDE = DIR_PKG / "dynamatic_include"

CLOCK_NS = 10  # the same target period the other targets use
HDL = "vhdl"  # the LSQ generator emits VHDL whatever --hdl says; all-VHDL keeps GHDL able to simulate it
# fpga20 (throughput-driven MILP buffering, needs a solver and the profiling run `main()` provides) is what makes the
# cycle counts comparable with the static schedulers; on-merges is the solver-free fallback the degrade step uses
BUFFER_ALGORITHM = "fpga20"
MILP_SOLVER = "cbc"


def render_dynamatic_driver(top: str, sources: list[str]) -> str:
    """The command script `bin/dynamatic --exit-on-failure --run` replays; the kernel file must be named after
    the top function and sit in the output folder, which is mounted at /work."""
    kernel = f"{top}.c"
    if kernel not in sources:
        kernel = next((s for s in sources if s.endswith(".c")), kernel)
    return (
        "# Dynamatic command script; lines starting with # are comments\n"
        "# The kernel file is named after the top function and holds main() with CALL_KERNEL (co-simulation inputs)\n"
        f"set-dynamatic-path {DYNAMATIC_ROOT}\n"
        f"set-src /work/{kernel}\n"
        f"set-clock-period {CLOCK_NS}\n"
        f"compile --buffer-algorithm {BUFFER_ALGORITHM} --milp-solver {MILP_SOLVER}\n"
        f"write-hdl --hdl {HDL}\n"
        "simulate --simulator ghdl\n"
        "exit\n"
    )


# first match wins; keys are fragments of the tool's `[FATAL]` messages (tools/dynamatic/scripts/*.sh)
FAILURE_CLASSES = (
    ("No 'dynamatic' executable", "tool-missing"),
    ("Failed to copy C source", "kernel-file-missing"),
    ("Failed to disable short-circuiting", "source-rewriter"),
    ("Failed to compile to LLVM IR", "compile-error"),
    ("Failed to apply optimization to LLVM IR", "llvm-pass"),
    ("Failed to perform array partitioning", "array-partition"),
    ("Failed to apply memory dependency analysis", "memory-dependency"),
    ("Failed to canonicalize GEPs", "llvm-pass"),
    ("Segmentation fault", "tool-crash"),
    ("Failed to convert to std dialect", "unsupported-construct"),
    ("Failed to apply CF transformations", "cf-transform"),
    ("Failed to mark memory interfaces", "memory-interface"),
    ("Failed to compile cf to handshake", "handshake-lowering"),
    ("Failed to apply transformations to handshake", "handshake-transform"),
    ("Failed to build kernel for profiling", "profiling"),
    ("Failed to kernel for profiling", "profiling"),
    ("Failed to profile cf-level", "profiling"),
    ("buffers", "buffer-placement"),
    ("Failed to generate handshake_export", "handshake-export"),
    ("Failed to create", "dot-export"),
    ("Failed to lower to HW", "hw-lowering"),
    ("Failed to export RTL", "hdl-export"),
    ("Failed to build kernel for IO gen", "testbench-build"),
    ("Failed to run kernel for IO gen", "testbench-run"),
    ("outputs do not match", "cosim-mismatch"),
    ("Maximum clock count", "cosim-timeout"),
    ("Simulation failed", "cosim-error"),
)
RE_FATAL = re.compile(r"^\[FATAL\] (.*)$", re.M)
RE_CLANG_ERROR = re.compile(r"^.*: error: .*$", re.M)
RE_MATCH = re.compile(r"C and VHDL outputs match")
RE_LATENCY = re.compile(r"Simulation done! Latency = (\d+) cycles")  # the generated testbench's note


def classify_dynamatic_error(log: str) -> str:
    """Only the `[FATAL]` lines, the compiler's `error:` lines and the verifier's verdict count: an `[INFO]`
    line such as `Placed simple buffers` must not match a fragment."""
    text = "\n".join(RE_FATAL.findall(log) + RE_CLANG_ERROR.findall(log) + re.findall(r"^.*(?:outputs do not match|Maximum clock count|Unknow Simulator|No 'dynamatic' executable|Segmentation fault).*$", log, re.M))
    return next((name for key, name in FAILURE_CLASSES if key in text), "other")


RE_CALL_KERNEL = re.compile(r"\bCALL_KERNEL\s*\(\s*([A-Za-z_]\w*)")


def check_kernel_call(dir_output: Path) -> list[str]:
    """The verifier records one kernel transaction: `main()` must invoke `CALL_KERNEL` exactly once, naming the
    function the kernel file is named after (lstm_n called it three times: "Transaction number mismatch")."""
    out: list[str] = []
    for p in sorted(dir_output.glob("*.c")):
        code = re.sub(r"//[^\n]*|/\*.*?\*/", "", p.read_text(encoding="utf-8", errors="replace"), flags=re.S)
        calls = RE_CALL_KERNEL.findall(code)
        if len(calls) != 1:
            out.append(f"{p.name}: CALL_KERNEL appears {len(calls)} times; main() must call the kernel exactly once "
                       "(the co-simulation records a single transaction): fold repeated calls into one, or loop inside the kernel")
        elif calls[0] != p.stem:
            out.append(f"{p.name}: CALL_KERNEL names `{calls[0]}` but the kernel file must be named after the kernel (`{calls[0]}.c`)")
    return out


def _first_error(log: str) -> str:
    m = RE_CLANG_ERROR.search(log) or RE_FATAL.search(log)
    return m.group(0).strip()[:300] if m else ""


def parse_dynamatic_report(dir_run: Path) -> SynthResult:
    """PASS means compile, write-hdl and the GHDL co-simulation all ran and the C and RTL outputs matched;
    RTL alone is a FAILED result whose class names the co-simulation problem. Latency is the simulated
    cycle count when the simulator log gives a finishing time."""
    f_log = dir_run / "synth_stdout.log"
    log = f_log.read_text(encoding="utf-8", errors="replace") if f_log.exists() else ""
    f_report = dir_run / "out" / "sim" / "report.txt"
    report = f_report.read_text(encoding="utf-8", errors="replace") if f_report.exists() else ""
    text = log + "\n" + report
    hdl = list((dir_run / "out" / "hdl").glob(f"*.{'vhd' if HDL == 'vhdl' else 'v'}"))
    if not hdl:
        return SynthResult("FAILED", first_error=_first_error(text), failure_class=classify_dynamatic_error(text))
    if not RE_MATCH.search(report):
        cls = classify_dynamatic_error(text)
        return SynthResult("FAILED", first_error=_first_error(text) or "RTL written; co-simulation did not pass",
                           failure_class=cls if cls != "other" else "cosim-error")
    m = RE_LATENCY.search(report)
    return SynthResult("PASS", latency=int(m.group(1)) if m else None, resources={"hdl_files": len(hdl)})


SYNTH_OUTPUTS = ("out", "synth_stdout.log", "synth.json")


RE_SMART_BUFFER_FAIL = re.compile(r"Failed to (place smart buffers|build kernel for profiling|kernel for profiling|profile cf-level)")


def dynamatic_degrade_step(dir_failed: Path, dir_next: Path) -> str | None:
    """Two knobs the driver has, tried in order and each at most once: when the profiling or MILP buffer placement
    failed, fall back to the solver-free `on-merges` placement; when the log names the load-store queue (LSQ
    generation, its VHDL, or a stalled co-simulation behind it), `--disable-lsq` routes every access through a
    plain memory controller. The co-simulation still has to match afterwards."""
    f_log = dir_failed / "synth_stdout.log"
    log = f_log.read_text(encoding="utf-8", errors="replace") if f_log.exists() else ""
    driver = dir_failed / "run_dynamatic.dyn"
    if not driver.exists():
        return None
    text = driver.read_text(encoding="utf-8")
    if f"--buffer-algorithm {BUFFER_ALGORITHM}" in text and RE_SMART_BUFFER_FAIL.search(log):
        step, text = "buffers:on-merges", text.replace(f"--buffer-algorithm {BUFFER_ALGORITHM}", "--buffer-algorithm on-merges")
    elif "--disable-lsq" not in text and re.search(r"\bLSQ\b|\blsq\b", log):
        step, text = "lsq:off", re.sub(r"^compile\b", "compile --disable-lsq", text, flags=re.M)
    else:
        return None
    shutil.copytree(dir_failed, dir_next, ignore=shutil.ignore_patterns(*SYNTH_OUTPUTS))
    (dir_next / "run_dynamatic.dyn").write_text(text, encoding="utf-8")
    return step


_DROP = "removed from source"
_UNSUPPORTED = "NOT SUPPORTED by Dynamatic: stop and report the design as untranslatable"
DYNAMATIC = TargetSpec(
    name="dynamatic",
    display_name="Dynamatic (EPFL dataflow HLS, plain C)",
    include_dir=DIR_DYNAMATIC_INCLUDE,
    include_dir_name="dynamatic_include",
    cxx_std_check="c17",
    language="c",
    fence=False,
    header_map={
        "ap_int.h": "<stdint.h>",
        "ap_fixed.h": _UNSUPPORTED,
        "hls_stream.h": _UNSUPPORTED,
        "hls_math.h": "<math.h>",
    },
    type_map={
        "ap_int<8|16|32>": "int8_t | int16_t | int32_t",
        "ap_uint<8|16|32>": "uint8_t | uint16_t | uint32_t",
        "ap_int<N> (other N)": "_BitInt(N)",
        "ap_uint<N> (other N)": "unsigned _BitInt(N)",
        "ap_fixed / ap_ufixed": _UNSUPPORTED,
        "hls::stream<T>": _UNSUPPORTED + " (Dynamatic has no channels: every array is a memory, every scalar a token)",
        "long, long long, int64_t, uint64_t": "not synthesizable; keep only in main() for file I/O, never in the kernel",
        "x.range(h, l)": "(x >> l) & ((1u << (h - l + 1)) - 1), or a _BitInt cast of the shifted value",
        "x[i] (bit select)": "(x >> i) & 1",
        "bool": "bool from <stdbool.h>",
        "float / double": "kept as they are",
    },
    pragma_rules=(
        PragmaRule(
            source="#pragma HLS PIPELINE ...  (inside the loop body)",
            target=None,
            placement=_DROP,
            note="Dynamatic schedules dynamically: loops pipeline by themselves, there is no II to set. Remove and list as DROPPED.",
            category="absent",
        ),
        PragmaRule(
            source="#pragma HLS UNROLL factor=n  (inside the loop body)",
            target="#pragma clang loop unroll_count(n)",
            placement="on the line directly BEFORE the `for` statement",
        ),
        PragmaRule(
            source="#pragma HLS UNROLL  (inside the loop body)",
            target="#pragma clang loop unroll(full)",
            placement="on the line directly BEFORE the `for` statement",
        ),
        PragmaRule(
            source="#pragma HLS ARRAY_PARTITION variable=a dim=d type=cyclic|block|complete factor=n",
            target="#pragma DYN array_partition array=a dimension=d style=cyclic|block|complete factor=n",
            placement="anywhere inside the kernel's body (the first line of the body is fine); `a` must be a kernel parameter",
            note="dim=0 (all dimensions) has no equivalent: use dimension=1 and say so in the report",
        ),
        PragmaRule(
            source="#pragma HLS INLINE",
            target=None,
            placement=_DROP,
            note="Functions cannot be called from the kernel at all: inline the body by hand or as a macro (Step 3). List as DROPPED.",
            category="absent",
        ),
        PragmaRule(
            source="#pragma HLS DATAFLOW",
            target=None,
            placement=_DROP,
            note="Lossy: the region becomes one sequential kernel. Remove and list as DROPPED.",
            category="lossy",
        ),
        PragmaRule(
            source="#pragma HLS INTERFACE ...",
            target=None,
            placement=_DROP,
            note="Interfaces come from the parameter types (array = memory, scalar = token). Remove and list as DROPPED.",
            category="directive",
        ),
        PragmaRule(
            source="any other #pragma HLS ... (LOOP_TRIPCOUNT, BIND_STORAGE, STREAM, DEPENDENCE, RESOURCE, ...)",
            target=None,
            placement=_DROP,
            note="Remove and list as DROPPED with the original text.",
            category="absent",
        ),
    ),
    forbidden_patterns=(
        r"\bap_u?int\s*<",
        r"\bap_u?fixed\s*<",
        r"\bhls::",
        r"#\s*pragma\s+HLS\b",
        r"#\s*include\s*[<\"](ap_int|ap_fixed|hls_stream|hls_math|ap_axi_sdata|hls_vector)\.h[>\"]",
    ),
    dut_forbidden_patterns=(
        r"[A-Za-z_]\w*\s*\[\s*\]\s*[,)]",  # an unsized array parameter: the translator segfaults (ave8, fir)
        r"\btemplate\s*<",
        r"\bclass\b",
        r"\bstd::",
        r"\bnew\s+[A-Za-z_]|\bmalloc\s*\(",
        r"\bnamespace\b",
        # stdout is the profiler's input (one line per kernel argument): main() reports on stderr only
        r"(?<![\w.])printf\s*\(",
        r"\bputs\s*\(|\bputchar\s*\(|\bstdout\b",
    ),
    required_includes=(
        (r"\bu?int(8|16|32)_t\b", "#include <stdint.h>"),
        (r"\bCALL_KERNEL\b", '#include "dynamatic/Integration.h"'),
        (r"\b(cos|sin|tan|sqrt|exp|log|pow|atan2?|fabs|floor|ceil)f?\s*\(", "#include <math.h>"),
        (r"\bbool\b", "#include <stdbool.h>"),
        (r"\bfprintf\s*\(|\bfputs\s*\(|\bstderr\b", "#include <stdio.h>"),
    ),
    blocking_types=("ap_fixed", "ap_ufixed", "hls::stream", "hls::vector"),
    extra_checks=(check_kernel_call,),
    required_files=("run_dynamatic.dyn", "translation_report.md"),
    driver_file="run_dynamatic.dyn",
    driver_required_tokens=("set-src", "compile", "write-hdl"),
    top_marker="",  # the kernel file's name is the top
    extra_rules=(
        "The output is ONE C file named exactly `<top>.c` (the top function's name) holding the kernel and a `main()`; "
        "headers it includes may stay as separate files next to it. Delete `testbench.cpp`: its checks move into `main()`.",
        "`main()` includes `dynamatic/Integration.h`, declares one variable per kernel parameter with the SAME NAME and type "
        "as the parameter (arrays with the same constant dimensions), fills them, and calls `CALL_KERNEL(<top>, arg1, arg2, ...)` "
        "exactly once, then checks the results and returns 0 on success, 1 on failure. Never call the kernel directly.",
        "The kernel calls no function at all (not even its own helpers, memcpy or math): inline every callee's body by hand "
        "or as a macro. No recursion. `main()` may call anything (file I/O, printf, rand).",
        "The kernel declares no local arrays: each one becomes an extra array parameter appended AFTER the original parameters, "
        "allocated and (if it was initialised) filled in `main()` with the same values. Local scalars are fine.",
        "No pointers, pointer arithmetic, dynamic memory, globals or static variables in the kernel: arrays are parameters "
        "with constant dimensions (`int a[N][M]`), accessed by indexing. A global the kernel read becomes a parameter or a #define.",
        "Integer types are int8_t/int16_t/int32_t, their unsigned forms, char, short, int, unsigned, `_BitInt(N)`, float, double. "
        "No 64-bit integers, no `long`, no fixed point, no streams, no structs as parameters.",
        "The same file is compiled as C (clang -std=c17) by the tool and as C++ (clang++) for co-simulation: stay in the "
        "common subset (cast `void*` explicitly, no VLAs, no designated-initializer tricks, no implicit int, `bool` from <stdbool.h>).",
        "Every array parameter has constant dimensions (`T a[N]`, never `T a[]` with a runtime count): the tool crashes on an "
        "unsized parameter. The tool's int<->float converters are 32-bit only: an integer array the kernel converts to or from "
        "float must be int32_t/uint32_t at the interface (a narrower type is folded back by the optimizer and rejected, idct); "
        "main() then fills it from the original narrower values and compares with the same casts.",
        "Data files the original testbench read stay in the output folder and are opened by the same relative path "
        "(the tool runs from that folder).",
        "`main()` writes NOTHING to stdout: the tool captures the program's stdout as the profiling vectors (one line per "
        "kernel argument, printed by CALL_KERNEL), so any printf/puts/cout breaks the compile step. Report through "
        "`fprintf(stderr, ...)` and the exit code (0 = pass, 1 = fail).",
        "`#pragma clang loop unroll_count(n)` goes on the line before the loop; `#pragma DYN array_partition ...` goes inside "
        "the kernel body and names a parameter. There is no pipelining pragma and no top marker: the file name is the top.",
    ),
    tool="dynamatic",
    synth_command=(f"{DYNAMATIC_ROOT}/bin/dynamatic", "--exit-on-failure", "--run", "run_dynamatic.dyn"),
    synth_timeout_s=1200,
    synth_success_marker=f"out/hdl/*.{'vhd' if HDL == 'vhdl' else 'v'}",
    parse_report=parse_dynamatic_report,
    degrade_step=dynamatic_degrade_step,
)
