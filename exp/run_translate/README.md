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

```
python exp/run_translate/run.py --target catapult
python exp/run_translate/run.py --design exp/run_translate/designs/vitis_mac --model deepseek/deepseek-v4-flash
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
- The full agent path runs end to end (workspace, container, Pi invocation, session capture, checks, outputs).
  The key currently in `.env` is not an OpenRouter key, so the agent step returns 401 and the run is recorded
  as failed. A live translation has therefore not been observed yet. Once a valid key is in `.env`, the
  command above is the whole experiment.
- Not verified: Catapult synthesis of the output. `run.tcl` follows the structure Allo's Catapult backend
  emits (nangate sample library, 5 ns clock) and needs a Catapult install to run.

## Known gaps

- `ARRAY_PARTITION`, `INTERFACE`, `DEPENDENCE`, `BIND_STORAGE` have no source-level Catapult equivalent and are
  dropped with a note in the report. Catapult sets these in TCL; that mapping is future work.
- `DATAFLOW` maps to per-function `hls_design block`, which is lossy.
- Only one fixture design so far. Next: run on real extracted designs from the base run.
- Adding a target means adding a `TargetSpec` entry; the prompt and checks are generated from it.
