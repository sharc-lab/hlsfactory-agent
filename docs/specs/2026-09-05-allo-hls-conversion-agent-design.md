# Allo-based HLS conversion agent: design

Date: 2026-09-05
Status: draft for review. Revision note 2026-09-10: after source-level review of Allo's Catapult and XLS emitters
(section 3.4), the hub decision changed from Allo to a thin portable HLS-C layer owned by the lab, with Allo kept
as an optional template-authoring frontend. Sections 4, 5.2, 5.3, 5.4, 9, and 12 still describe the Allo hub and
are to be rewritten. The first implemented flow is a direct Vitis to Catapult translation on the existing agent
harness, in `hlsfactory_agent/translate.py` with the run script in `exp/run_translate/`; it is the first target row
of the projection table the portable layer will generalize.
Scope: the standalone conversion phase only. HLSFactory integration, the template library, SyntheticHLS as an input source, and the cross-tool QoR study are later phases and are named here only where a seam has to be left for them.

## 1. Goal

Build an agent that takes a standalone Vitis HLS design, as produced by HLSFactory-Agent, and turns it into a design that exists on several HLS tools at once, with evidence that each version still computes the same thing. The first three tools are Vitis HLS, Google XLS through xlscc, and Siemens Catapult. The agent does this by lifting the C++ design into Allo, a Python-embedded HLS frontend from Cornell that already has backends for all three tools, and then checking every emitted version against the original testbench and against each other.

The research questions this phase answers:

1. What fraction of real extracted HLS designs can an agent lift into a portable form, and which constructs block the rest.
2. For lifted designs, how many survive emission and synthesis on each target.
3. How much each design costs to convert, in model spend and wall time.

## 2. Non-goals for this phase

- Collecting latency, area, or frequency numbers. That is rung 4 of the ladder in section 6 and belongs to HLSFactory.
- Design-space exploration, Pareto frontiers, or the template library.
- Intel HLS, oneAPI, Dynamatic, or Bambu targets.
- Changing HLSFactory-Agent. It is used read-only as the source of designs.
- A hand-written neutral C++ dialect. It is held in reserve only if the lift rate is poor. See section 12.

## 3. Background and what already exists

### 3.1 HLSFactory-Agent, this repository

An LLM agent on the Pi framework inside a Docker container extracts standalone designs from repositories. Its output is one folder per design containing sources, headers, a `testbench.cpp`, data files, a `synth.tcl` with a `set_top` line, a README, and a compile log. Validity today means only that Vitis csynth exits zero; whether the testbench passes is never checked. The last run produced 271 candidate designs across 26 repositories, 130 passing csynth. Orchestration lives in `hlsfactory_agent/core.py` and `hlsfactory_agent/docker.py`; the Vitis wrapper is `exp/run_base/tools.py`; the checker is `exp/run_base/check_output_designs.py`.

### 3.2 HLSFactory

The lab's dataset framework. It has synthesis flow classes for Vitis, Intel HLS, Catapult, and XLS that emit a standard QoR JSON, a design-source directory layout, and the OptDSL design-space language. This phase does not call it, but the results layout in section 5.6 is chosen so a later phase can write into HLSFactory's design-source layout without restructuring.

### 3.3 SyntheticHLS

The lab's LLM mutation pipeline. Relevant here: its validity gate is Vitis csim plus csynth, it samples several candidates per step and selects the best passing one, it repairs only when every candidate fails, and it reports pass@1 and pass@8. Those four choices are adopted in section 5.2.

### 3.4 Allo, verified 2026-09-05

Documentation: https://cornell-zhang.github.io/allo/ and source under `allo/backend/`.

- Backends: LLVM CPU, Vitis HLS (`target="vhls"`, modes csim and csyn, generates a project with run.tcl), Catapult (`target="catapult"`, emits `ac_int`, `ac_fixed`, `ac_channel`, a `run.tcl` and Makefile, modes csim and csyn), Google XLS (`target="xlscc"`, emits xlscc C++, `use_memory` flag, `mode="sw_emu"`), RapidStream TAPA, a multi-threaded simulator, and AMD AI Engine.
- Schedule primitives translate to backend pragmas with correct placement. The Catapult page documents pipeline and unroll; coverage of partition, reorder, buffer_at, reuse_at, and dataflow on Catapult and XLS is undocumented and is measured in section 5.3.
- Types: integers of any width, unsigned, fixed point, float32, index. The XLS backend rejects floats and supports only single-port and simple dual-port memories.
- Frontend rejects break, continue, pointers, and structs. While loops, nested functions, template kernels, and a dataflow module with streams exist.
- `allo.verify(s1, s2)` checks two schedules of the same kernel against each other. It is not used here because the check needed is original C++ against lifted code.
- Allo is heavy to build because it pins its own LLVM. It ships a Docker folder. This design treats Allo as a black box, Python in and C++ out, and never composes with its MLIR dialects.

