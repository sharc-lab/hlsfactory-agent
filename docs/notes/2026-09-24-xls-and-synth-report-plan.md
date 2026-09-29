# Plan: basic XLS support and a synthesis diagnostics reporter

Status: draft for review, not approved. Nothing here is implemented yet.
Background reading: `docs/notes/2026-09-24-xls-overview.md` (how XLS works), `docs/plans/2026-09-22-synthesis-gate.md`
(target contract and ownership).

Revision 2026-09-24: build the barebones path first and learn from the first real run, instead of validating the
XLS install by hand beforehand. The earlier "Phase 0: reconnaissance" is gone; what it was meant to find out is now
the "first run" checklist in Milestone 1.

## Goal, in three milestones

1. **Barebones XLS.** A Vitis C++ design goes in; an XLS design comes out that gets through all three XLS tool steps
   (xlscc, opt_main, codegen_main) on the research server and produces Verilog. Whether the hardware is any good is
   not the question yet.
2. **XLS diagnostics.** The same run with a real delay model and clock target, and the numbers pulled out: pipeline
   stages, critical path, failure class.
3. **Synthesis diagnostics reporter.** One script that runs synthesis over saved translation results for any target
   (Catapult, XLS, optionally the original on Vitis) and writes one table.

Each milestone is usable on its own. Stop after any of them.

## Non-goals

- Wiring synthesis into the translation run loop (`--synth` on `exp/run_translate/run.py`). Tanmay's item.
- Putting XLS into the Docker image.
- Tuning designs for XLS (clock exploration, II tuning, restructuring for performance).
- Making the translation agent smarter about XLS. Failures from Milestone 1 will show what it needs; that is
  follow-up work.

## What already exists

Most of "Vitis in, XLS out" is already built. What is missing is the last part: running the real XLS tools.

| piece | where | state |
|---|---|---|
| Vitis design in, XLS-dialect C++ out | `run.py --target xlscc`, `run_repo.py --target xlscc` | works: scan, feasibility (floats rejected), pre-pass, agent, checks |
| XLS target spec | `hlsfactory_agent/targets/xlscc.py` | type, header, pragma maps; no synth fields |
| XLS driver script | `render_xlscc_driver` in the same file, written as `run_xlscc.sh` | runs the three tools with bare names, no include paths, `--delay_model=unit --pipeline_stages=1` |
| correctness check | `xls_emu.h` plus clang in Docker | proves the C++ behaves like the original; does not prove xlscc accepts it |
| generic synth runner | `hlsfactory_agent/synth.py:run_synth` | works, no callers |
| result type | `SynthResult` in `hlsfactory_agent/spec.py` | status, latency, throughput, area, first_error, failure_class |
| saved results | `exp/run_translate/results/<date>-<repo>-to-<target>/<design>/{vitis,<target>}/` | written by `run_repo.py` |
| XLS, Catapult, Vitis tools | research server (SSH) | not available locally |

Two details matter for running on the server:

- `translate.py` imports `docker` at module level; `synth.py`, `spec.py` and `targets/*.py` do not. Anything that
  runs on the server imports only those.
- The saved results hold only the translated design folder. The include directories the translated code needs
  (`xls_emu_include/`, and ac headers) live in the package, not in the results. On the server, xlscc must be pointed at
  them explicitly or it will fail to find `xls_emu.h`.

## Milestone 1: barebones XLS

Done when: for a translated design, one command on the server runs xlscc, opt_main and codegen_main, and reports
either "PASS, Verilog written" or "FAILED at <step>: <first error line>".

### Step 1. Make the driver runnable anywhere

Change `render_xlscc_driver(top, sources)` in `targets/xlscc.py` so `run_xlscc.sh`:

- finds the tools through `${XLS_BIN}` when it is set, and `PATH` otherwise;
- passes include directories to xlscc through `${XLS_INCLUDES}` (so the server can point at xlscc's own ac headers
  and at our `xls_emu_include/` without the script hardcoding paths);
- writes each step's errors to its own log (`xlscc.log`, `opt.log`, `codegen.log`), so a failure names its step;
- keeps `--delay_model=unit --pipeline_stages=1` for now. For this milestone that is the right choice: with no
  clock target the scheduler cannot fail on timing, so any failure is a real "XLS does not accept this" failure.
  Milestone 2 changes it.

Sketch (flag spellings confirmed on the first run):

```sh
#!/bin/sh
set -e
X=${XLS_BIN:+$XLS_BIN/}
"${X}xlscc" <sources> --top <top> ${XLS_INCLUDES} > <top>.ir 2> xlscc.log
"${X}opt_main" <top>.ir > <top>.opt.ir 2> opt.log
"${X}codegen_main" <top>.opt.ir --generator=pipeline --delay_model=unit --pipeline_stages=1 \
    --module_name=<top> --output_verilog_path=<top>.v 2> codegen.log
```

