# Translation flow: Vitis HLS -> Catapult HLS, XLS, Bambu, Dynamatic

Cross-tool translation flows built on the existing Pi-in-Docker agent harness. A standalone Vitis HLS
design folder (as produced by HLSFactory-Agent) becomes an equivalent design for a target tool. Targets
are data (`TargetSpec` in `translate.py`): Catapult, Bambu and Dynamatic are complete through synthesis, XLS through emulation.

Code: `hlsfactory_agent/translate.py` (targets, prompt, checks, run class), `scan.py` (inventory),
`rewrite.py` (mechanical pre-pass). Tests: `tests/`.

## Pipeline for one design

1. **Scan**: `scan.py` inventories every `#pragma HLS` kind, vendor type, header, and risky construct;
   written as `scan.json` for the agent and used later to check the report.
2. **Oracle**: the design's own testbench must pass on its original code (`run_oracle_check`). Before it,
   `repair_top` and `normalize_vitis_script` fix the extracted `synth.tcl` (top name, unset Tcl variables,
   include paths, removed options), and `--repair-extraction` (`extraction_repair.py`) makes escaped includes
   resolve and recovers missing headers and data files from a repository clone (`--repo-clone`).
3. **Mechanical pre-pass**: `rewrite.py` copies the design and swaps headers and types, moves loop
   pragmas to the target's placement, drops pragmas with no equivalent, inserts the top marker, and
   writes `rewrite_log.json` with a `residue` list of what it could not do safely. No numerics are guessed.
4. **Agent**: Pi fixes the residue, ports the testbench, writes the driver script and a translation
   report. Bounded by `attempts`; each failed attempt feeds a retry prompt built from the harness's own
   findings (`build_retry_prompt`). Before a retry is spent, the harness inserts missing required includes
   and repairs the fence mechanically (fenced lines back to the pre-pass text, kept if no check gets worse).
5. **Checks**, independent of the agent's claims: static (required files, no leftover Vitis identifiers,
   top marker, driver complete, report accounts for every pragma kind in the scan), in-container clang
   syntax check and testbench run with the target's headers, and `compare_outputs` against the original
   testbench's stdout.
6. Everything is recorded: `run_data.json` (prompt, sessions per attempt), `check_data.json` (all checks,
   attempts), plus the transcripts.

## What is here

```
designs/vitis_mac/            small Vitis fixture: ap_fixed MAC over hls::stream, three pragma kinds
expected/catapult_mac/        hand-translated reference output, used to validate the check path without an LLM
run.py                        runs the agent on one or more design folders
run_repo.py                   full loop on a repository: extract, oracle-check, translate, save
runs/, results/               outputs, git-ignored; results are not committed to this repository
```

## How the flow works

1. The design folder is copied to `run_area/input_design`. The vendored Catapult headers
   (`hlsfactory_agent/ac_types_include`, hlslibs ac_types, Apache 2.0) and the Vitis headers are
   copied next to it so the agent can compile inside the container.
2. The Pi agent gets a prompt built from the Catapult target table in `translate.py`: header map,
   type map, pragma rules with placement, top-function marker, driver script template, and a
   required translation report. It writes `run_area/output_design`.
3. After the agent exits, the harness runs its own checks, independent of anything the agent claims:
   - static: required files present, no Vitis identifiers left, top marker present, `run.tcl` complete;
   - in container: clang syntax check per source, build and run the translated testbench with a timeout.
4. `run_data.json` (prompt, session trace, agent output tail) and `check_data.json` are written per run.

## Running

Needs Docker with the `hlsfactory-agent` image and a valid `OPENROUTER_API_KEY` in `.env`.
Run from the repository root with the package on the path, either through `uv run` or `PYTHONPATH`:

```
uv run python exp/run_translate/run.py --target catapult
PYTHONPATH=. python exp/run_translate/run.py --design exp/run_translate/designs/vitis_mac --model deepseek/deepseek-v4-flash
PYTHONPATH=. python exp/run_translate/run.py --target catapult --synth none        # translation checks only, no tool runs
```

Synthesis is off by default (a run stops at compile). `--synth` synthesizes the source with Vitis HLS and the
passing translation with the target tool, each in a fresh container with the tool install mounted read-only
(`hlsfactory_agent/tools.py`); `--synth source|target` selects one side. `--no-translate` stops after path A.
`--agent-tools` mounts the tools into the agent's container too; off, the agent cannot call a tool. Results
are cached by content under `<runs-dir>/.synth_cache/`.
`--require-source-synth` skips translating a design whose Vitis synthesis did not pass. `--synth-timeout` overrides
the target's budget; `--degrade N` (default 2) retries a failed target synthesis with one log-driven edit per round.

