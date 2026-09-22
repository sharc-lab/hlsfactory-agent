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
from pathlib import Path
from typing import Any

import docker
from docker.models.containers import Container

from hlsfactory_agent.spec import PragmaRule as PragmaRule
from hlsfactory_agent.spec import SynthResult as SynthResult
from hlsfactory_agent.spec import TargetSpec
from hlsfactory_agent.targets.catapult import (
    CATAPULT,
    directive_template,
    load_directive_rules as load_directive_rules,
    render_catapult_run_tcl,
    render_directives,
)
from hlsfactory_agent.targets.xlscc import XLSCC, render_xlscc_driver
from hlsfactory_agent.utils import load_jsonl_text

DIR_CURRENT = Path(__file__).resolve().parent
DIR_VITIS_HLS_INCLUDE = DIR_CURRENT / "vitis_hls_include"
DIR_AC_TYPES_INCLUDE = DIR_CURRENT / "ac_types_include"
DIR_TARGETS = DIR_CURRENT / "targets"
DIR_XLS_EMU_INCLUDE = DIR_CURRENT / "xls_emu_include"

DOCKER_IMAGE_NAME = "hlsfactory-agent"

CONTAINER_RUN_AREA = "/workspace/run_area"
INPUT_DIR_NAME = "input_design"
OUTPUT_DIR_NAME = "output_design"

SOURCE_SUFFIXES = (".cpp", ".cc", ".c", ".h", ".hpp")


# --------------------------------------------------------------------------------------
# Target descriptions
# --------------------------------------------------------------------------------------


TARGETS: dict[str, TargetSpec] = {CATAPULT.name: CATAPULT, XLSCC.name: XLSCC}


def get_target(name: str) -> TargetSpec:
    try:
        return TARGETS[name]
    except KeyError as e:
        raise ValueError(f"Unknown translation target `{name}`. Known: {sorted(TARGETS)}") from e


def feasibility(scan: dict, target: TargetSpec) -> list[dict]:
    blocked = []
    for t in scan.get("types", []):
        if t["kind"] in target.blocking_types and Path(t["file"]).name != "testbench.cpp":
            blocked.append(
                {"kind": t["kind"], "file": t["file"], "line": t["line"], "reason": f"{target.display_name} cannot represent `{t['kind']}`"}
            )
    return blocked


def untranslatable_check_data(blockers: list[dict], target: TargetSpec) -> dict:
    not_run = {"exit_code": None, "output": "not run: untranslatable"}
    return {
        "untranslatable": blockers,
        "static": {"passed": False, "checks": {}, "failures": [f"untranslatable for {target.name}: {b['reason']}" for b in blockers], "leftovers": []},
        "container": {"cpp_files": [], "syntax_check": {}, "syntax_all_ok": False, "testbench_build": not_run, "testbench_run": not_run, "testbench_ok": False},
        "attempts": [],
        "attempts_used": 0,
        "passed": False,
    }


# --------------------------------------------------------------------------------------
# Driver script rendering
# --------------------------------------------------------------------------------------


def render_driver(target: TargetSpec, top: str, sources: list[str], testbench: str | None) -> str:
    if target.name == "xlscc":
        return render_xlscc_driver(top, sources)
    return render_catapult_run_tcl(top, sources, testbench)


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


