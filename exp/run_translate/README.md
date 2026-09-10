# Translation flow: Vitis HLS -> Catapult HLS

First cross-tool translation flow, built on the existing Pi-in-Docker agent harness.
The agent takes a standalone Vitis HLS design folder (as produced by HLSFactory-Agent) and
produces an equivalent Catapult HLS design folder. Deterministic checks run after the agent.

Code: `hlsfactory_agent/translate.py`. Tests: `tests/test_translate.py`.

## What is here

```
designs/vitis_mac/            small Vitis fixture: ap_fixed MAC over hls::stream, three pragma kinds
expected/catapult_mac/        hand-translated reference output, used to validate the check path without an LLM
expected/catapult_mac_check.json  result of running the in-container checks on the reference (all pass)
run.py                        runs the agent on one or more design folders
runs/                         outputs, git-ignored
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

  Artifacts, including the session transcript, are in `results/2026-09-10-vitis_mac-to-catapult/`.
- Catapult synthesis of the fixture, both the hand reference and the agent output, verified on a lab machine with
  Catapult Prime Synthesis 2026.2: `go extract` completes with run.tcl unmodified, II 1, latency 8 cycles, one
  multiplier. See `results/2026-09-10-catapult-synth/` once that commit is pushed from the lab machine.

- Full loop on a real repository, `UCLA-VAST/HP-FFT-HLS` from the base run, 2026-09-10: extraction produced 16
  standardized Vitis designs (FFT variants, the main source is about 2,600 lines). Every one of the 16 passes its own
  testbench on the original code (oracle). 15 of 16 translate to Catapult and pass every check; the one failure,
  `n256_no_StagePipeline`, is a missing `#pragma hls_design top` and nothing else, and its run.tcl names the top so
  it should still synthesize. Total agent cost 0.37 USD, median agent time about six minutes per design. Two
  designs left commented-out Vitis pragmas in place; the checker records those separately and does not fail them.
  Results, originals, reports, checks and transcripts: `results/2026-09-10-HP-FFT-HLS-to-catapult/`.
  Catapult synthesis of these 16 has not been run yet.

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
Logs, reports, and full notes (including a license-server workaround specific to that machine)
are in `results/2026-09-10-catapult-synth/`.

## Known gaps

- `ARRAY_PARTITION`, `INTERFACE`, `DEPENDENCE`, `BIND_STORAGE` have no source-level Catapult equivalent and are
  dropped with a note in the report. Catapult sets these in TCL; that mapping is future work.
- `DATAFLOW` maps to per-function `hls_design block`, which is lossy.
- Only one fixture design so far. Next: run on real extracted designs from the base run.
- Adding a target means adding a `TargetSpec` entry; the prompt and checks are generated from it.
