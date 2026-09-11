# Translation flow: Vitis HLS -> Catapult HLS, Vitis HLS -> XLS

Cross-tool translation flows built on the existing Pi-in-Docker agent harness. A standalone Vitis HLS
design folder (as produced by HLSFactory-Agent) becomes an equivalent design for a target tool. Targets
are data (`TargetSpec` in `translate.py`): Catapult is complete through synthesis, XLS through emulation.

Code: `hlsfactory_agent/translate.py` (targets, prompt, checks, run class), `scan.py` (inventory),
`rewrite.py` (mechanical pre-pass). Tests: `tests/`.

## Pipeline for one design

1. **Scan**: `scan.py` inventories every `#pragma HLS` kind, vendor type, header, and risky construct;
   written as `scan.json` for the agent and used later to check the report.
2. **Oracle**: the design's own testbench must pass on its original code (`run_oracle_check`).
3. **Mechanical pre-pass**: `rewrite.py` copies the design and swaps headers and types, moves loop
   pragmas to the target's placement, drops pragmas with no equivalent, inserts the top marker, and
   writes `rewrite_log.json` with a `residue` list of what it could not do safely. No numerics are guessed.
4. **Agent**: Pi fixes the residue, ports the testbench, writes the driver script and a translation
   report. Bounded by `attempts`; each failed attempt feeds a retry prompt built from the harness's own
   findings (`build_retry_prompt`).
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
```

Full loop on a real repository, extraction with HLSFactory-Agent followed by an oracle check and translation of
every extracted design, with outputs saved under `results/<date>-<repo>-to-<target>/` for synthesis elsewhere:

```
PYTHONPATH=. python exp/run_translate/run_repo.py --repo UCLA-VAST/HP-FFT-HLS --target catapult --jobs 4
PYTHONPATH=. python exp/run_translate/run_repo.py --repo UCLA-VAST/HP-FFT-HLS --skip-extract   # reuse a previous extraction
```

The oracle check (`run_oracle_check` in `translate.py`) compiles each extracted design's own testbench against its
own sources with clang and the Vitis headers inside the container and runs it. A design whose testbench does not
pass on the original code has no reference to translate against; it is still translated, but flagged.

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

## Known gaps

- `ARRAY_PARTITION`, `INTERFACE`, `DEPENDENCE`, `BIND_STORAGE` have no source-level Catapult equivalent and are
  dropped with a note in the report. Catapult sets these in TCL; that mapping is future work.
- `DATAFLOW` maps to per-function `hls_design block`, which is lossy. XLS drops it.
- The pre-pass moves a loop pragma to the nearest loop header within three lines; unusual layouts land in `residue`.
- XLS synthesis rung not run; lessons store and pass@k sampling not built. See `docs/plans/2026-09-10-conversion-next-steps.md`.
- Adding a target means adding a `TargetSpec` entry; the prompt, pre-pass, and checks are generated from it.