Backend maturity, verified from source on 2026-09-10. The real converters are C++ MLIR translation passes under `mlir/lib/Translation/`; the Python files in `allo/backend/` are wrappers that generate harnesses and scripts. All are deterministic.

| Backend | Emitter | Size | Age | Primitives emitted | Not emitted | Tests |
|---|---|---|---|---|---|---|
| Vitis | `EmitVivadoHLS.cpp` | ~2,100 lines | since 2023, PLDI 2024 evaluation | pipeline with II and rewind, unroll with factor, dataflow, inline, stream depth, array_partition block, cyclic and complete, bind_storage | | mature suites |
| Catapult | `EmitCatapultHLS.cpp`, inherits the Vitis emitter | ~480 lines | added 2026-02-05, one commit | pipeline_init_interval, unroll with factor, hls_design top, dataflow, inline | any memory or partition directive, marked TODO in source | 16 tests on tiny kernels; 12 check strings only, csim and csyn tests skip without `MGC_HOME` |
| XLS | `EmitXlsHLS.cpp` | ~1,150 lines | added 2026-01-05, three commits | pipeline_init_interval fixed at 1, unroll yes with optional factor, hls_top | partition, dataflow, any II other than 1; loops default to full unroll unless `use_memory` | ~20 tests on scalar ops, vvadd and a 2 by 2 GEMM; pragma string checks plus g++ emulation; never invokes xlscc, opt_main or codegen_main |
| Intel | `EmitIntelHLS.cpp` | ~850 lines | oneAPI 2024.2 style, `[[intel::initiation_interval]]`, `#pragma unroll` | | "can only support one function now" in source; reachable as `target="ihls"`, which writes `kernel.cpp` and a run.tcl but has no execution path in `HLSModule.__call__` | none found |

Two consequences for this design:

1. Allo's fixed-point type carries width and fraction only. There is no rounding or overflow mode. The Vitis emitter writes `ap_fixed<W,I>` and the Catapult emitter writes `ac_fixed<W,I,S>`, both with tool defaults, which are truncate and wrap on both tools. An extracted design that uses any other mode, such as `AP_RND` or `AP_SAT`, cannot be lifted faithfully and fails rule 4 in section 5.2 with reason `numeric_unsupported`. The bundle stage records whether a design's sources mention non-default modes so the prevalence is known before lifting starts.
2. Allo's own tests never run the emitted xlscc code through the XLS tools. Rung 3 of this design is therefore the first real exercise of that emitter on non-trivial kernels. Expect to find emitter bugs and to report them upstream; the failure label `emit_failed` with backend `xlscc` and the coverage table are where they show up.

### 3.5 Principles taken from the reading list

- Do not build an intermediate representation that composes with other projects' MLIR dialects; version drift breaks it. Source: the Dynamatic MLIR experience paper.
- Use deterministic tools for everything mechanical and the LLM only for the semantic rewrite; keep a memory of lessons across tasks. Source: AgRefactor.
- Put verification inside the agent's loop, not after it. Source: the NSF AI-for-EDA report and SyntheticHLS.
- Report designs per dollar and per minute, not only totals. Source: the HLSFactory-Agent abstract.

## 4. Architecture

Six stages. Only the second uses an LLM.

1. Design bundle: an extracted folder plus a manifest derived by script, plus an oracle check that the original testbench actually passes against the original sources.
2. Lift agent: an LLM agent on the Pi harness writes an Allo kernel and schedule with the same interface, iterating against rungs 0 and 1 of the ladder inside its loop, informed by a lessons file.
3. Allo projection: scripts call Allo's Vitis, Catapult, and xlscc backends and write one folder per target, logging any schedule primitive a backend did not honor.
4. Emulation checks: the original testbench runs against the emitted Vitis kernel; a generated differential harness checks the Catapult and xlscc kernels against the Vitis one. All inside the container with open-source headers.
5. Tool runs: xlscc, opt, and codegen inside the container; Vitis csynth on the host; Catapult csyn on the host only if a license exists.
6. Results: one `verify.json` per design, session traces, cost, a failure label, and a run summary.