def build_translate_prompt(design_name: str, target: TargetSpec, prepass: bool = False) -> str:
    inc = " ".join(f"-I{CONTAINER_RUN_AREA}/{name}" for name in target.include_dir_names)
    out = f"{CONTAINER_RUN_AREA}/{OUTPUT_DIR_NAME}"
    driver_example = render_driver(target, "<top_function>", ["<kernel>.cpp"], "testbench.cpp")
    if prepass:
        step2 = (
            "### Step 2: A mechanical pre-pass already ran\n"
            f"'./{OUTPUT_DIR_NAME}' already contains every input file with headers, types, and loop pragmas rewritten by a script, "
            "and `rewrite_log.json` listing what it changed, what it dropped, and a `residue` list of what it could not handle.\n"
            "- Read `rewrite_log.json` first. Fix every residue item by hand.\n"
            "- Review the moved pragmas: the script places a loop pragma on the line before the nearest loop header; correct any it placed wrongly.\n"
            "- Do not redo the mechanical work; do verify it in Step 6.\n"
            "- `synth.tcl` was intentionally not copied; the output gets its own driver script (Step 7).\n"
            f"- `{target.driver_file}` and `directives.json` were generated from the input's resource pragmas. "
            f"Do NOT rewrite `{target.driver_file}` from scratch. For every line in it starting with `# UNRESOLVED`, "
            f"work out the numeric value (the expression and the #defines are in `{CONTAINER_RUN_AREA}/scan.json`), "
            "replace that whole comment line with the finished directive, and leave every other line alone.\n"
        )
    else:
        step2 = (
            "### Step 2: Copy everything into the output folder\n"
            f"- Copy every file from './{INPUT_DIR_NAME}' into './{OUTPUT_DIR_NAME}', including data files and README.\n"
            "- Do not copy `synth.tcl`; the output gets its own driver script (Step 7).\n"
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
        f"- `{CONTAINER_RUN_AREA}/scan.json` lists every `#pragma HLS` kind, vendor type, and header in the input with counts. "
        "Every pragma kind listed there must appear in your report, translated or DROPPED.\n"
        "\n"
        + step2 +
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
        f"clang++ -std={target.cxx_std_check} {inc} -I{out} -w -fsyntax-only <file.cpp>\n"
        "```\n"
        "Then build and run the testbench with a timeout:\n"
        "```\n"
        f"cd {out} && clang++ -std={target.cxx_std_check} {inc} -I{out} -w *.cpp -o testbench.out && timeout 30s ./testbench.out\n"
        "```\n"
        "- Record the commands and their results in `compile_log.txt` in the output folder.\n"
        "- If compilation or the testbench fails, fix the translation and retry, up to 3 times. Never weaken the testbench to make it pass.\n"
        "- Never run a binary without a timeout.\n"
        "\n"
        "### Step 7: Write the driver script\n"
        f"Create `{target.driver_file}` in the output folder. Use exactly this structure, filling in the top function and the "
        "source file names (for Catapult: one `solution file add` line per source file and the testbench with `-exclude true`; "
        "for xlscc: the kernel sources only, never the testbench, and make the script executable):\n"
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


def check_translated_design(dir_output: Path, target: TargetSpec, scan: dict | None = None) -> dict:
    """Static checks on the agent's output. Pure Python, no tools, no container.

    When ``scan`` (the scanner inventory of the input) is given, the translation report must mention
    every pragma kind in it, translated or dropped, or the check fails.
    """
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

    if scan is not None:
        report = dir_output / "translation_report.md"
        report_text = report.read_text(encoding="utf-8", errors="replace").lower() if report.exists() else ""
        kinds = sorted(scan.get("counts", {}).get("pragmas", {}).keys())
        unaccounted = [
            k
            for k in kinds
            if not re.search(rf"#\s*pragma\s+hls\s+{re.escape(k)}\b", report_text)
            and not re.search(rf"\b{re.escape(k)}\b", report_text)
        ]
        checks["report_covers_all_pragma_kinds"] = not unaccounted
        result["unaccounted_pragma_kinds"] = unaccounted
        if unaccounted:
            result["failures"].append(f"report does not account for pragma kinds: {unaccounted}")

    dfile = dir_output / "directives.json"
    if dfile.exists():
        d = json.loads(dfile.read_text(encoding="utf-8", errors="replace"))
        script_text = driver.read_text(encoding="utf-8", errors="replace") if driver.exists() else ""
        missing = [ln for ln in d.get("rendered", []) if ln not in script_text]
        leftover = [ln for ln in script_text.splitlines() if ln.startswith("# UNRESOLVED")]
        result["directive_failures"] = [f"missing: {m}" for m in missing] + [f"unresolved: {ln}" for ln in leftover]
        checks["directives_in_script"] = not result["directive_failures"]
        if result["directive_failures"]:
            result["failures"].append(
                f"{len(result['directive_failures'])} directive(s) missing or unresolved in {target.driver_file}"
            )

    result["passed"] = not result["failures"]
    return result


def _container_exec(container: Container, cmd: str, workdir: str) -> tuple[int, str]:
    exit_code, output = container.exec_run(["sh", "-lc", cmd], workdir=workdir)
    text = output.decode("utf-8", errors="replace") if isinstance(output, bytes) else str(output)
    return (exit_code if exit_code is not None else -1), text


_RE_NEG_ZERO = re.compile(r"-(0\.0+)\b")
_RE_SIM_NOISE = re.compile(r"^(INFO|WARNING|ERROR) \[(HLS SIM|SIM)\]:", re.IGNORECASE)


def _normalize_output(text: str) -> list[str]:
    """Whitespace-collapsed, negative-zero-free, non-empty lines of a testbench's stdout."""
    lines: list[str] = []
    for raw in text.splitlines():
        s = " ".join(raw.split())
        s = _RE_NEG_ZERO.sub(r"\1", s)
        if s and not _RE_SIM_NOISE.match(s):
            lines.append(s)
    return lines


def compare_outputs(original: str, translated: str) -> dict:
    """Line-by-line comparison of the original testbench's stdout with the translated one's.

    The original design is the oracle, so any printed value that differs is a translation concern,
    even when both testbenches report PASS.
    """
    a, b = _normalize_output(original), _normalize_output(translated)
    n = max(len(a), len(b))
    differing = 0
    sample: list[str] = []
    for i in range(n):
        x = a[i] if i < len(a) else ""
        y = b[i] if i < len(b) else ""
        if x != y:
            differing += 1
            if len(sample) < 5:
                sample.append(f"{x} | {y}")
    return {
        "identical": differing == 0,
        "original_lines": len(a),
        "translated_lines": len(b),
        "differing": differing,
        "sample": sample,
    }


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
            **{
                str(src.resolve()): {"bind": f"{CONTAINER_RUN_AREA}/{name}", "mode": "ro"}
                for src, name in target.extra_include_dirs
            },
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
    inc = " ".join(f"-I{CONTAINER_RUN_AREA}/{name}" for name in target.include_dir_names)
    out = f"{CONTAINER_RUN_AREA}/{OUTPUT_DIR_NAME}"
    std = target.cxx_std_check
    results: dict = {}

    code, listing = _container_exec(container, f"ls -1 {out}/*.cpp {out}/*.cc {out}/*.c 2>/dev/null", out)
    cpp_files = [ln.strip() for ln in listing.splitlines() if ln.strip()]
    results["cpp_files"] = [Path(f).name for f in cpp_files]

    syntax: dict[str, dict] = {}
    for f in cpp_files:
        code, output = _container_exec(
            container, f"clang++ -std={std} {inc} -I{out} -w -fsyntax-only {shlex.quote(f)}", out
        )
        syntax[Path(f).name] = {"exit_code": code, "output": output[-4000:]}
    results["syntax_check"] = syntax
    results["syntax_all_ok"] = bool(cpp_files) and all(v["exit_code"] == 0 for v in syntax.values())

    if cpp_files:
        build_cmd = f"clang++ -std={std} {inc} -I{out} -w " + " ".join(shlex.quote(f) for f in cpp_files) + " -o /tmp/tb_check.out"
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


def build_retry_prompt(base_prompt: str, check_data: dict) -> str:
    """The base prompt plus what the harness found wrong with the previous attempt."""
    static = check_data.get("static", {})
    cont = check_data.get("container", {})
    lines = [
        "",
        "## Previous attempt failed",
        "The harness checked your previous output and found these problems. Fix them in place in "
        f"'./{OUTPUT_DIR_NAME}'. Do not start over. Then rerun Step 6 and the checklist.",
        "",
    ]
    for f in static.get("failures", []):
        lines.append(f"- static check: {f}")
    for k in static.get("unaccounted_pragma_kinds", []):
        lines.append(f"- report does not mention pragma kind `{k}`")
    for name, v in (cont.get("syntax_check") or {}).items():
        if v.get("exit_code") not in (0, None):
            tail = "\n".join((v.get("output") or "").splitlines()[-40:])
            lines += [f"- syntax check failed for {name}:", "```", tail, "```"]
    for key in ("testbench_build", "testbench_run"):
        v = cont.get(key) or {}
        if v.get("exit_code") not in (0, None):
            tail = "\n".join((v.get("output") or "").splitlines()[-40:])
            lines += [f"- {key.replace('_', ' ')} failed:", "```", tail, "```"]
    return base_prompt + "\n".join(lines) + "\n"


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
        prepass: bool = True,
        attempts: int = 1,
    ):
        self.run_id = run_id
        self.dir_design = Path(dir_design).resolve()
        self.dir_work = Path(dir_work)
        self.model_name = model_name
        self.api_key = api_key
        self.target = get_target(target) if isinstance(target, str) else target
        self.docker_image_name = docker_image_name
        self.agent_timeout_s = agent_timeout_s
        self.prepass = prepass
        self.attempts = max(1, int(attempts))

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
        for src, name in self.target.extra_include_dirs:
            shutil.copytree(src, dir_run_area / name)
        shutil.copytree(DIR_VITIS_HLS_INCLUDE, dir_run_area / "vitis_hls_include")
        from hlsfactory_agent.scan import write_scan

        write_scan(dir_run_area / INPUT_DIR_NAME, dir_run_area, top=find_top_from_synth_tcl(self.dir_design))
        dir_output = dir_run_area / OUTPUT_DIR_NAME
        if self.prepass:
            from hlsfactory_agent.rewrite import rewrite_design

            top = find_top_from_synth_tcl(self.dir_design)
            rewrite_design(dir_run_area / INPUT_DIR_NAME, dir_output, self.target, top)
            if self.target.directive_rules:
                self._write_directives(dir_run_area, dir_output, top or "top")
        else:
            dir_output.mkdir(parents=True, exist_ok=True)
        os.chmod(dir_output, 0o777)
        return dir_run_area

    def _write_directives(self, dir_run_area: Path, dir_output: Path, top: str) -> None:
        scan = json.loads((dir_run_area / "scan.json").read_text(encoding="utf-8"))
        rendered, unrendered = render_directives(scan.get("resources", []), top, self.target.directive_rules)
        (dir_output / "directives.json").write_text(
            json.dumps({"rendered": rendered, "unrendered": unrendered}, indent=2), encoding="utf-8"
        )
        comments = []
        for u in unrendered:
            r = u["resource"]
            template = directive_template(r, self.target.directive_rules)
            if template is None:
                continue
            hint = template.replace("{top}", top).replace("{var}", r["args"].get("variable", "?")).replace("{factor}", "<factor>")
            comments.append(f"# UNRESOLVED {u['reason']}: {hint}")
        sources = [
            f.name for f in _iter_source_files(dir_output)
            if f.suffix.lower() in (".cpp", ".cc", ".c") and f.name != "testbench.cpp"
        ]
        tb = "testbench.cpp" if (dir_output / "testbench.cpp").exists() else None
        (dir_output / self.target.driver_file).write_text(
            render_catapult_run_tcl(top, sources, tb, directives=rendered + comments), encoding="utf-8"
        )

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
        prompt = build_translate_prompt(design_name, self.target, prepass=self.prepass)
        run_data["prompt_task"] = prompt
        run_data["prepass"] = self.prepass
        scan_path = dir_run_area / "scan.json"
        scan = json.loads(scan_path.read_text(encoding="utf-8")) if scan_path.exists() else {}
        blockers = feasibility(scan, self.target)
        if blockers:
            check_data = untranslatable_check_data(blockers, self.target)
            run_data["untranslatable"] = blockers
            (self.dir_work / "run_data.json").write_text(json.dumps(run_data, indent=4), encoding="utf-8")
            (self.dir_work / "check_data.json").write_text(json.dumps(check_data, indent=4), encoding="utf-8")
            print(f"Translation `{self.run_id}`: untranslatable for {self.target.name}: {blockers[0]['reason']}")
            return check_data

        client = docker.from_env()
        container: Container = client.containers.run(
            image=self.docker_image_name,
            command="sleep 2h",
            detach=True,
            volumes={str(dir_run_area.resolve()): {"bind": CONTAINER_RUN_AREA, "mode": "rw"}},
        )
        try:
            scan_path = dir_run_area / "scan.json"
            scan = json.loads(scan_path.read_text(encoding="utf-8")) if scan_path.exists() else None
            dir_output = dir_run_area / OUTPUT_DIR_NAME
            attempts_log: list[dict] = []
            run_data["sessions"] = []
            prompt_this = prompt
            check_data: dict[str, Any] = {}
            for attempt in range(1, self.attempts + 1):
                self._run_agent_once(container, prompt_this, dir_run_area, run_data, attempt)
                if not (dir_output / self.target.driver_file).exists():
                    self._render_missing_driver(dir_output)
                check_data = {
                    "static": check_translated_design(dir_output, self.target, scan=scan),
                    "container": run_container_checks(container, self.target),
                }
                passed = bool(check_data["static"]["passed"] and check_data["container"]["testbench_ok"])
                attempts_log.append({"attempt": attempt, "passed": passed, "failures": list(check_data["static"]["failures"])})
                if passed:
                    break
                prompt_this = build_retry_prompt(prompt, check_data)
            check_data["attempts"] = attempts_log
            check_data["attempts_used"] = len(attempts_log)
        finally:
            container.stop()
            container.remove(force=True)

        check_data["passed"] = bool(check_data["static"]["passed"] and check_data["container"]["testbench_ok"])
        (self.dir_work / "run_data.json").write_text(json.dumps(run_data, indent=4), encoding="utf-8")
        (self.dir_work / "check_data.json").write_text(json.dumps(check_data, indent=4), encoding="utf-8")
        print(f"Translation `{self.run_id}`: passed={check_data['passed']} failures={check_data['static']['failures']}")
        return check_data

    def _run_agent_once(self, container: Container, prompt_text: str, dir_run_area: Path, run_data: dict, attempt: int) -> None:
        """Run Pi once with the given prompt and capture the session it produced."""
        cmd = f"umask 000 && timeout {self.agent_timeout_s}s pi -p {shlex.quote(prompt_text)}"
        exit_code, output_agent = container.exec_run(
            ["sh", "-lc", cmd],
            environment={"OPENROUTER_API_KEY": self.api_key},
            workdir=CONTAINER_RUN_AREA,
        )
        tail = output_agent.decode("utf-8", errors="replace")[-8000:] if isinstance(output_agent, bytes) else str(output_agent)[-8000:]
        run_data["agent_exit_code"] = exit_code
        run_data["agent_stdout_tail"] = tail

        dir_sessions = dir_run_area / ".pi" / "sessions"
        seen = {s["file"] for s in run_data["sessions"] if s.get("file")}
        candidates = (
            sorted((p for p in dir_sessions.glob("*.jsonl") if p.name not in seen), key=lambda p: p.stat().st_mtime)
            if dir_sessions.exists()
            else []
        )
        session_file = candidates[-1] if candidates else None
        if session_file is None:
            run_data["session_data"] = None
            run_data["sessions"].append({"attempt": attempt, "file": None, "exit_code": exit_code, "events": 0})
            return
        events = load_jsonl_text(session_file.read_text(encoding="utf-8", errors="replace"))
        run_data["session_data"] = events  # the latest attempt; session_stats in run_repo reads this
        run_data["sessions"].append({"attempt": attempt, "file": session_file.name, "exit_code": exit_code, "events": len(events)})
        container.exec_run(
            [
                "sh",
                "-lc",
                f"umask 000 && pi --export {CONTAINER_RUN_AREA}/.pi/sessions/{session_file.name} "
                f"{CONTAINER_RUN_AREA}/.pi/sessions/{session_file.name.replace('.jsonl', '.html')}",
            ],
            workdir=CONTAINER_RUN_AREA,
        )

    def _render_missing_driver(self, dir_output: Path) -> None:
        top = find_top_from_synth_tcl(self.dir_design) or "top"
        sources = [p.name for p in _iter_source_files(dir_output) if p.suffix.lower() in (".cpp", ".cc", ".c") and p.name != "testbench.cpp"]
        tb = "testbench.cpp" if (dir_output / "testbench.cpp").exists() else None
        (dir_output / self.target.driver_file).write_text(render_driver(self.target, top, sources, tb), encoding="utf-8")