Full loop on a real repository, extraction with HLSFactory-Agent followed by an oracle check and translation of
every extracted design, with outputs saved under `results/<date>-<repo>-to-<target>/` for synthesis elsewhere:

```
PYTHONPATH=. python exp/run_translate/run_repo.py --repo UCLA-VAST/HP-FFT-HLS --target catapult --jobs 4
PYTHONPATH=. python exp/run_translate/run_repo.py --repo UCLA-VAST/HP-FFT-HLS --skip-extract   # reuse a previous extraction
```

The oracle check (`run_oracle_check` in `translate.py`) compiles each extracted design's own testbench against its
own sources with clang and the Vitis headers inside the container and runs it. The sources are the ones `synth.tcl`
lists, with its `-cflags`; a source the testbench already includes is dropped, a second `main` loses to the `-tb`
file, and duplicate symbols across units fall back to a single translation unit (`oracle.json` records
`cpp_files`, `dropped`, `cflags`, `build_mode`). A design whose testbench does not pass on the original code has
no reference to translate against; it is still translated, but flagged. `--oracle-only` runs just this step
(`--oracle-timeout` seconds per design, default 120) and keeps an earlier translation's checks if the run dir has
them; `--reoracle` does the same on a `--resume`:

```
PYTHONPATH=. python exp/run_translate/run_repo.py --repo ETHZ-DYNAMO/balor --skip-extract --oracle-only \
    --repair-extraction --repo-clone /export/hdd/scratch/tshukla31/census_repo_clones --oracle-timeout 900
```

Unit tests, no Docker or key needed:

```
uv pip install pytest
.venv/Scripts/python -m pytest tests
```

Check path only, no LLM, Docker needed:

```python
from pathlib import Path
from hlsfactory_agent.translate import CATAPULT, run_checks_standalone
run_checks_standalone(Path("exp/run_translate/expected/catapult_mac"), CATAPULT)
```

## Status, 2026-09-10

- Unit tests pass (12).
- The hand-translated reference passes every check inside the container: syntax, build, testbench PASS.
- First live run, `deepseek/deepseek-v4-flash` on `designs/vitis_mac`: every check passed. The agent's
  `mac.h`, `mac.cpp`, and `testbench.cpp` are identical to the hand-translated reference apart from whitespace,
  the report accounts for all four original pragmas (one translated and moved, three dropped with reasons), and
  the translated testbench printed PASS with exit 0 inside the container.

  | Metric | Value |
  |---|---|
  | wall time | 1 min 54 s |
  | model messages | 16 |
  | tool calls | 22 (12 bash, 4 read, 6 write) |
  | tokens in / out | 12,573 / 4,824 |
  | cost | 0.0034 USD |

- Catapult synthesis of the fixture, both the hand reference and the agent output, verified on a lab machine with
  Catapult Prime Synthesis 2026.2: `go extract` completes with run.tcl unmodified, II 1, latency 8 cycles, one
  multiplier.

- Full loop on a real repository, `UCLA-VAST/HP-FFT-HLS` from the base run, 2026-09-10: extraction produced 16
  standardized Vitis designs (FFT variants, the main source is about 2,600 lines). Every one of the 16 passes its own
  testbench on the original code (oracle). 15 of 16 translate to Catapult and pass every check; the one failure,
  `n256_no_StagePipeline`, is a missing `#pragma hls_design top` and nothing else, and its run.tcl names the top so
  it should still synthesize. Total agent cost 0.37 USD, median agent time about six minutes per design. Two
  designs left commented-out Vitis pragmas in place; the checker records those separately and does not fail them.
  Catapult synthesis of these 16 has not been run yet. Results are kept locally and are not committed; the dataset
  will live in its own repository.

- Harness fixes made during that run: all file reads and writes forced to UTF-8 (a Windows default-encoding crash
  while reading a session transcript; the same one-line fix applied to `core.py`), a workspace wipe that survives
  OneDrive file locks, per-design error isolation in the batch, resume, and a `--only` selector.

## Catapult synthesis, 2026-09-10

Both committed Catapult designs were synthesized with Catapult Prime 2026.2 on a lab machine:

| Design | Result |
|---|---|
| `expected/catapult_mac` (hand-translated reference) | PASS: `go extract` completed, RTL written |
| `results/2026-09-10-vitis_mac-to-catapult` (live agent output) | PASS: identical reports (sources are byte-identical) |

`run.tcl` ran unmodified: `nangate-45nm_beh`, `ccs_sample_mem`, and `CppStandard c++11` are all
accepted by this install. Schedule: latency 8, throughput 10 cycles, II=1 on the 8-tap loop at
5 ns; one 16x16 multiplier plus a 32-bit accumulator, as expected for a rolled MAC.
Logs and reports are kept outside this repository.

