"""Agentic translation of a standalone HLS design from one tool dialect to another.

The first supported flow is Vitis HLS -> Catapult HLS. The translation itself is done by the
same Pi coding agent used for extraction (see ``core.py``); this module adds a target
description table, a prompt built from that table, deterministic post-checks, and a run
class that mirrors ``HLSFactoryAgentRun`` but takes a local design directory as input.
"""

from __future__ import annotations

import json
import os
import re
import shlex
import shutil
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Any

import docker
from docker.models.containers import Container

from hlsfactory_agent.utils import load_jsonl_text

DIR_CURRENT = Path(__file__).resolve().parent
DIR_VITIS_HLS_INCLUDE = DIR_CURRENT / "vitis_hls_include"
DIR_AC_TYPES_INCLUDE = DIR_CURRENT / "ac_types_include"

DOCKER_IMAGE_NAME = "hlsfactory-agent"

CONTAINER_RUN_AREA = "/workspace/run_area"
INPUT_DIR_NAME = "input_design"
OUTPUT_DIR_NAME = "output_design"

SOURCE_SUFFIXES = (".cpp", ".cc", ".c", ".h", ".hpp")


# --------------------------------------------------------------------------------------
# Target descriptions
# --------------------------------------------------------------------------------------


@dataclass(frozen=True)
class PragmaRule:
    """How one source-tool pragma maps onto the target tool."""

    source: str
    target: str | None
    placement: str
    note: str = ""


@dataclass(frozen=True)
class TargetSpec:
    name: str
    display_name: str
    include_dir: Path
    include_dir_name: str
    cxx_std_check: str
    header_map: dict[str, str]
    type_map: dict[str, str]
    pragma_rules: tuple[PragmaRule, ...]
    forbidden_patterns: tuple[str, ...]
    required_files: tuple[str, ...]
    driver_file: str
    driver_required_tokens: tuple[str, ...]
    top_marker: str
    extra_rules: tuple[str, ...] = field(default_factory=tuple)

    def to_dict(self) -> dict:
        d = asdict(self)
        d["include_dir"] = str(self.include_dir)
        return d


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
        ),
        PragmaRule(
            source="#pragma HLS INTERFACE ...",
            target=None,
            placement="removed from source",
            note="Interfaces are TCL directives in Catapult. Remove and list as DROPPED.",
        ),
        PragmaRule(
            source="#pragma HLS LOOP_TRIPCOUNT ...",
            target=None,
            placement="removed from source",
            note="Analysis-only pragma. Remove and list as DROPPED.",
        ),
        PragmaRule(
            source="#pragma HLS BIND_STORAGE / RESOURCE / BIND_OP ...",
            target=None,
            placement="removed from source",
            note="Resource binding is TCL in Catapult. Remove and list as DROPPED.",
        ),
        PragmaRule(
            source="#pragma HLS DEPENDENCE ...",
            target=None,
            placement="removed from source",
            note="Remove and list as DROPPED. Mention it in the report as a possible performance difference.",
        ),
        PragmaRule(
            source="any other #pragma HLS ...",
            target=None,
            placement="removed from source",
            note="Remove and list as DROPPED with the original text.",
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
    ),
)

TARGETS: dict[str, TargetSpec] = {CATAPULT.name: CATAPULT}


def get_target(name: str) -> TargetSpec:
    try:
        return TARGETS[name]
    except KeyError as e:
        raise ValueError(f"Unknown translation target `{name}`. Known: {sorted(TARGETS)}") from e


# --------------------------------------------------------------------------------------
# Driver script rendering
# --------------------------------------------------------------------------------------


def render_catapult_run_tcl(
    top: str,
    sources: list[str],
    testbench: str | None,
    clock_period_ns: float = 5.0,
) -> str:
    lines = [
        "# Catapult HLS run script generated by hlsfactory_agent.translate",
        "solution new -state initial",
        "solution options set /Input/CppStandard c++11",
        "solution options set /Input/CompilerFlags {-I. -DCATAPULT}",
    ]
    for src in sources:
        lines.append(f"solution file add ./{src} -type C++")
    if testbench:
        lines.append(f"solution file add ./{testbench} -type C++ -exclude true")
    lines += [
        "go analyze",
        f"directive set -DESIGN_HIERARCHY {top}",
        "go compile",
        "solution library add nangate-45nm_beh -- -rtlsyntool DesignCompiler -vendor Nangate -technology 045nm",
        "solution library add ccs_sample_mem",
        "go libraries",
        f"directive set -CLOCKS {{clk {{-CLOCK_PERIOD {clock_period_ns}}}}}",
        "go assembly",
        "go architect",
        "go allocate",
        "go schedule",
        "go extract",
    ]
    return "\n".join(lines) + "\n"


