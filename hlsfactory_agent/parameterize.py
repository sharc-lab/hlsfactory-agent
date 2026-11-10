"""Agentic design-space parameterization: run the Pi agent to turn one
already-extracted HLS design into many validated, synthesized variants.

Mirrors translate.py's HLSTranslationRun: one Docker container held open
across a bounded retry loop, pure-Python (static) validation followed by
real compile/testbench checks run inside that same container, then
synthesis as a separate pass over whichever points pass validation.

The design-space / rendering engine itself (explicit and implicit spaces,
Jinja and preprocessor-macro rendering) lives in ``parameterization.py``,
ported from hls-eval -- this module is the orchestrator that calls into it,
not a second copy of it.

The agent writes Jinja syntax (``{{ }}`` / ``{% %}``) directly into the real
kernel source files -- no separate ``.jinja``-suffixed template file,
matching hls-eval's original convention, since parameterization.py's flows
operate on a name -> content dict in memory and don't care about file
extensions either way. Jinja usage is detected by scanning the submitted
kernel files' content for ``{{`` / ``{%`` / ``{#``, not by filename.
"""

from __future__ import annotations

import json
import math
import os
import re
import shlex
import shutil
import time
from pathlib import Path
from typing import Any

import docker
from docker.models.containers import Container
from jinja2.exceptions import TemplateError

from hlsfactory_agent.parameterization import (
    DesignSpaceExplicit,
    DesignSpaceImplicit,
    JinjaParamaterizationFlow,
    PreprocessorParamaterizationFlow,
    T_sources,
    convert_design_space_implicit_to_explicit,
)
from hlsfactory_agent.spec import SynthResult, TargetSpec
from hlsfactory_agent.synth import run_synth_cached
from hlsfactory_agent.targets.vitis import VITIS
from hlsfactory_agent.translate import find_top_from_synth_tcl
from hlsfactory_agent.utils import load_jsonl_text

DIR_CURRENT = Path(__file__).resolve().parent
DIR_VITIS_HLS_INCLUDE = DIR_CURRENT / "vitis_hls_include"

DOCKER_IMAGE_NAME = "hlsfactory-agent"
CONTAINER_RUN_AREA = "/workspace/run_area"
INPUT_DIR_NAME = "input"
ORIGINAL_DIR_NAME = "original"  # untouched reference copy, used only for the protected-files diff
POINTS_DIR_NAME = "points"

TESTBENCH_FILENAME = "testbench.cpp"
SOURCE_SUFFIXES = (".cpp", ".cc", ".c")

# The agent signals which kind of design space it submitted by which of
# these two filenames it writes -- decided over the format-sniffing
# alternative so a malformed submission gets one clean, correctly-attributed
# error instead of two unrelated ones: ambiguous failure feedback is worse
# here than in most codebases, because the agent is the one that has to
# read and act on it.
DESIGN_SPACE_EXPLICIT_FILENAME = "design_space.jsonl"
DESIGN_SPACE_IMPLICIT_FILENAME = "design_space_implicit.json"


def _container_exec(container: Container, cmd: str, workdir: str) -> tuple[int, str]:
    exit_code, output = container.exec_run(["sh", "-lc", cmd], workdir=workdir)
    text = output.decode("utf-8", errors="replace") if isinstance(output, bytes) else str(output)
    return (exit_code if exit_code is not None else -1), text


def find_sources_from_synth_tcl(dir_design: Path) -> list[str]:
    """Kernel source files, from the non -tb ``add_files`` lines in synth.tcl.

    Falls back to every ``*.cpp``/``*.cc``/``*.c`` in the design directory
    except ``testbench.cpp`` if synth.tcl is missing or has no add_files
    lines, so a malformed synth.tcl doesn't just silently parameterize
    nothing.
    """
    tcl = dir_design / "synth.tcl"
    sources: list[str] = []
    if tcl.exists():
        for line in tcl.read_text(encoding="utf-8", errors="replace").splitlines():
            m = re.match(r"\s*add_files\s+(?!-tb)(\S+)", line)
            if m:
                sources.append(m.group(1))
    if sources:
        return sources
    return [
        p.name
        for p in sorted(dir_design.iterdir())
        if p.is_file() and p.suffix.lower() in SOURCE_SUFFIXES and p.name != TESTBENCH_FILENAME
    ]


