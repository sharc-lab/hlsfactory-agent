# Architecture

Automatic translation of standalone HLS designs between tools. Vitis is always the source; Catapult and
XLS are the targets. This file describes the pipeline as it is in the repo now: no history, no plans.
Updated 2026-09-24.

```
GitHub repo -(0)-> Vitis design -(1)-> scan -(2)-> feasibility -(3)-> pre-pass -(4)-> resource channel
   -(5)-> agent -(6)-> static checks -(7)-> container checks -(8)-> synthesis gate
```

## 0. Extraction — `hlsfactory_agent/core.py`
An agent (Pi, via OpenRouter) in the `hlsfactory-agent` container pulls standalone Vitis designs out of a
GitHub repo. Vitis-only; the upstream HLSFactory-Agent project. Output: one folder per design with sources,
testbench, `synth.tcl`.

## 1. Scan — `hlsfactory_agent/scan.py`
Read-only inventory of one design: every pragma, vendor type, header, and for each array a resource
pragma names, argument vs local, all dimensions, total size, with `#define` arithmetic resolved.
Output `scan.json`. Does not fold conditionals (a size like `N<2 ? 1 : N/2` stays unknown).

## 2. Feasibility — `translate.feasibility`
Scan types against the target's `blocking_types`; reject before any model spend. XLS blocks float and
double; Catapult blocks nothing today.

## 3. Mechanical pre-pass — `hlsfactory_agent/rewrite.py`
Lookup-table conversion without judgment: headers, types with widths and rounding modes preserved,
`PIPELINE`/`UNROLL`/`INLINE` renamed and moved to the target's placement, unsupported pragmas dropped with a
reason, top marker inserted. Anything uncertain goes on `rewrite_log.json`'s `residue` list.
Covers the integer/fixed-point dialect only: no rules for `std::complex`, native float, math calls,
`hls_design block` placement, static channels, `<stdint.h>`.

## 4. Resource channel — `hlsfactory_agent/targets/catapult.py`, `catapult_directives.json`
`ARRAY_PARTITION` pragmas become lines in `run.tcl`; `directives.json` records rendered and unrendered
(with reason). Paths are looked up, not predicted: `run.tcl` defines a `resource_directive` Tcl proc that,
after `go assembly`, globs Catapult's real `:rsc` paths (nested per block, one per struct field) and applies
the directive to every match. Partition rules are dimension-aware: outer-dim split -> `BLOCK_SIZE`,
innermost -> `INTERLEAVE`, register maps refused above `REGISTER_THRESHOLD` (256).
No rule for `BIND_STORAGE` (dual-port), `STREAM depth`, `INTERFACE`, `DEPENDENCE`.

## 5. Agent — `hlsfactory_agent/translate.py` (Pi in Docker)
Receives the pre-passed code, the residue list, the target's rule text and a driver example; fixes only the
residue, fills `# UNRESOLVED` directive lines, writes `translation_report.md`. Up to `attempts` tries with a
retry prompt built from check failures. Never rewrites `run.tcl` from scratch.

## 6. Static checks — `translate.check_translated_design`
Independent of the agent's claims: required files, no leftover Vitis identifiers, top marker, driver has the
required commands, every rendered directive present verbatim, no `# UNRESOLVED` left, report covers every
pragma kind from the scan.

## 7. Container checks — `translate.run_container_checks`
Compile each file (clang, `-std=c++17`), build and run the testbench with a timeout, compare its output
line by line with the original design's testbench output.

## 8. Synthesis gate — `hlsfactory_agent/synth.py`, `exp/run_translate/synth.py`
Run the target tool on the output (`TargetSpec.synth_*`), parse latency/throughput/area into `SynthResult`,
classify failures (`FAILURE_CLASSES`), record how many resources each directive hit. Runs after the
pipeline (`exp/run_translate/synth.py` over finished runs); its result does not feed the retry loop.
There is no recovery stage: a design that fails the gate is reported with its failure class.

## Targets
| | Catapult | XLS (xlscc) |
|---|---|---|
| spec | `targets/catapult.py` | `targets/xlscc.py` |
| tool on chao-srv1 | Catapult Prime 2026.2, license `1717@ece-winlic` | xlscc not built (needs Bazel from source) |
| stages exercised | 0-9 | 0-7 |