A rung 1 failure at stage 4 returns to stage 2 with the error report, bounded by the budgets in section 5.2. A rung 2 failure is recorded and does not return to the agent, because rung 2 measures Allo's backends, not the lift, and rule 4 in section 5.2 forbids the agent from changing numerics to satisfy a backend.

## 5. Components

Each component has one job, a defined input and output, and can be tested without the others.

### 5.1 Design bundle and oracle check

Module: `bundle.py`.

Input: a path to an HLSFactory-Agent design folder.

Output: `bundle.json` next to the sources:

```json
{
  "design_id": "<repo>__<design>",
  "origin": {"run_id": "...", "path": "..."},
  "top": "fft",
  "sources": ["fft.cpp"],
  "headers": ["fft.h"],
  "include_dirs": ["."],
  "testbench": "testbench.cpp",
  "data_files": ["input.dat", "golden.dat"],
  "oracle": {"status": "pass", "compile_s": 3.2, "run_s": 0.4}
}
```

Rules:

- `top` is read from the `set_top` line of `synth.tcl`. If there is no such line, the bundle is rejected with reason `no_top`.
- Sources and headers are read from the `add_files` and `add_files -tb` lines, falling back to a directory scan.
- The oracle check compiles the testbench and sources with clang against the vendored headers in `hlsfactory_agent/vitis_hls_include`, runs the binary in the design folder, and records pass if the exit code is zero and the output does not contain a failure marker. The failure markers are the strings `FAIL`, `ERROR`, and `MISMATCH`, case-insensitive, on any output line.
- Designs whose oracle check fails are excluded from lift statistics and reported separately under `no_oracle`. This number is itself a finding about the extraction pipeline.

### 5.2 Lift agent

Modules: `lift/prompt.py` for the lifting guide, `lift/agent.py` for the harness call, `lift/notes.py` for the lessons file.

Harness: the same Pi-in-Docker runner used by HLSFactory-Agent, imported from the `hlsfactory_agent` package. The container image is described in section 9.

Inputs to the agent:

- The design folder with `bundle.json`.
- The lifting guide: Allo syntax rules, the list of rejected constructs and how to restructure each, the schedule primitive mapping table from section 5.3, backend constraints such as no floats on XLS, and three worked examples of a lift.
- The lessons file `lifting_notes.md`, if present.

Outputs written into `<design>/allo/`:

- `kernel.py`: the Allo algorithm with the same top function name, argument names, order, shapes, and widths as the original.
- `schedule.py`: the customize call and every schedule primitive, each with a comment naming the original pragma it replaces.
- `adapter.h`: optional. Only when the emitted Vitis signature cannot be made identical to the original, a header defining a function with the original signature that calls the emitted one.
- `lift_report.md`: what was restructured, what was dropped and why, and any numeric concern.

The agent's loop: write the kernel, run rung 0, run rung 1, read the diff or error, fix, repeat. It stops on rung 1 pass or when a budget is hit. An attempt is one fresh agent session with no memory of earlier attempts on the same design other than the lessons file; the loop above happens inside a single attempt.

Budgets, all configurable, with these defaults:

| Budget | Default |
|---|---|
| Tool calls per attempt | 80 |
| Wall time per attempt | 30 minutes |
| Model spend per attempt | 2 USD |
| Attempts per design (k) | 1 for the pilot, 3 for the final measurement |

Selection with k > 1: the first attempt to pass rung 1 wins. If several pass, the one with the most rung 2 targets passing wins, then the lowest cost. Report pass@1 and pass@k the way SyntheticHLS does.

Lessons file: after any attempt in which the agent fixed a rung 0 or rung 1 failure, the harness asks the agent for one entry of the form `construct, symptom, fix, design_id`, appended to `lifting_notes.md`. The file is injected into the guide for later designs in the same run. A human reviews it between runs. Its effect is measured by running the same design subset with the file enabled and disabled.

Lift rules the guide enforces:

1. Keep the top-level interface exactly.
2. Every pragma becomes a schedule primitive. Nothing is dropped silently. A pragma with no primitive is recorded in `lift_report.md` as dropped.
3. Rejected constructs are restructured, never approximated. Break becomes a guarded loop, pointer arithmetic becomes indexing, structs become separate arrays.
4. Numerics never change. Widths, signedness, and fixed-point rounding and overflow modes must match. If Allo cannot express one, the attempt fails with reason `numeric_unsupported`.

### 5.3 Allo projection

Module: `project.py`.