def _uses_jinja(sources: T_sources) -> bool:
    return any(marker in content for content in sources.values() for marker in ("{{", "{%", "{#"))


def build_parameterization_prompt(
    design_name: str, kernel_files: list[str], max_design_points: int
) -> str:
    """Task prompt for one parameterization run, adapted from hls-eval's
    prompt sections to this repo's conventions and extended to cover the
    explicit-vs-implicit submission choice."""
    files_list = ", ".join(f"`{f}`" for f in kernel_files)
    return (
        f"You are optimizing the complete, working Vitis HLS design `{design_name}` in "
        f"'./{INPUT_DIR_NAME}'. Read every source, header, and the testbench first.\n"
        "\n"
        "Your goal is to turn the kernel implementation into a design-space-parameterized "
        "version, so that multiple distinct parameter assignments each yield a distinct, "
        "working hardware implementation. Together these implementations should trace out a "
        "useful latency/resource tradeoff: some fast and large, some slow and small, and "
        "points in between.\n"
        "\n"
        "## Input / output\n"
        f"Modify only these existing kernel implementation files: {files_list}. "
        "Do not parameterize headers or the testbench. Preserve every other original file "
        "byte for byte, including every header, the testbench, input data, and `synth.tcl`.\n"
        "\n"
        "Parameterize using Jinja syntax written directly into the kernel files above: "
        "`{{ factor }}` for values, `{% if pipeline %} ... {% endif %}` for conditional code. "
        "Do not create separate `.jinja` template files -- write the Jinja syntax directly "
        "into the real `.cpp`/`.h` files named above.\n"
        "\n"
        "Then choose ONE of the following two ways to describe the design space, and write "
        f"EXACTLY ONE of these two files in './{INPUT_DIR_NAME}':\n"
        "\n"
        f"**Option A -- explicit** (`{DESIGN_SPACE_EXPLICIT_FILENAME}`): one complete parameter "
        f"assignment per nonblank line, as a JSON object, between 2 and {max_design_points} "
        "distinct lines. This is an explicit list of points, not a schema or range. Example:\n"
        '```\n{"unroll_factor": 1, "pipeline": false}\n{"unroll_factor": 4, "pipeline": true}\n```\n'
        "\n"
        f"**Option B -- implicit** (`{DESIGN_SPACE_IMPLICIT_FILENAME}`): a single JSON object "
        "naming each parameter's allowed values, plus optional constraints (Jinja boolean "
        "expressions over the parameter names) to exclude combinations that don't make sense. "
        f"Every combination satisfying all constraints is tried, capped at {max_design_points} "
        "points. Example:\n"
        '```\n{"parameters": {"unroll_factor": [1, 2, 4], "pipeline": [false, true]},\n'
        ' "constraints": []}\n```\n'
        "\n"
        "Use string keys and finite string/number/boolean values only, no arrays, nulls, or "
        "nested objects as parameter values. Every template variable must have a value in "
        "every point; missing variables fail rendering.\n"
        "\n"
        "Validation strictly re-renders every point and fails it if the rendered source still "
        "contains a literal `{{`, `{%`, or `{#` anywhere, even inside a `{% raw %}` block. If "
        "your C/C++ contains a literal double brace (e.g. a nested initializer like "
        "`{{0,1,2}, {3,4,5}}`), reformat it so no `{{` substring appears in the rendered output "
        "(add a space: `{ {0,1,2}, {3,4,5} }`), or emit the brace explicitly with "
        "`{{ '{' }}`, rather than relying on `{% raw %}`.\n"
        "\n"
        "## Optimization\n"
        "You are not required to keep the original implementation's loop structure or "
        "architecture, as long as every rendered point stays functionally equivalent at the "
        "top-level interface: identical outputs for all valid inputs. Seek useful tradeoffs "
        "through loop unrolling, pipelining, array partitioning, and storage/resource choices. "
        "Choose legal factors and compatible combinations; most points should be distinct, "
        "nondominated hardware tradeoffs.\n"
        "\n"
        "Preserve the exact top-level function, its linkage, parameter and return types, and "
        "behavior for all valid inputs at every point. Do not change numerical precision or "
        "arithmetic semantics to obtain a speedup, and do not bypass or weaken the testbench.\n"
        "\n"
        "## Evaluation\n"
        "Each rendered point is compiled and run against the unchanged testbench with a "
        "timeout, then synthesized with Vitis HLS. A point that fails to render, compile, "
        "pass the testbench, or synthesize is scored as a failure. Validate what you can "
        "yourself with `clang++ -fsyntax-only` and by compiling/running the testbench before "
        "finishing; Vitis HLS synthesis itself is not available to you here and is checked "
        "automatically after you submit.\n"
        "\n"
        "Start immediately. Do not only describe the steps. Do NOT terminate until you have "
        f"written exactly one of the two design-space files, between 2 and {max_design_points} "
        "points, and self-validated as many points as you can.\n"
    )