# --------------------------------------------------------------------------------------
# Prompt
# --------------------------------------------------------------------------------------


def _format_pragma_table(target: TargetSpec) -> str:
    rows = []
    for rule in target.pragma_rules:
        tgt = rule.target if rule.target is not None else "DROP (no equivalent)"
        row = f"- `{rule.source}` -> `{tgt}`; placement: {rule.placement}"
        if rule.note:
            row += f"; note: {rule.note}"
        rows.append(row)
    return "\n".join(rows)


def _format_map(m: dict[str, str]) -> str:
    return "\n".join(f"- `{k}` -> `{v}`" for k, v in m.items())


def build_translate_prompt(design_name: str, target: TargetSpec) -> str:
    inc = f"{CONTAINER_RUN_AREA}/{target.include_dir_name}"
    out = f"{CONTAINER_RUN_AREA}/{OUTPUT_DIR_NAME}"
    driver_example = (
        render_catapult_run_tcl("<top_function>", ["<kernel>.cpp"], "testbench.cpp")
        if target.name == "catapult"
        else ""
    )
    return (
        f"Translate the Vitis HLS design in './{INPUT_DIR_NAME}' (design name: `{design_name}`) into an "
        f"equivalent {target.display_name} design in './{OUTPUT_DIR_NAME}' in the current working directory.\n"
        "\n"
        "The input is a standalone, already-extracted Vitis HLS design: sources, headers, a testbench, and a "
        "`synth.tcl` whose `set_top` line names the top-level function. The output must be the SAME design, "
        f"same algorithm, same function names, same numerics, expressed in {target.display_name} form.\n"
        "\n"
        "## Steps\n"
        "\n"
        "### Step 1: Inspect the input\n"
        f"- List './{INPUT_DIR_NAME}' and read every source, header, the testbench, and `synth.tcl`.\n"
        "- Identify the top-level function from the `set_top` line.\n"
        "- Write down every `#pragma HLS` line and every Vitis type or header you find. You will account for each one in the report.\n"
        "\n"
        "### Step 2: Copy everything into the output folder\n"
        f"- Copy every file from './{INPUT_DIR_NAME}' into './{OUTPUT_DIR_NAME}', including data files and README.\n"
        "- Do not copy `synth.tcl`; the output gets its own driver script (Step 7).\n"
        "\n"
        "### Step 3: Translate headers and types\n"
        "Headers:\n"
        f"{_format_map(target.header_map)}\n"
        "Types and operations:\n"
        f"{_format_map(target.type_map)}\n"
        "Rules:\n" + "\n".join(f"- {r}" for r in target.extra_rules) + "\n"
        "- Apply the same translation to the testbench. The testbench must keep its checks and still return 0 on success and 1 on failure.\n"
        "\n"
        "### Step 4: Translate pragmas\n"
        "Placement differs between the tools. Vitis pragmas sit INSIDE the loop body; "
        f"{target.display_name} loop pragmas sit on the line BEFORE the loop. Move them.\n"
        f"{_format_pragma_table(target)}\n"
        "\n"
        "### Step 5: Mark the top-level function\n"
        f"- Put `{target.top_marker}` on the line directly before the top-level function definition in the source file (not the header).\n"
        "\n"
        "### Step 6: Compile-check and run the testbench\n"
        "For every .cpp file in the output folder run:\n"
        "```\n"
        f"clang++ -std={target.cxx_std_check} -I{inc} -I{out} -w -fsyntax-only <file.cpp>\n"
        "```\n"
        "Then build and run the testbench with a timeout:\n"
        "```\n"
        f"cd {out} && clang++ -std={target.cxx_std_check} -I{inc} -I{out} -w *.cpp -o testbench.out && timeout 30s ./testbench.out\n"
        "```\n"
        "- Record the commands and their results in `compile_log.txt` in the output folder.\n"
        "- If compilation or the testbench fails, fix the translation and retry, up to 3 times. Never weaken the testbench to make it pass.\n"
        "- Never run a binary without a timeout.\n"
        "\n"
        "### Step 7: Write the driver script\n"
        f"Create `{target.driver_file}` in the output folder. Use exactly this structure, filling in the top function, "
        "one `solution file add` line per source file, and the testbench with `-exclude true`:\n"
        "```\n"
        f"{driver_example}"
        "```\n"
        "\n"
        "### Step 8: Write the translation report\n"
        "Create `translation_report.md` in the output folder with three tables:\n"
        "1. Every original `#pragma HLS` line: file, original text, what it became, or DROPPED with the reason.\n"
        "2. Every type or header change: original, replacement.\n"
        "3. Anything that could change behavior or performance, for example rounding modes, dropped partitions, dropped dependence pragmas.\n"
        "State at the end whether the testbench passed.\n"
        "\n"
        "## Hard Rules\n"
        "- Do not change the algorithm, loop structure, bit widths, signedness, or rounding behavior.\n"
        "- Do not rename functions or reorder function arguments. Only their types change.\n"
        "- No Vitis identifiers may remain in the output: no `ap_int`, `ap_uint`, `ap_fixed`, `hls::stream`, no `#pragma HLS`, no Vitis headers.\n"
        "- Do not delete testbench checks or replace them with placeholders.\n"
        "- Start immediately. Do not only describe the steps.\n"
        "\n"
        "## Pre-Ending Checklist (MANDATORY)\n"
        f"1. `grep -rnE 'ap_int|ap_uint|ap_fixed|hls::stream|pragma HLS' {out}` prints nothing.\n"
        f"2. `{out}` contains at least one .cpp source, `testbench.cpp`, `{target.driver_file}`, `translation_report.md`, and `compile_log.txt`.\n"
        "3. The syntax check passed for every .cpp file and the testbench ran and exited 0, or the report explains exactly why not.\n"
        "Do NOT terminate until all checks pass or are explained in the report.\n"
    )