`driver_required_tokens` (`xlscc`, `opt_main`, `codegen_main`) stay satisfied, so static checks on agent-written
drivers still pass.

### Step 2. XLS synth fields and a minimal parser

In `targets/xlscc.py`:

- `synth_command=("sh", "run_xlscc.sh")`
- `synth_env={}`: `XLS_BIN` and `XLS_INCLUDES` come from the server environment; no machine paths committed.
- `synth_success_marker="*.v"`
- `parse_report=parse_xlscc_report`

`parse_xlscc_report(dir_run) -> SynthResult`, minimal version:

- Go through the steps in order. The first one whose log contains an error, or whose output file is missing or
  empty, is the failure. Return `FAILED` with `first_error` (first error line, 300 characters, as the Catapult
  parser does) and `failure_class` set to the step name for now: `xlscc`, `opt`, `codegen`.
- Otherwise `PASS`.

No metrics yet, and no change to the shared `SynthResult`.

### Step 3. A way to run it on the server

A small script, `exp/synth_report/run.py`, that takes a results folder, copies each `<design>/xlscc/` into a fresh
run area, calls `run_synth(XLSCC, run_area)`, and prints one line per design. This is the seed of the Milestone 3
reporter; starting it here avoids writing a throwaway script.

```
PYTHONPATH=. python exp/synth_report/run.py --results exp/run_translate/results/<date>-<repo>-to-xlscc
```

### Step 4. Tests (local, no tools)

- the driver renders with all three steps, the per-step logs, and the env var hooks;
- `parse_xlscc_report` on hand-written fake logs: PASS, a failure at each step, missing Verilog.

Replace the fake logs with real ones once the first run produces them.

### Step 5. First run

1. Locally: `PYTHONPATH=. python exp/run_translate/run_repo.py --repo <repo> --target xlscc`. Start with a repo whose
   designs are integer or fixed point: floats are rejected before translation.
2. Copy the results folder and the package to the server.
3. Set `XLS_BIN` and `XLS_INCLUDES`, run the script from Step 3.

What the first run is likely to expose, and where to look:

| symptom | likely cause | fix |
|---|---|---|
| `xlscc: not found` | tools not on `PATH` | set `XLS_BIN` |
| xlscc cannot find `xls_emu.h` or `ac_int.h` | include dirs missing | add them to `XLS_INCLUDES`; our `xls_emu_include/` plus xlscc's bundled ac headers |
| errors inside `xls_emu.h` | xlscc does not define `__SYNTHESIS__` | change the header's guard to whatever xlscc defines |
| unknown flag | flag spelling differs in the installed version | check `--help`, fix the template |
| errors about the top's channel arguments | this xlscc version wants a block definition file | see "Later: channel tops" below |
| errors about pointers, loops, unsupported constructs | the translation, not the setup | record them; they are the input for improving the pre-pass and prompt |

Record what was fixed and the answers (tool paths, flags, xlscc version) in the Open questions section of the XLS
overview note.

## Milestone 2: XLS diagnostics

Done when: a passing XLS run reports pipeline stages and critical path in picoseconds, and failures carry a
meaningful class.

1. **Real scheduling.** Driver switches to `--delay_model=${XLS_DELAY_MODEL:-asap7}` and
   `--clock_period_ps=${XLS_CLOCK_PS:-1000}`, drops `--pipeline_stages`. The scheduler now picks the number of stages,
   so the stage count becomes a measurement. New failure possibility: an operation too slow for the clock
   (`schedule-infeasible`).
2. **Report outputs.** codegen also writes `--output_schedule_path` and `--output_signature_path`; the driver adds
   `benchmark_main` on the optimized IR, writing `benchmark.log`. Its failure does not fail the run.
3. **Extend `SynthResult`** (`spec.py` is shared: agree with Tanmay first). Two additive fields with defaults:
   `stage: str` (tool step that failed) and `metrics: dict` (target-specific numbers). The Catapult parser and its test
   are unaffected.
4. **Parser grows.** `failure_class` from a first-match table of message fragments, like Catapult's
   `FAILURE_CLASSES`, built from the real logs collected in Milestone 1. On PASS, `metrics` gets `pipeline_stages`,
   `critical_path_ps`, `clock_period_ps`, `delay_model`; `latency` is set to the stage count so a shared table has one
   comparable column.
5. **Tests** on real captured logs, committed as small text fixtures under `tests/fixtures/xls/`.