def build_retry_prompt(base_prompt: str, check_data: dict) -> str:
    """The base prompt plus what the harness found wrong with the previous attempt."""
    static = check_data.get("static", {})
    container = check_data.get("container", {})
    lines = [
        "",
        "## Previous attempt failed",
        f"The harness checked your previous output in './{INPUT_DIR_NAME}' and found these "
        "problems. Fix them in place. Do not start over from the unparameterized kernel unless "
        "a failure below says your design-space file itself could not be read.",
        "",
    ]
    for f in static.get("failures", []):
        lines.append(f"- {f}")
    for point_result in container.get("points", []):
        if point_result.get("passed"):
            continue
        idx = point_result.get("point_index")
        params = point_result.get("parameters")
        lines.append(f"- point {idx} ({params}) failed:")
        for key in ("testbench_build", "testbench_run"):
            v = point_result.get(key) or {}
            if v.get("exit_code") not in (0, None):
                tail = "\n".join((v.get("output") or "").splitlines()[-30:])
                lines += [f"  {key.replace('_', ' ')}:", "  ```", tail, "  ```"]
        for name, v in (point_result.get("syntax_check") or {}).items():
            if v.get("exit_code") not in (0, None):
                tail = "\n".join((v.get("output") or "").splitlines()[-30:])
                lines += [f"  syntax check failed for {name}:", "  ```", tail, "  ```"]
    return base_prompt + "\n".join(lines) + "\n"


