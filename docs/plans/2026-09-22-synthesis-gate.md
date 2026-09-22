# Synthesis gate and target split (2026-09-22)

All 16 hpfft Catapult translations passed our checks (compile, testbench matches the original);
3 of 16 synthesize. A translation now counts only once the target tool produces hardware.

## Target contract

Each target is a `TargetSpec` in `hlsfactory_agent/targets/<tool>.py`. `blocking_types` lists types the
target cannot represent; `translate.feasibility` rejects such designs before any model spend. Synthesis fields:

| field | meaning |
|---|---|
| `synth_command` | tool invocation; `$NAME` expands from `synth_env`, then the environment |
| `synth_timeout_s` | seconds before TIMEOUT |
| `synth_env` | tool paths and license; overrides the environment |
| `synth_files` | files written into the run area first (Catapult: `synth.tcl` wrapper that exits) |
| `synth_success_marker` | glob that must exist for a PASS |
| `parse_report` | run area -> `SynthResult` (status, latency, throughput, area, first_error, failure_class) |

`hlsfactory_agent/synth.py:run_synth(target, dir)` is target-agnostic.

## Ownership

| area | owner |
|---|---|
| `targets/catapult.py`, Catapult rules in `rewrite.py`, `exp/run_translate/run.py --synth` | Tanmay |
| `targets/xlscc.py`, xlscc build, feasibility pre-check in `scan.py` | Justin |
| `spec.py`, `synth.py`, shared flow in `translate.py` | change by agreement |

## Next

1. Catapult: port the 11 hand fixes into rules (see hpfft `derived_directives_findings.md`), then derive resource directives after `go assembly` instead of predicting paths.
2. XLS: build xlscc, verify the two PQC designs, fill in the XLSCC synth fields.
3. Together: sort the 13 Catapult failures into rule / agent / reject.