Input: `<design>/allo/`. Output: `<design>/targets/vhls/`, `<design>/targets/catapult/`, `<design>/targets/xlscc/`, each holding the emitted kernel, any generated driver files, and `emit.json` listing the schedule primitives applied and, per backend, which were not honored.

The audit table: once per Allo version, a script builds a fixture kernel that uses every schedule primitive, emits it on every backend, and records which primitives appear as pragmas or structural changes in the output. The table is stored as `primitive_coverage.json` and the dropped list in each `emit.json` is computed from it. This replaces a designed pragma tier system with a measured one.

Failure at this stage is reason `emit_failed` with the backend name and the Allo error text.

### 5.4 Emulation checks

Modules: `verify/vitis_tb.py`, `verify/differential.py`.

Rung 1, Vitis testbench: compile the original testbench, `adapter.h` if present, and the emitted Vitis kernel with clang against the vendored Vitis headers, run it in the design folder, apply the same pass rule as the oracle check. Timeout 5 minutes.

Rung 2, differential harness: a script generates a C++ harness from the kernel signature in `bundle.json` and the emitted headers. For each of Catapult and xlscc, the harness drives the target kernel and the emitted Vitis kernel with the same inputs and compares outputs bit for bit. Inputs are 32 random vectors from a fixed seed plus edge values per type, meaning zero, minimum, maximum, and alternating bits. Supported argument kinds are scalars, fixed-shape arrays, and streams of integer or fixed-point type. Catapult compiles with the open-source ac_types headers; xlscc compiles with the XLS synth-only headers in emulation mode. A kernel whose types a target cannot represent, for example floats on XLS, is recorded as `skipped` with the reason, not as a failure. Timeout 5 minutes per target.

Rung 2 checks projection fidelity, not the lift. The lift is checked by rung 1 only, because only the original testbench knows what the design is supposed to compute.

### 5.5 Tool runs

Modules: `verify/synth_xls.py`, `verify/synth_vitis.py`, `verify/synth_catapult.py`.

- XLS, inside the container: `xlscc` on the emitted file with the top name, then `opt_main`, then `codegen_main` with the pipeline generator, the unit delay model, and `--pipeline_stages=1`. These are the least constraining settings that still exercise scheduling and codegen, which is all rung 3 needs; a real delay model and clock period belong to rung 4. The values live in `config.py` and are recorded in `summary.json`. Success means codegen writes a Verilog file. Timeout 10 minutes.
- Vitis, on the host: run the `run.tcl` that Allo's Vitis backend generates, through the existing wrapper pattern in `exp/run_base/tools.py`. Success means csynth exits zero. Timeout 6 minutes, matching the current checker.
- Catapult, on the host, only when the `MGC_HOME` environment variable is set: run the generated `run.tcl`. Otherwise recorded as `skipped` with reason `no_license`.

Rung 3 runs as a batch after the agent run, not inside the agent loop.

### 5.6 Results

Module: `report.py`.

Per design, `verify.json`:

```json
{
  "design_id": "HP-FFT-HLS__fft_16",
  "attempts": 1,
  "oracle": {"status": "pass"},
  "lift": {"status": "pass", "tool_calls": 41, "cost_usd": 0.62, "wall_s": 540, "notes_enabled": true},
  "allo": {"frontend": "pass", "primitives": ["pipeline", "unroll", "partition"]},
  "targets": {
    "vhls":     {"emit": "pass", "testbench": "pass", "synth": "pass", "dropped": []},
    "catapult": {"emit": "pass", "differential": "pass", "synth": "skipped:no_license", "dropped": ["partition"]},
    "xlscc":    {"emit": "pass", "differential": "pass", "synth": "pass", "dropped": ["partition"]}
  },
  "failure_label": null,
  "trace": "session.jsonl"
}
```

Failure labels, assigned by a script from the stage and error text, with the LLM asked to choose among the labels only when the script cannot:

`no_top`, `no_oracle`, `unsupported_construct`, `interface_mismatch`, `numeric_unsupported`, `primitive_unsupported`, `allo_frontend_error`, `emit_failed`, `testbench_mismatch`, `differential_mismatch`, `tool_crash`, `timeout`, `budget_exhausted`.

Run summary, `summary.json` and a Markdown table: counts per label, lift rate, per-target emulation and synthesis pass rates, pass@1 and pass@k, mean and median cost and wall time, designs per dollar, designs per minute, and the primitive coverage table.

Results directory layout, chosen so a later phase can copy `targets/<tool>/` into HLSFactory's design-source layout unchanged:

```
runs/<run_id>/
  lifting_notes.md
  summary.json
  summary.md
  designs/<design_id>/
    bundle.json
    allo/{kernel.py, schedule.py, adapter.h, lift_report.md}
    targets/{vhls,catapult,xlscc}/{emitted files, emit.json, logs}
    verify.json
    session.jsonl
    session.html
```

## 6. Verification ladder

| Rung | Check | Where | Inside agent loop |
|---|---|---|---|
| Oracle | Original testbench passes against original sources | container, clang | no, pre-filter |
| 0 | Allo frontend accepts kernel and schedule, Vitis emit succeeds | container | yes |
| 1 | Original testbench passes against emitted Vitis kernel | container, clang | yes |
| 2 | Catapult and xlscc kernels match the Vitis kernel bit for bit | container, open-source headers | no, batch |
| 3 | Real synthesis per target | XLS in container, Vitis and Catapult on host | no, batch |
| 4 | QoR collection | HLSFactory flows | out of scope |

## 7. Data flow for one design

1. `bundle.py` reads the folder, writes `bundle.json`, runs the oracle check. Fail: stop, label `no_oracle`.
2. `lift/agent.py` starts a container, mounts the folder, runs the agent with the guide and notes. The agent iterates rungs 0 and 1. It writes `allo/` and exits.
3. `project.py` emits the three targets and writes `emit.json` files.
4. `verify/differential.py` runs rung 2 for Catapult and xlscc.
5. If rung 1 failed and attempts remain, go to step 2 with the error report attached. Otherwise continue. Rung 2 failures are recorded in `emit.json` and `verify.json` and do not trigger another attempt.
6. After all designs in the run: `verify/synth_*.py` run rung 3 as a batch.
7. `report.py` writes `verify.json` per design and the run summary.

## 8. Error handling

- Every stage writes its status and reason into `verify.json` before raising. A crash in any stage never loses the results of earlier stages.
- Timeouts are per stage, listed above, and produce label `timeout` with the stage name.
- Tool crashes, meaning a non-zero exit that is not a diagnosed compile or simulation failure, produce `tool_crash` with the last 50 lines of the log.
- A container that dies mid-lift is retried once with the same attempt number before counting as `tool_crash`.
- The run is resumable: a design with a complete `verify.json` is skipped on rerun unless `--force` is passed.

## 9. Container image

One image, built by extending the Dockerfile in Allo's `docker` folder so the pinned LLVM build is done once and cached, with these additions:

- Node 24 and `@mariozechner/pi-coding-agent`, matching `docker_images/Dockerfile` in this repository.
- clang, make, cmake, git, tree.
- The vendored Vitis headers copied from `hlsfactory_agent/vitis_hls_include`.
- The open-source ac_types headers.
- XLS release binaries: `xlscc`, `opt_main`, `codegen_main`, and the synth-only headers.
- Environment variables `CC=clang`, `CXX=clang++`.

Allo's version and the XLS release are pinned in the Dockerfile and recorded in `summary.json`.

## 10. Package layout

A new repository, working name `hls-lift`, depending on the `hlsfactory_agent` package for the Docker runner and Pi session parsing, and on Allo only inside the container.

```
hls_lift/
  bundle.py
  lift/{prompt.py, agent.py, notes.py}
  project.py
  verify/{vitis_tb.py, differential.py, synth_xls.py, synth_vitis.py, synth_catapult.py}
  report.py
  config.py
docker/Dockerfile
exp/pilot/{run.py, subsets/}
tests/
```

The spec lives in this repository for now because this is where the design source and the harness are.

## 11. Pilot plan and success criteria

Two weeks, three phases. Phase 1 runs with no agent at all and exists to make sure the ladder works before anything is automated.

Phase 1, days 1 to 3: build the image, hand-lift three passing extracted designs of different shapes, one GEMM-like kernel, one streaming filter, one FFT. Push each through rungs 0 to 3. Every pain point goes into the lifting guide and the first `lifting_notes.md`. Exit criterion: all three pass rung 1, and rung 2 and rung 3 run to completion on each, whether pass or fail.

Phase 2, days 4 to 8: implement `bundle.py`, the agent harness, and the notes mechanism. Run on a fixed subset of 20 designs drawn from the passing set across at least eight repositories, k equal to 1, model `deepseek/deepseek-v4-flash` for comparability with the abstract. Exit criterion: the full data flow of section 7 completes for every design in the subset and produces a summary.