# --------------------------------------------------------------------------------------
# Deterministic post-checks
# --------------------------------------------------------------------------------------


def _iter_source_files(dir_design: Path) -> list[Path]:
    return sorted(
        p for p in dir_design.rglob("*") if p.is_file() and p.suffix.lower() in SOURCE_SUFFIXES
    )


def find_top_from_synth_tcl(dir_design: Path) -> str | None:
    tcl = dir_design / "synth.tcl"
    if not tcl.exists():
        return None
    m = re.search(r"^\s*set_top\s+(\S+)", tcl.read_text(encoding="utf-8", errors="replace"), re.MULTILINE)
    return m.group(1) if m else None


def check_translated_design(dir_output: Path, target: TargetSpec) -> dict:
    """Static checks on the agent's output. Pure Python, no tools, no container."""
    result: dict = {"target": target.name, "checks": {}, "failures": []}
    checks = result["checks"]

    if not dir_output.exists():
        result["failures"].append("output directory does not exist")
        checks["output_exists"] = False
        return result
    checks["output_exists"] = True

    sources = _iter_source_files(dir_output)
    cpp_sources = [p for p in sources if p.suffix.lower() in (".cpp", ".cc", ".c")]
    checks["has_source"] = len(cpp_sources) > 0
    if not checks["has_source"]:
        result["failures"].append("no .cpp/.c source file in output")

    for fname in target.required_files:
        ok = (dir_output / fname).exists()
        checks[f"has_{fname}"] = ok
        if not ok:
            result["failures"].append(f"missing required file {fname}")

    leftovers: list[dict] = []
    commented: list[dict] = []
    for p in sources:
        text = p.read_text(encoding="utf-8", errors="replace")
        for i, line in enumerate(text.splitlines(), start=1):
            for pat in target.forbidden_patterns:
                if re.search(pat, line):
                    entry = {"file": str(p.relative_to(dir_output)), "line": i, "text": line.strip()}
                    # A line that is entirely a // comment is inert for the target tool; record it separately.
                    (commented if line.lstrip().startswith("//") else leftovers).append(entry)
                    break
    checks["no_source_tool_leftovers"] = len(leftovers) == 0
    result["leftovers"] = leftovers
    result["commented_leftovers"] = commented
    if leftovers:
        result["failures"].append(f"{len(leftovers)} line(s) still use the source tool's dialect")

    has_top_marker = any(target.top_marker in p.read_text(encoding="utf-8", errors="replace") for p in cpp_sources)
    checks["has_top_marker"] = has_top_marker
    if not has_top_marker:
        result["failures"].append(f"top marker `{target.top_marker}` not found in any source")

    driver = dir_output / target.driver_file
    if driver.exists():
        driver_text = driver.read_text(encoding="utf-8", errors="replace")
        missing_tokens = [t for t in target.driver_required_tokens if t not in driver_text]
        checks["driver_complete"] = not missing_tokens
        if missing_tokens:
            result["failures"].append(f"{target.driver_file} missing: {missing_tokens}")
    else:
        checks["driver_complete"] = False

    result["passed"] = not result["failures"]
    return result