## XLS target

`--target xlscc`. Same ac types as Catapult (xlscc ships ac-compatible headers), streams become
`__xls_channel<T>`, top gets `#pragma hls_top`, loop pragmas use the Catapult spellings. For host
emulation the translated code includes `xls_emu.h` (`hlsfactory_agent/xls_emu_include/`), a small
channel emulation that is empty under `__SYNTHESIS__`. The driver `run_xlscc.sh` runs xlscc, opt_main
and codegen_main; the synthesis rung for XLS needs those binaries in the Docker image and is not run yet.

## Bambu target

`--target bambu`. Same ac types as Catapult (Bambu bundles ac_types and ac_math); native `float`/`double`
and `<math.h>` stay, since the driver links Bambu's soft libm (`-lm`); `std::complex` becomes `ac_complex`.
Loop pragmas use Bambu's spellings on the line before the loop (`#pragma HLS unroll [factor=n]`,
`#pragma HLS inline`); `PIPELINE` is dropped (Bambu rejects it on loops) and `DATAFLOW` is dropped (lossy).
No top marker: `run_bambu.sh` names the top with `--top-fname` and infers interfaces
(`--generate-interface=INFER`: `ac_channel` parameters become FIFOs, arrays memories). Synthesis runs the
extracted AppImage (`BAMBU_APPDIR`, default `/export/hdd/scratch/tshukla31/tools/bambu-2024.10`) in the
container at 10 ns (`-I.` for the shim headers); PASS means `<top>.v` was written, area/DSP/FF come from
the log, latency is not measured. Bambu 2024.10 aborts on a top that returns an ac type by value, so the
pre-pass makes it `void` with a trailing `ret_out` reference and leaves the testbench call sites as residue;
a loop label moves above the pragma. One recovery step: `unexpected pattern` from interface inference
(a pointer the top passes to memcpy) reruns with `--generate-interface=MINIMAL`.

## Dynamatic target

`--target dynamatic`. Plain C, one file: the kernel `<top>.c` also holds `main()`, which calls the kernel
once through `CALL_KERNEL(top, args...)` with variables named like the parameters (Dynamatic dumps them as
co-simulation inputs). The kernel calls no function, declares no local array (hoisted to parameters appended
after the originals), uses no pointers, globals or 64-bit types; `ap_int<N>` becomes `int8/16/32_t` or
`_BitInt(N)`; `UNROLL` becomes `#pragma clang loop unroll_count(n)`, `ARRAY_PARTITION` becomes
`#pragma DYN array_partition`, `PIPELINE`/`INLINE`/`DATAFLOW` are dropped (dynamic scheduling, no calls).
Designs with `ap_fixed`, `hls::stream` or `hls::vector` are reported untranslatable before the pre-pass. The
checks compile the file as C17 and build it as C++ (the tool does both). Synthesis runs in the image built by
`docker_images/build_dynamatic.sh` (`hlsfactory/dynamatic:latest`, built with `--enable-cbc`):
`compile --buffer-algorithm fpga20 --milp-solver cbc` (throughput-driven MILP buffering, profiled through the
`CALL_KERNEL` in `main()`, which is why `main()` must print nothing to stdout: the pre-pass redirects prints to stderr and the check forbids `printf`/`puts`/`stdout`), `write-hdl --hdl vhdl`, `simulate --simulator ghdl`; PASS means the VHDL was
written and the C/RTL co-simulation matched, latency is the simulated cycle count. A target check requires
exactly one `CALL_KERNEL`, naming the function the file is named after (the verifier records one transaction).
Recovery steps, one per round: `on-merges` buffering when profiling or the MILP failed, `--disable-lsq` when
the log names the LSQ.

## Run statistics

`run_repo.py` records per-stage times, the attempt list and host samples under `<runs>/stats/`;
`PYTHONPATH=. python exp/run_translate/stats_report.py --runs-dir ... --results-dir ... [--baseline-results ...]`
writes `docs/RUN_STATISTICS.md`.

## Known gaps

- `ARRAY_PARTITION`, `INTERFACE`, `DEPENDENCE`, `BIND_STORAGE` have no source-level Catapult equivalent and are
  dropped with a note in the report. Catapult sets these in TCL; that mapping is future work.
- `DATAFLOW` maps to per-function `hls_design block`, which is lossy. XLS drops it.
- The pre-pass moves a loop pragma to the nearest loop header within three lines; unusual layouts land in `residue`.
- XLS synthesis rung not run; lessons store and pass@k sampling not built.
- Adding a target means adding a `TargetSpec` entry; the prompt, pre-pass, and checks are generated from it.