Phase 3, days 9 to 14: run on all designs that pass the oracle check, k equal to 3 on the 20-design subset only, and one run of the subset with a stronger model. Deliverables: the summary, the failure taxonomy, the primitive coverage table, and a one-page write-up for the mentor.

Decision point at the end of phase 2: if the rung 1 lift rate on the subset is below 30 percent and the dominant labels are `unsupported_construct` or `interface_mismatch`, revisit the reserved C++ dialect for the designs Allo cannot express. If the dominant labels are `allo_frontend_error` or `testbench_mismatch`, the guide and notes need work, not the architecture.

## 12. Risks

| Risk | Effect | Mitigation |
|---|---|---|
| The C++ to Python semantic hop is large | Low lift rate on pointer-heavy or bit-twiddling designs | Restructuring rules in the guide; the failure taxonomy is a result; C++ dialect held in reserve |
| Models know Vitis C++ far better than Allo | Frontend errors, hallucinated primitives | Syntax guide plus worked examples in context; fast rung 0 feedback; lessons file; one run with a stronger model |
| Allo backend coverage of primitives is uneven: Catapult has no partition, XLS honors only II equal to 1 and no partition or dataflow | Silent QoR differences later | Measured coverage table; dropped list per design; both facts already recorded in section 3.4 |
| Allo fixed-point types have no rounding or overflow modes | Designs using `AP_RND`, `AP_SAT` or similar cannot be lifted | Counted at bundle time; fail with `numeric_unsupported`; reported as its own bucket |
| Allo's XLS emitter is untested against real xlscc | Emitter bugs surface as rung 3 failures | Treat as findings; report upstream; keep `use_memory` on by default so large loops are not fully unrolled |
| Allo is maintained slowly by a few students, with no releases, about three commits a month in 2026, and no commits to the Catapult, XLS, or Intel emitters since February 2026 | Upstream fixes may not land; APIs may shift without notice | Pin a commit SHA in the image; keep a lab fork of Allo and apply emitter fixes there; the emitters are small enough to own, about 480 lines for Catapult and 1,150 for XLS |
| Original testbenches may not run | Fewer designs with an oracle | Oracle check reported as its own number; feeds back to HLSFactory-Agent |
| Allo version churn | Breakage between runs | Pinned version in image; version recorded in summary |
| No Catapult license | Rung 3 skipped for Catapult | Rung 2 still checks Catapult emission; report as skipped, not failed |
| Type-system bridging in the differential harness | Harness generation fails for unusual signatures | Supported kinds limited to scalars, arrays, streams of int or fixed; anything else is `skipped:harness_unsupported` |

## 13. Testing strategy

- Unit tests for `bundle.py` on three fixture folders: a normal design, one without `set_top`, one whose testbench fails.
- Unit tests for the differential harness generator on fixture signatures covering scalars, one-dimensional and two-dimensional arrays, streams, and one unsupported kind.
- A projection test that emits the fixture kernel on all three backends and checks the coverage table against a committed expectation, so an Allo upgrade that changes coverage fails a test.
- An integration test that runs the full section 7 flow on the three hand-lifted designs from phase 1, with the agent replaced by their committed `allo/` folders, so the ladder is tested without model spend.
- The 20-design subset is a committed list and serves as the regression set for prompt changes.

## 14. Open questions for the mentor

1. Is a Catapult license available on a lab machine, and if not, is Catapult worth keeping as an emit-only target in this phase.
2. Should the pilot use `deepseek/deepseek-v4-flash` for comparability, or a stronger model to find the ceiling first.
3. Is the 30 percent decision threshold in section 11 the right bar.
4. Does the lab want the converter in a new repository, as this document assumes, or inside this one.

## 15. Seams for later phases

- HLSFactory: copy `targets/<tool>/` folders into its design-source layout and run its flows for rung 4.
- Templates: a template instantiation writes `allo/` directly and skips stage 2.
- SyntheticHLS: its designs are bundles with a testbench and already-clean code, so they enter at stage 1 unchanged, and its mutation loop could gain a portability objective defined as the number of targets passing rung 2.
- Dynamatic: emit Vitis C++, strip pragmas, and widen arbitrary-precision types to native widths, recorded as lossy.
- Intel: Allo already reaches an Intel emitter in oneAPI style through `target="ihls"`, but only to write `kernel.cpp`; there is no execution path, no tests, and the emitter supports one function. It is a candidate fourth target that matches the mentor's interest in Intel, and it needs a run path, tests, and a oneAPI toolchain before it can enter the ladder.