def _container_exec(container: Container, cmd: str, workdir: str) -> tuple[int, str]:
    exit_code, output = container.exec_run(["sh", "-lc", cmd], workdir=workdir)
    text = output.decode("utf-8", errors="replace") if isinstance(output, bytes) else str(output)
    return (exit_code if exit_code is not None else -1), text


def run_oracle_check(
    dir_design: Path,
    docker_image_name: str = DOCKER_IMAGE_NAME,
    timeout_s: int = 120,
) -> dict:
    """Compile and run a Vitis design's own testbench against its own sources with clang.

    This is the pre-filter: a design whose testbench does not pass on the original code has no
    oracle for translation. The design is copied into a scratch folder so the testbench may write files.
    """
    import tempfile

    dir_design = Path(dir_design).resolve()
    tmp_root = Path(tempfile.mkdtemp(prefix="oracle_"))
    dir_copy = tmp_root / "input_design"
    shutil.copytree(dir_design, dir_copy)
    os.chmod(dir_copy, 0o777)

    client = docker.from_env()
    container: Container = client.containers.run(
        image=docker_image_name,
        command="sleep 30m",
        detach=True,
        volumes={
            str(dir_copy): {"bind": f"{CONTAINER_RUN_AREA}/{INPUT_DIR_NAME}", "mode": "rw"},
            str(DIR_VITIS_HLS_INCLUDE.resolve()): {"bind": f"{CONTAINER_RUN_AREA}/vitis_hls_include", "mode": "ro"},
        },
    )
    inp = f"{CONTAINER_RUN_AREA}/{INPUT_DIR_NAME}"
    inc = f"{CONTAINER_RUN_AREA}/vitis_hls_include"
    result: dict[str, Any] = {"design": dir_design.name, "top": find_top_from_synth_tcl(dir_design)}
    try:
        code, listing = _container_exec(container, f"ls -1 {inp}/*.cpp {inp}/*.cc {inp}/*.c 2>/dev/null", inp)
        cpp_files = [ln.strip() for ln in listing.splitlines() if ln.strip()]
        result["cpp_files"] = [Path(f).name for f in cpp_files]
        if not cpp_files:
            result["build"] = {"exit_code": None, "output": "no sources"}
            result["run"] = {"exit_code": None, "output": "no sources"}
        else:
            build_cmd = (
                f"clang++ -std=c++17 -I{inc} -I{inp} -w " + " ".join(shlex.quote(f) for f in cpp_files) + " -o /tmp/oracle.out"
            )
            code, output = _container_exec(container, build_cmd, inp)
            result["build"] = {"exit_code": code, "output": output[-4000:]}
            if code == 0:
                code, output = _container_exec(container, f"timeout {timeout_s}s /tmp/oracle.out", inp)
                result["run"] = {"exit_code": code, "output": output[-4000:]}
            else:
                result["run"] = {"exit_code": None, "output": "not run: build failed"}
    finally:
        container.stop()
        container.remove(force=True)
        shutil.rmtree(tmp_root, ignore_errors=True)

    result["passed"] = result["run"]["exit_code"] == 0
    return result


def run_checks_standalone(
    dir_output: Path,
    target: TargetSpec,
    docker_image_name: str = DOCKER_IMAGE_NAME,
) -> dict:
    """Run the in-container compile and testbench checks on an already-translated design folder.

    Useful for hand-translated designs and for validating the check path without an LLM.
    """
    dir_output = Path(dir_output).resolve()
    client = docker.from_env()
    container: Container = client.containers.run(
        image=docker_image_name,
        command="sleep 30m",
        detach=True,
        volumes={
            str(dir_output): {"bind": f"{CONTAINER_RUN_AREA}/{OUTPUT_DIR_NAME}", "mode": "ro"},
            str(target.include_dir.resolve()): {"bind": f"{CONTAINER_RUN_AREA}/{target.include_dir_name}", "mode": "ro"},
        },
    )
    try:
        return {
            "static": check_translated_design(dir_output, target),
            "container": run_container_checks(container, target),
        }
    finally:
        container.stop()
        container.remove(force=True)


