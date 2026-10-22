"""Agentic design-space parameterization: run the Pi agent to turn one
already-extracted HLS design into many validated, synthesized variants.

Mirrors translate.py's HLSTranslationRun: one Docker container held
open across a bounded retry loop, pure-Python (static) validation followed
by real compile/testbench checks run inside that same container, then
synthesis as a separate pass over whichever points pass validation.

The design-space / rendering engine itself (explicit and implicit spaces,
Jinja and preprocessor-macro rendering) lives in ``parameterization.py``,
ported from hls-eval -- this module is the orchestrator that calls into it,
not a second copy of it.

Method bodies raise NotImplementedError; see each
docstring for what still needs to be written.
"""

from __future__ import annotations

from pathlib import Path

from docker.models.containers import Container

from hlsfactory_agent.parameterization import DesignSpaceExplicit
from hlsfactory_agent.spec import TargetSpec
from hlsfactory_agent.targets.vitis import VITIS

DOCKER_IMAGE_NAME = "hlsfactory-agent"
CONTAINER_RUN_AREA = "/workspace/run_area"
INPUT_DIR_NAME = "input"

# The agent signals which kind of design space it submitted by which of
# these two filenames it writes -- decided over the format-sniffing
# alternative so a malformed submission gets one clean, correctly-attributed
# error instead of two unrelated ones (see the discussion this was settled
# in: ambiguous failure feedback is worse here than in most codebases,
# because the agent is the one that has to read and act on it).
DESIGN_SPACE_EXPLICIT_FILENAME = "design_space.jsonl"
DESIGN_SPACE_IMPLICIT_FILENAME = "design_space_implicit.json"


def build_parameterization_prompt(
    design_name: str,
    max_design_points: int,
    *,
    retry_section: str | None = None,
) -> str:
    """Build the task prompt handed to the Pi agent for one parameterization run.

    TODO: adapt hls-eval's prompt sections (intro, I/O rules, harness
    availability, optimization guidance, evaluation description) to this
    repo's conventions, and extend the I/O section to explain the
    explicit-vs-implicit submission choice: write DESIGN_SPACE_EXPLICIT_FILENAME
    for a hand-listed set of points, or DESIGN_SPACE_IMPLICIT_FILENAME for a
    parameter-axes-plus-constraints spec, not both. State max_design_points
    explicitly here (this is where the earlier "do we need a size guard on
    the implicit space" question gets resolved -- by bounding it in the
    prompt, not with extra code).

    `retry_section`, when given, replaces the intro section the same way
    translate.py's build_retry_prompt does for a repeat attempt.
    """
    raise NotImplementedError