## Milestone 3: synthesis diagnostics reporter

Grow the Step 3 script into the full reporter.

### Inputs and outputs

Input: results folders written by `run_repo.py`. The target comes from the folder name (`-to-<target>`) or
`--target`.

Output, under `--out` or next to the input:

```
synth_<date>/
  <design>/<target>/       run area: copy of the translated design plus every tool log and report
  <design>/synth.json      SynthResult for this design
  synth_summary.json
  synth_summary.csv
  SYNTH_REPORT.md          one table, pass counts, failure-class counts
```

### Behaviour

- Never synthesize inside the saved results; always copy to a fresh run area.
- `--jobs` (default 1: Catapult license seats and server load), `--only`, `--limit`, `--resume` (skip designs with a
  `synth.json`), `--timeout`, and `--clock-ps` for XLS, following `run_repo.py`'s conventions.
- A crash in one design records an error row; the batch continues.
- Table columns: design, translation checks passed (from `check_data.json`), synth status, failed step and failure
  class, then the target's own metrics. Catapult: latency, throughput, area. XLS: stages, critical path, clock.
  Metrics from different tools go in separate columns and are never merged, because they do not measure the same
  thing.
- Failure-class counts at the bottom feed "sort the Catapult failures into rule / agent / reject" (item 3 of the
  synthesis-gate plan).

### Files

| file | responsibility |
|---|---|
| `hlsfactory_agent/synth_report.py` (new) | run a results folder, collect rows, write the report; imports only `synth`, `spec`, `targets` |
| `exp/synth_report/run.py` (from Milestone 1) | command line |
| `tests/test_synth_report.py` (new) | table rendering from fake rows, folder discovery, `--resume` skipping; no tools needed |

## Later, optional

- **Channel tops.** If the installed xlscc needs a block definition file for designs with stream arguments, generate
  it from the scan's list of the top's stream arguments and directions. Only if the first run shows it is needed.
- **Original Vitis design as baseline.** Run `vitis_hls -f synth.tcl` on `<design>/vitis/` and parse
  `<solution>/syn/report/csynth.xml` (latency, II, BRAM, DSP, FF, LUT). Reuse the timeout handling in
  `exp/run_base/tools.py`.
- **Area for XLS with Yosys**, if it is installed: `yosys -p "read_verilog -sv <top>.v; synth -top <top>; stat"`,
  cell count into `metrics["cells"]`. A Liberty file (ASAP7) gives real area.
- **`--synth` on the translation run**, once the parsers exist. Tanmay's item.
- **XLS in the Docker image**, so the XLS step can run locally and inside the fixer loop.

## Coordination

| change | file | owner |
|---|---|---|
| driver, synth fields, parser | `targets/xlscc.py` | Justin |
| `stage` and `metrics` fields (Milestone 2) | `spec.py` | agree with Tanmay first |
| `run_synth` (should need no change) | `synth.py` | agree if it does |
| reporter | new files | Justin; tell Tanmay, since it runs Catapult too |
| anything in `targets/catapult.py` | | Tanmay |

## Verification checklist

- [ ] Local unit tests pass: `.venv/bin/python -m pytest tests -q`
- [ ] Milestone 1: at least one translated design reaches Verilog on the server; every failure names its step
- [ ] Milestone 2: a passing design reports stages and critical path in ps; the Catapult parser test still passes
- [ ] Milestone 3: the reporter over the HP-FFT Catapult results reproduces the known 3 of 16 synthesizing, with
      failure classes for the other 13
- [ ] No tool paths, license servers, or results committed

## Risks

| risk | effect | mitigation |
|---|---|---|
| Installed xlscc differs from the docs | first run fails on setup, not on designs | expected; the first-run table lists the usual causes |
| Most translated designs fail xlscc (pointers, unbounded loops) | few PASS rows at first | that is the point of Milestone 1: the failures show what the pre-pass and prompt need |
| The `unit` delay model outlives Milestone 1 | numbers look real but mean nothing | Milestone 2 replaces it; the report records the delay model used |
| Catapult license seats | parallel runs fail with a license error | `--jobs 1` default; the `license` failure class exists |
| Server Python or packages differ | reporter does not start | reporter imports only the standard library and tool-free modules |
| Changing `SynthResult` clashes with Tanmay's work | merge conflicts | additive fields with defaults, agreed first, and not until Milestone 2 |

## Order and size

| step | size |
|---|---|
| Milestone 1, steps 1 to 4 (local) | half a day |
| Milestone 1, step 5 (server, fixing what breaks) | half a day to a day |
| Milestone 2 | a day |
| Milestone 3 | a day |