def run_container_checks(container: Container, target: TargetSpec) -> dict:
    """Independent compile and testbench checks, run inside the container after the agent exits."""
    inc = f"{CONTAINER_RUN_AREA}/{target.include_dir_name}"
    out = f"{CONTAINER_RUN_AREA}/{OUTPUT_DIR_NAME}"
    std = target.cxx_std_check
    results: dict = {}

    code, listing = _container_exec(container, f"ls -1 {out}/*.cpp {out}/*.cc {out}/*.c 2>/dev/null", out)
    cpp_files = [ln.strip() for ln in listing.splitlines() if ln.strip()]
    results["cpp_files"] = [Path(f).name for f in cpp_files]

    syntax: dict[str, dict] = {}
    for f in cpp_files:
        code, output = _container_exec(
            container, f"clang++ -std={std} -I{inc} -I{out} -w -fsyntax-only {shlex.quote(f)}", out
        )
        syntax[Path(f).name] = {"exit_code": code, "output": output[-4000:]}
    results["syntax_check"] = syntax
    results["syntax_all_ok"] = bool(cpp_files) and all(v["exit_code"] == 0 for v in syntax.values())

    if cpp_files:
        build_cmd = f"clang++ -std={std} -I{inc} -I{out} -w " + " ".join(shlex.quote(f) for f in cpp_files) + " -o /tmp/tb_check.out"
        code, output = _container_exec(container, build_cmd, out)
        results["testbench_build"] = {"exit_code": code, "output": output[-4000:]}
        if code == 0:
            code, output = _container_exec(container, "timeout 60s /tmp/tb_check.out", out)
            results["testbench_run"] = {"exit_code": code, "output": output[-4000:]}
        else:
            results["testbench_run"] = {"exit_code": None, "output": "not run: build failed"}
    else:
        results["testbench_build"] = {"exit_code": None, "output": "no sources"}
        results["testbench_run"] = {"exit_code": None, "output": "no sources"}

    results["testbench_ok"] = results["testbench_run"]["exit_code"] == 0
    return results


def _rmtree_robust(path: Path, attempts: int = 5, delay_s: float = 1.0) -> None:
    """rmtree that survives Windows read-only bits and transient file locks (OneDrive, antivirus)."""
    import stat
    import sys
    import time

    def _retry_writable(func, p, _exc):
        os.chmod(p, stat.S_IWRITE)
        func(p)

    last: Exception | None = None
    for i in range(attempts):
        try:
            if sys.version_info >= (3, 12):
                shutil.rmtree(path, onexc=_retry_writable)
            else:
                shutil.rmtree(path, onerror=_retry_writable)  # pragma: no cover
            return
        except Exception as e:  # PermissionError, OSError from locks
            last = e
            time.sleep(delay_s * (i + 1))
    raise RuntimeError(f"Could not remove {path} after {attempts} attempts: {last!r}")


# --------------------------------------------------------------------------------------
# Run class
# --------------------------------------------------------------------------------------