class HLSParameterizationRun:
    """Parameterize one already-extracted design directory using the Pi agent.

    Mirrors ``HLSTranslationRun``: same Docker image, same Pi settings, same
    session capture, one container held open across a bounded retry loop.
    Differs in what gets validated (a submitted design space, explicit or
    implicit, rather than a translated dialect) and in running synthesis per
    rendered point as a separate pass once validation passes, rather than as
    part of the in-container checks.
    """

    def __init__(
        self,
        run_id: str,
        dir_design: Path,
        dir_work: Path,
        model_name: str,
        api_key: str,
        target: TargetSpec = VITIS,
        docker_image_name: str = DOCKER_IMAGE_NAME,
        agent_timeout_s: int = 60 * 30,
        attempts: int = 1,
        max_design_points: int = 8,
    ) -> None:
        self.run_id = run_id
        self.dir_design = Path(dir_design).resolve()
        self.dir_work = Path(dir_work)
        self.model_name = model_name
        self.api_key = api_key
        self.target = target
        self.docker_image_name = docker_image_name
        self.agent_timeout_s = agent_timeout_s
        self.attempts = max(1, int(attempts))
        self.max_design_points = max_design_points

    # ---- workspace -----------------------------------------------------------------

    def prepare_run_area(self) -> Path:
        if not self.dir_design.is_dir():
            raise FileNotFoundError(f"Design directory not found: {self.dir_design}")
        if self.dir_work.exists():
            shutil.rmtree(self.dir_work)
        self.dir_work.mkdir(parents=True, exist_ok=True)

        dir_run_area = self.dir_work / "run_area"
        dir_run_area.mkdir(parents=True, exist_ok=True)

        dir_pi = dir_run_area / ".pi"
        dir_pi.mkdir(parents=True, exist_ok=True)
        (dir_pi / "settings.json").write_text(
            json.dumps(
                {
                    "defaultProvider": "openrouter",
                    "defaultModel": self.model_name,
                    "sessionDir": ".pi/sessions",
                },
                indent=4,
            ),
            encoding="utf-8",
        )
        dir_sessions = dir_pi / "sessions"
        dir_sessions.mkdir(parents=True, exist_ok=True)
        os.chmod(dir_pi, 0o777)
        os.chmod(dir_sessions, 0o777)

        shutil.copytree(self.dir_design, dir_run_area / INPUT_DIR_NAME)
        shutil.copytree(self.dir_design, dir_run_area / ORIGINAL_DIR_NAME)
        shutil.copytree(DIR_VITIS_HLS_INCLUDE, dir_run_area / "vitis_hls_include")
        os.chmod(dir_run_area / INPUT_DIR_NAME, 0o777)
        return dir_run_area

    def _feasible(self, dir_original: Path) -> list[str]:
        """Cheap pre-check before spending any LLM cost: is there anything
        here worth parameterizing at all. Not a deep scan."""
        reasons = []
        if not (dir_original / "synth.tcl").exists():
            reasons.append("no synth.tcl in the extracted design")
        if not (dir_original / TESTBENCH_FILENAME).exists():
            reasons.append(f"no {TESTBENCH_FILENAME} in the extracted design")
        if find_top_from_synth_tcl(dir_original) is None:
            reasons.append("could not find set_top in synth.tcl")
        if not find_sources_from_synth_tcl(dir_original):
            reasons.append("no kernel source files found")
        return reasons

    # ---- validation ------------------------------------------------------------------

    def _read_submission(self, dir_run_area: Path) -> dict:
        """Static (pure Python, no container) check of what the agent wrote."""
        dir_input = dir_run_area / INPUT_DIR_NAME
        dir_original = dir_run_area / ORIGINAL_DIR_NAME
        result: dict[str, Any] = {"passed": False, "failures": [], "points": []}

        kernel_files = find_sources_from_synth_tcl(dir_original)

        # Protected-files check: nothing outside kernel_files + the two
        # design-space filenames may differ from the pristine original.
        modified: list[str] = []
        for original_file in sorted(dir_original.rglob("*")):
            if not original_file.is_file():
                continue
            rel = original_file.relative_to(dir_original).as_posix()
            if rel in kernel_files:
                continue
            candidate = dir_input / rel
            if not candidate.is_file() or candidate.read_bytes() != original_file.read_bytes():
                modified.append(rel)
        if modified:
            result["failures"].append(f"protected files were modified or removed: {modified}")
            return result

        explicit_fp = dir_input / DESIGN_SPACE_EXPLICIT_FILENAME
        implicit_fp = dir_input / DESIGN_SPACE_IMPLICIT_FILENAME
        if explicit_fp.exists() and implicit_fp.exists():
            result["failures"].append(
                f"both {DESIGN_SPACE_EXPLICIT_FILENAME} and {DESIGN_SPACE_IMPLICIT_FILENAME} "
                "were written; submit exactly one"
            )
            return result

        try:
            if explicit_fp.exists():
                space = DesignSpaceExplicit()
                lines = [
                    ln for ln in explicit_fp.read_text(encoding="utf-8").splitlines() if ln.strip()
                ]
                if not 2 <= len(lines) <= self.max_design_points:
                    raise ValueError(
                        f"expected 2..{self.max_design_points} design-space points, got {len(lines)}"
                    )
                space.parse_from_jsonl("\n".join(lines))
            elif implicit_fp.exists():
                implicit = DesignSpaceImplicit()
                implicit.parse_from_json(implicit_fp.read_text(encoding="utf-8"))
                space = convert_design_space_implicit_to_explicit(
                    implicit, max_points=self.max_design_points
                )
                if len(space.design_space) < 2:
                    raise ValueError(
                        f"implicit space expanded to {len(space.design_space)} point(s) after "
                        "constraints, need at least 2"
                    )
            else:
                result["failures"].append(
                    f"neither {DESIGN_SPACE_EXPLICIT_FILENAME} nor {DESIGN_SPACE_IMPLICIT_FILENAME} "
                    "was found"
                )
                return result
        except (OSError, ValueError, TypeError, TemplateError) as error:
            result["failures"].append(f"could not read design space: {error}")
            return result

        if any(
            isinstance(v, float) and not math.isfinite(v)
            for point in space.design_space
            for v in point.values()
        ):
            result["failures"].append("design-space numbers must be finite")
            return result
        canonical = [json.dumps(point, sort_keys=True) for point in space.design_space]
        if len(set(canonical)) != len(canonical):
            result["failures"].append("design-space assignments must be distinct")
            return result

        sources: T_sources = {
            f: (dir_input / f).read_text(encoding="utf-8", errors="replace") for f in kernel_files
        }
        used_jinja = _uses_jinja(sources)
        flow = (
            JinjaParamaterizationFlow(strict_undefined=True, sandboxed=True)
            if used_jinja
            else PreprocessorParamaterizationFlow()
        )
        try:
            rendered = flow.preprocess(sources, space)
        except Exception as error:  # noqa: BLE001 -- any rendering failure becomes a clean submission failure
            result["failures"].append(f"rendering failed: {error}")
            return result

        for i, design in enumerate(rendered):
            # The leftover-marker check only makes sense for Jinja output: a
            # macro-rendered file can legitimately contain a literal `{{` in
            # an ordinary nested-brace C++ initializer.
            leftover = used_jinja and any(
                marker in content for content in design.values() for marker in ("{{", "{%", "{#")
            )
            result["points"].append(
                {
                    "point_index": i,
                    "parameters": space.design_space[i],
                    "rendered": design,
                    "render_ok": not leftover,
                }
            )
            if leftover:
                result["failures"].append(f"point {i} still contains unexpanded Jinja syntax")

        result["kernel_files"] = kernel_files
        result["passed"] = not result["failures"]
        return result

    def _check_points_in_container(
        self,
        container: Container,
        dir_run_area: Path,
        kernel_files: list[str],
        points: list[dict],
    ) -> dict:
        """Compile and run the testbench for every rendered point, inside the
        already-open agent container. Modeled on translate.py's
        run_container_checks / _container_exec."""
        inc = f"-I{CONTAINER_RUN_AREA}/vitis_hls_include"
        results: dict[str, Any] = {"points": []}
        all_ok = True
        for point in points:
            idx = point["point_index"]
            if not point["render_ok"]:
                results["points"].append({**point, "passed": False})
                all_ok = False
                continue

            point_dir_name = f"{POINTS_DIR_NAME}/point_{idx}"
            point_dir = dir_run_area / point_dir_name
            shutil.copytree(dir_run_area / ORIGINAL_DIR_NAME, point_dir)
            for fname, content in point["rendered"].items():
                (point_dir / fname).write_text(content, encoding="utf-8")
            os.chmod(point_dir, 0o777)
            c_out = f"{CONTAINER_RUN_AREA}/{point_dir_name}"

            syntax: dict[str, dict] = {}
            for f in kernel_files:
                code, output = _container_exec(
                    container,
                    f"clang++ -std=c++17 {inc} -I{c_out} -w -fsyntax-only {shlex.quote(f)}",
                    c_out,
                )
                syntax[f] = {"exit_code": code, "output": output[-4000:]}
            syntax_ok = bool(syntax) and all(v["exit_code"] == 0 for v in syntax.values())

            testbench_build = {"exit_code": None, "output": "not run: syntax check failed"}
            testbench_run = {"exit_code": None, "output": "not run: syntax check failed"}
            build_ok = False
            if syntax_ok:
                all_cpp = [
                    p.name for p in sorted(point_dir.iterdir()) if p.suffix.lower() in SOURCE_SUFFIXES
                ]
                build_cmd = (
                    f"clang++ -std=c++17 {inc} -I{c_out} -w "
                    + " ".join(shlex.quote(f) for f in all_cpp)
                    + f" -o /tmp/point_{idx}.out"
                )
                code, output = _container_exec(container, build_cmd, c_out)
                testbench_build = {"exit_code": code, "output": output[-4000:]}
                if code == 0:
                    code, output = _container_exec(container, f"timeout 60s /tmp/point_{idx}.out", c_out)
                    testbench_run = {"exit_code": code, "output": output[-4000:]}
                    build_ok = code == 0
                else:
                    testbench_run = {"exit_code": None, "output": "not run: build failed"}

            passed = syntax_ok and build_ok
            all_ok = all_ok and passed
            results["points"].append(
                {
                    **point,
                    "syntax_check": syntax,
                    "syntax_all_ok": syntax_ok,
                    "testbench_build": testbench_build,
                    "testbench_run": testbench_run,
                    "passed": passed,
                }
            )
        results["all_points_ok"] = all_ok
        return results

    # ---- main ------------------------------------------------------------------------

    def run(self) -> dict:
        design_name = self.dir_design.name
        print(f"Parameterizing design `{design_name}` (run `{self.run_id}`)")

        run_data: dict[str, Any] = {
            "run_id": self.run_id,
            "design_name": design_name,
            "design_dir": str(self.dir_design),
            "model_name": self.model_name,
            "docker_image_name": self.docker_image_name,
            "sessions": [],
        }

        dir_run_area = self.prepare_run_area()
        dir_original = dir_run_area / ORIGINAL_DIR_NAME

        blockers = self._feasible(dir_original)
        if blockers:
            check_data = {"passed": False, "failures": blockers, "points": [], "attempts": []}
            run_data["infeasible"] = blockers
            (self.dir_work / "run_data.json").write_text(json.dumps(run_data, indent=4), encoding="utf-8")
            (self.dir_work / "check_data.json").write_text(
                json.dumps(check_data, indent=4), encoding="utf-8"
            )
            print(f"Parameterization `{self.run_id}`: infeasible: {blockers[0]}")
            return check_data

        kernel_files = find_sources_from_synth_tcl(dir_original)
        prompt = build_parameterization_prompt(design_name, kernel_files, self.max_design_points)
        run_data["prompt_task"] = prompt

        client = docker.from_env()
        container: Container = client.containers.run(
            image=self.docker_image_name,
            command="sleep 2h",
            detach=True,
            volumes={str(dir_run_area.resolve()): {"bind": CONTAINER_RUN_AREA, "mode": "rw"}},
        )

        check_data: dict[str, Any] = {}
        attempts_log: list[dict] = []
        try:
            prompt_this = prompt
            for attempt in range(1, self.attempts + 1):
                t0 = time.monotonic()
                self._run_agent_once(container, prompt_this, dir_run_area, run_data, attempt)
                t1 = time.monotonic()
                static = self._read_submission(dir_run_area)
                container_check = (
                    self._check_points_in_container(container, dir_run_area, kernel_files, static["points"])
                    if static["passed"]
                    else {"points": [], "all_points_ok": False}
                )
                passed = bool(static["passed"] and container_check["all_points_ok"])
                check_data = {"static": static, "container": container_check, "passed": passed}
                attempts_log.append(
                    {
                        "attempt": attempt,
                        "passed": passed,
                        "failures": list(static.get("failures", [])),
                        "agent_s": round(t1 - t0),
                        "check_s": round(time.monotonic() - t1),
                    }
                )
                if passed:
                    break
                prompt_this = build_retry_prompt(prompt, check_data)
            check_data["attempts"] = attempts_log
            check_data["attempts_used"] = len(attempts_log)
        finally:
            container.stop()
            container.remove(force=True)

        if check_data.get("passed"):
            dir_cache = self.dir_work / "synth_cache"
            synth_results: list[dict] = []
            for point in check_data["container"]["points"]:
                idx = point["point_index"]
                point_dir = dir_run_area / POINTS_DIR_NAME / f"point_{idx}"
                dir_point_work = self.dir_work / "synth" / f"point_{idx}"
                result: SynthResult = run_synth_cached(self.target, point_dir, dir_point_work, dir_cache)
                synth_results.append(
                    {"point_index": idx, "parameters": point["parameters"], **result.__dict__}
                )
            check_data["synth_results"] = synth_results
            check_data["summary"] = self._summarize_results(synth_results)

        # Drop the full rendered-source text before writing to disk -- useful
        # in memory for the container checks, wasteful and unreadable in the
        # JSON report.
        static_for_report = dict(check_data.get("static", {}))
        static_for_report["points"] = [
            {k: v for k, v in point.items() if k != "rendered"} for point in static_for_report.get("points", [])
        ]
        check_data_for_report = {**check_data, "static": static_for_report}

        (self.dir_work / "run_data.json").write_text(json.dumps(run_data, indent=4), encoding="utf-8")
        (self.dir_work / "check_data.json").write_text(
            json.dumps(check_data_for_report, indent=4, default=str), encoding="utf-8"
        )
        print(f"Parameterization `{self.run_id}`: passed={check_data.get('passed')}")
        return check_data

    def _run_agent_once(
        self,
        container: Container,
        prompt_text: str,
        dir_run_area: Path,
        run_data: dict,
        attempt: int,
    ) -> None:
        """Run `pi -p <prompt>` once and capture the session it produced.

        Near-identical to HLSTranslationRun._run_agent_once; a candidate for
        extracting into a shared helper both classes call, rather than a
        third copy of the same logic -- flagged for the conversion team
        rather than unified unilaterally here, since it'd touch their file.
        """
        cmd = f"umask 000 && timeout {self.agent_timeout_s}s pi -p {shlex.quote(prompt_text)}"
        exit_code, output_agent = container.exec_run(
            ["sh", "-lc", cmd],
            environment={"OPENROUTER_API_KEY": self.api_key},
            workdir=CONTAINER_RUN_AREA,
        )
        tail = (
            output_agent.decode("utf-8", errors="replace")[-8000:]
            if isinstance(output_agent, bytes)
            else str(output_agent)[-8000:]
        )
        run_data["agent_exit_code"] = exit_code
        run_data["agent_stdout_tail"] = tail

        dir_sessions = dir_run_area / ".pi" / "sessions"
        seen = {s["file"] for s in run_data["sessions"] if s.get("file")}
        candidates = (
            sorted(
                (p for p in dir_sessions.glob("*.jsonl") if p.name not in seen),
                key=lambda p: p.stat().st_mtime,
            )
            if dir_sessions.exists()
            else []
        )
        session_file = candidates[-1] if candidates else None
        if session_file is None:
            run_data["session_data"] = None
            run_data["sessions"].append({"attempt": attempt, "file": None, "exit_code": exit_code, "events": 0})
            return
        events = load_jsonl_text(session_file.read_text(encoding="utf-8", errors="replace"))
        run_data["session_data"] = events
        run_data["sessions"].append(
            {"attempt": attempt, "file": session_file.name, "exit_code": exit_code, "events": len(events)}
        )
        container.exec_run(
            [
                "sh",
                "-lc",
                f"umask 000 && pi --export {CONTAINER_RUN_AREA}/.pi/sessions/{session_file.name} "
                f"{CONTAINER_RUN_AREA}/.pi/sessions/{session_file.name.replace('.jsonl', '.html')}",
            ],
            workdir=CONTAINER_RUN_AREA,
        )

    def _summarize_results(self, synth_results: list[dict]) -> dict:
        """Pareto-optimal frontier across the synthesized points, adapted
        from hls-eval's summarize_design_space / _objective_values /
        _dominates, but against SynthResult's flatter resources dict
        (BRAM/DSP/FF/LUT from targets/vitis.py's parse_vitis_report) instead
        of hls-eval's six separately-named objective fields.

        Simplification versus hls-eval's version: there is no separate
        "baseline" (unparameterized) synthesis run here to compare the
        frontier against yet.
        """
        objectives = ("latency", "LUT", "FF", "DSP", "BRAM")

        def objective_values(point: dict) -> tuple[float, ...] | None:
            if point.get("status") != "PASS":
                return None
            resources = point.get("resources") or {}
            values = [point.get("latency")] + [resources.get(k) for k in ("LUT", "FF", "DSP", "BRAM")]
            if not all(
                isinstance(v, (int, float)) and not isinstance(v, bool) and math.isfinite(v) and v >= 0
                for v in values
            ):
                return None
            return tuple(values)

        def dominates(left: tuple, right: tuple) -> bool:
            return all(a <= b for a, b in zip(left, right, strict=True)) and any(
                a < b for a, b in zip(left, right, strict=True)
            )

        eligible = [
            (point["point_index"], values)
            for point in synth_results
            if (values := objective_values(point)) is not None
        ]
        frontier = [
            idx for idx, values in eligible if not any(dominates(other, values) for _, other in eligible)
        ]
        return {
            "objectives": list(objectives),
            "objective_direction": "minimize",
            "n_points": len(synth_results),
            "n_passed": sum(1 for p in synth_results if p.get("status") == "PASS"),
            "n_metric_eligible": len(eligible),
            "pareto_point_indices": frontier,
        }