def build_retry_prompt(base_prompt: str, check_data: dict) -> str:
    """Build a follow-up prompt describing exactly what failed on the last attempt.

    TODO: mirror translate.py's build_retry_prompt -- summarize which design-
    space file was submitted, any parse/validation error (including a
    jinja2.exceptions.TemplateError from a bad implicit constraint), and
    which specific points failed which check (render / clang syntax / csim),
    so the agent can fix just that rather than starting over.
    """
    raise NotImplementedError


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
        """Set up the host-side directory that gets bind-mounted into the container.

        TODO: mirror HLSTranslationRun.prepare_run_area -- fresh dir_work,
        .pi/settings.json + sessions dir, copy self.dir_design in under
        INPUT_DIR_NAME, copy in vitis_hls_include. Optionally a light
        feasibility pre-check (source files + testbench + synth.tcl actually
        present) before spending any LLM cost, mirroring translate.py's
        feasibility() early-exit -- not a deep scan, just "is there anything
        here worth parameterizing."
        """
        raise NotImplementedError

    # ---- validation ------------------------------------------------------------------

    def _read_submission(self, dir_run_area: Path) -> tuple[dict, DesignSpaceExplicit]:
        """Static (pure Python, no container) check of what the agent wrote.

        TODO:
        - Detect which of DESIGN_SPACE_EXPLICIT_FILENAME /
          DESIGN_SPACE_IMPLICIT_FILENAME is present.
        - Parse accordingly (DesignSpaceExplicit.parse_from_jsonl or
          DesignSpaceImplicit.parse_from_json); if implicit, call
          convert_design_space_implicit_to_explicit eagerly right here, so
          every caller downstream always holds a plain DesignSpaceExplicit.
          Catch jinja2.exceptions.TemplateError (covers both
          TemplateSyntaxError and UndefinedError) alongside
          (OSError, ValueError, TypeError) -- confirmed earlier that neither
          is a subclass of the other, so both need handling explicitly.
        - Validate point count in [2, self.max_design_points], finite values,
          distinct points -- reuse the same checks hls-eval's
          _read_submission used for the explicit case.
        - Confirm no protected files (headers, testbench, original kernel
          files) were modified, same spirit as this repo's own
          check_translated_design / the modified-files check hls-eval used.
        - Render every point: PreprocessorParamaterizationFlow if the design
          has no *.jinja files, JinjaParamaterizationFlow if it does -- same
          file-extension-based detection used in HLSFactory's
          CombinedFrontend. Reject any point whose rendered output still
          contains a literal {{ / {% / {#.
        Returns (checks_dict, expanded_design_space).
        """
        raise NotImplementedError

    def _check_points_in_container(
        self, container: Container, dir_run_area: Path, points: list[dict]
    ) -> dict:
        """Compile and run the testbench for every rendered point.

        TODO: modeled on translate.py's run_container_checks /
        _container_exec -- this is the missing csim piece. Runs inside the
        same already-open container as the agent (container.exec_run), not a
        second container spun up per point: a clang syntax/interface check
        plus compiling and running the testbench for each point, writing
        results per point the same way run_container_checks does per file.
        """
        raise NotImplementedError

    # ---- main ------------------------------------------------------------------------

    def run(self) -> dict:
        """Run the full parameterization pipeline for this design.

        TODO: mirror HLSTranslationRun.run() --
        1. Build run_data, call prepare_run_area().
        2. Build the prompt via build_parameterization_prompt.
        3. Start one container (held open across attempts).
        4. Bounded retry loop (self.attempts): _run_agent_once ->
           _read_submission -> _check_points_in_container -> if not passed,
           build_retry_prompt(...) and loop.
        5. Stop/remove the container.
        6. Once validation passes: run_synth_cached(self.target, ...) per
           validated point, as a separate pass (reuses synth.py as-is, no
           new synthesis code needed).
        7. _summarize_results(...) over the synthesized points.
        8. Write run_data.json / check_data.json to self.dir_work, return
           the final check_data dict.
        """
        raise NotImplementedError

    def _run_agent_once(
        self,
        container: Container,
        prompt_text: str,
        dir_run_area: Path,
        run_data: dict,
        attempt: int,
    ) -> None:
        """Run `pi -p <prompt>` once and capture the session it produced.

        TODO: near-identical to HLSTranslationRun._run_agent_once (timeout-
        wrapped exec_run, find the newest unseen session file, load it,
        export to HTML). Candidate for extracting into a shared helper both
        classes call instead of a third copy of the same logic -- worth
        raising with the conversion team rather than deciding unilaterally,
        since it'd touch their file too.
        """
        raise NotImplementedError

    def _summarize_results(self, points: list[dict]) -> dict:
        """Compute the Pareto-optimal frontier across the synthesized points.

        TODO: adapted from hls-eval's summarize_design_space /
        _objective_values / _dominates, but against SynthResult's flatter
        `resources: dict[str, int | None]` shape (BRAM/DSP/FF/LUT from
        targets/vitis.py's parse_vitis_report) instead of hls-eval's six
        separately-named PARETO_OBJECTIVES fields.
        """
        raise NotImplementedError