class HLSTranslationRun:
    """Translate one standalone design directory into a target tool's dialect using the Pi agent.

    Mirrors ``HLSFactoryAgentRun``: same Docker image, same Pi settings, same session capture.
    Differences: the input is a local design folder, and deterministic checks run after the agent.
    """

    def __init__(
        self,
        run_id: str,
        dir_design: Path,
        dir_work: Path,
        model_name: str,
        api_key: str,
        target: TargetSpec | str = CATAPULT,
        docker_image_name: str = DOCKER_IMAGE_NAME,
        agent_timeout_s: int = 60 * 30,
    ):
        self.run_id = run_id
        self.dir_design = Path(dir_design).resolve()
        self.dir_work = Path(dir_work)
        self.model_name = model_name
        self.api_key = api_key
        self.target = get_target(target) if isinstance(target, str) else target
        self.docker_image_name = docker_image_name
        self.agent_timeout_s = agent_timeout_s

    # ---- workspace -------------------------------------------------------------------

    def prepare_run_area(self) -> Path:
        if not self.dir_design.is_dir():
            raise FileNotFoundError(f"Design directory not found: {self.dir_design}")
        if self.dir_work.exists():
            _rmtree_robust(self.dir_work)
        self.dir_work.mkdir(parents=True, exist_ok=True)

        dir_run_area = self.dir_work / "run_area"
        dir_run_area.mkdir(parents=True, exist_ok=True)

        dir_pi = dir_run_area / ".pi"
        dir_pi.mkdir(parents=True, exist_ok=True)
        (dir_pi / "settings.json").write_text(
            encoding="utf-8",
            data=json.dumps(
                {"defaultProvider": "openrouter", "defaultModel": self.model_name, "sessionDir": ".pi/sessions"},
                indent=4,
            )
        )
        dir_sessions = dir_pi / "sessions"
        dir_sessions.mkdir(parents=True, exist_ok=True)
        os.chmod(dir_pi, 0o777)
        os.chmod(dir_sessions, 0o777)

        shutil.copytree(self.dir_design, dir_run_area / INPUT_DIR_NAME)
        shutil.copytree(self.target.include_dir, dir_run_area / self.target.include_dir_name)
        shutil.copytree(DIR_VITIS_HLS_INCLUDE, dir_run_area / "vitis_hls_include")
        (dir_run_area / OUTPUT_DIR_NAME).mkdir(parents=True, exist_ok=True)
        os.chmod(dir_run_area / OUTPUT_DIR_NAME, 0o777)
        return dir_run_area

    # ---- main ------------------------------------------------------------------------

    def run(self) -> dict:
        design_name = self.dir_design.name
        print(f"Translating design `{design_name}` -> {self.target.display_name} (run `{self.run_id}`)")

        run_data: dict = {
            "run_id": self.run_id,
            "design_name": design_name,
            "design_dir": str(self.dir_design),
            "target": self.target.name,
            "model_name": self.model_name,
            "docker_image_name": self.docker_image_name,
        }

        dir_run_area = self.prepare_run_area()
        prompt = build_translate_prompt(design_name, self.target)
        run_data["prompt_task"] = prompt

        client = docker.from_env()
        container: Container = client.containers.run(
            image=self.docker_image_name,
            command="sleep 2h",
            detach=True,
            volumes={str(dir_run_area.resolve()): {"bind": CONTAINER_RUN_AREA, "mode": "rw"}},
        )
        try:
            cmd = f"umask 000 && timeout {self.agent_timeout_s}s pi -p {shlex.quote(prompt)}"
            exit_code, output_agent = container.exec_run(
                ["sh", "-lc", cmd],
                environment={"OPENROUTER_API_KEY": self.api_key},
                workdir=CONTAINER_RUN_AREA,
            )
            run_data["agent_exit_code"] = exit_code
            run_data["agent_stdout_tail"] = (
                output_agent.decode("utf-8", errors="replace")[-8000:] if isinstance(output_agent, bytes) else str(output_agent)[-8000:]
            )

            dir_sessions = dir_run_area / ".pi" / "sessions"
            session_file = next(dir_sessions.glob("*.jsonl"), None) if dir_sessions.exists() else None
            if session_file is not None:
                run_data["session_data"] = load_jsonl_text(session_file.read_text(encoding="utf-8", errors="replace"))
                container.exec_run(
                    [
                        "sh",
                        "-lc",
                        f"umask 000 && pi --export {CONTAINER_RUN_AREA}/.pi/sessions/{session_file.name} "
                        f"{CONTAINER_RUN_AREA}/.pi/sessions/{session_file.name.replace('.jsonl', '.html')}",
                    ],
                    workdir=CONTAINER_RUN_AREA,
                )
            else:
                run_data["session_data"] = None

            dir_output = dir_run_area / OUTPUT_DIR_NAME
            if not (dir_output / self.target.driver_file).exists() and self.target.name == "catapult":
                self._render_missing_driver(dir_output)

            check_data: dict[str, Any] = {
                "static": check_translated_design(dir_output, self.target),
                "container": run_container_checks(container, self.target),
            }
        finally:
            container.stop()
            container.remove(force=True)

        check_data["passed"] = bool(check_data["static"]["passed"] and check_data["container"]["testbench_ok"])
        (self.dir_work / "run_data.json").write_text(json.dumps(run_data, indent=4), encoding="utf-8")
        (self.dir_work / "check_data.json").write_text(json.dumps(check_data, indent=4), encoding="utf-8")
        print(f"Translation `{self.run_id}`: passed={check_data['passed']} failures={check_data['static']['failures']}")
        return check_data

    def _render_missing_driver(self, dir_output: Path) -> None:
        top = find_top_from_synth_tcl(self.dir_design) or "top"
        sources = [p.name for p in _iter_source_files(dir_output) if p.suffix.lower() in (".cpp", ".cc", ".c") and p.name != "testbench.cpp"]
        tb = "testbench.cpp" if (dir_output / "testbench.cpp").exists() else None
        (dir_output / self.target.driver_file).write_text(render_catapult_run_tcl(top, sources, tb), encoding="utf-8")
