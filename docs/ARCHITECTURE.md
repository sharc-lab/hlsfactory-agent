# Architecture

Automatic translation of standalone HLS designs between tools. Vitis is always the source; Catapult and
XLS are the targets. This file describes the pipeline as it is in the repo now: no history, no plans.
Updated 2026-09-26.

Three paths, driven by `exp/run_translate/run.py`:
- **Path A** always runs.
- **Path B** runs unless `--no-translate`.
- **Path C** runs only with `--synth` (off by default, so a run stops at compile) and is cached.

```
A  extract (0) -> scan (1) -> feasibility (2)                       Vitis design, checked, inventoried
B  pre-pass (3) -> resource channel (4) -> agent (5) -> static checks (6) -> container checks (7)
C  synthesis (8): Vitis on the source, target tool on the translation      --synth both|source|target|none
```

Per design the run record keeps three separate outcomes, never merged:
- `checks_passed` (path B)
- `synth_source` and `synth_target` (path C; `{"status": "not run"}` when switched off)

With `--require-source-synth`, a design whose Vitis synthesis did not pass is not translated.

## 0. Extraction — `hlsfactory_agent/core.py`
- An agent (Pi, via OpenRouter) in the `hlsfactory-agent` container pulls standalone Vitis designs out of a
  GitHub repo. Vitis-only; the upstream HLSFactory-Agent project.
- Output: one folder per design with sources, `testbench.cpp`, data files, `synth.tcl`.
- Top repair: before anything else both drivers call `translate.repair_top`. When `set_top` names a function
  that is not defined (extraction writes the folder name), and exactly one DUT function is called from the
  testbench, `set_top` is rewritten to it.
- `exp/run_translate/import_dataset.py` produces the same layout from HLSFactory's built-in dataset folders
  (`dataset_hls.tcl`); designs whose testbench spans several source files are skipped.

## 1. Scan — `hlsfactory_agent/scan.py`
Read-only inventory of one design, written to `scan.json`:
- Every pragma, vendor type and header.
- For each array a resource pragma names: argument vs local, all dimensions, total size, with `#define`
  arithmetic resolved.
- An `arrays` inventory of every array declared in the DUT (name, dims, size, scope, const).
- Limitation: does not fold conditionals (a size like `N<2 ? 1 : N/2` stays unknown).

## 2. Feasibility — `translate.feasibility`
- Checks scanned types against the target's `blocking_types`; rejects before any model spend.
- XLS blocks `float` and `double`; Catapult blocks nothing today.

## 3. Mechanical pre-pass — `hlsfactory_agent/rewrite.py`
Lookup-table conversion without judgment. Anything uncertain goes on `rewrite_log.json`'s `residue` list. The
output is snapshotted as `run_area/prepass_design` before the agent runs.

General rules:
- Headers and types converted, with widths and rounding modes preserved.
- `PIPELINE`/`UNROLL`/`INLINE` renamed and moved to the target's placement:
  - an `INLINE` goes before the first line of a signature that spans lines;
  - a `PIPELINE` on a fully unrolled loop is dropped (Catapult refuses the pair);
  - a macro II or factor is resolved from its `#define`, since target pragmas are not macro-expanded.
- Unsupported pragmas dropped with a reason; `#pragma SDS` lines dropped.
- Top marker inserted.
- Residue entries carry the line's text and are re-pointed at the output file after insertions, so the fence
  licenses the right lines.

Catapult dialect rules — types and literals:
- `std::complex` -> `ac_complex`; `float`/`double` -> `ac_ieee_float32/64`.
- Literal tables stay native: a float array with a literal initializer stays native whether or not it is `const`.
- `ac_ieee_floatNN v = <literal>;` becomes direct initialization (the constructors are explicit, CRD-415); a
  typedef'd float type gets the same rule.
- A `constexpr float/double` stays native (the float types are not literal types).
- `ac_ieee_float64 v = <native arithmetic>;` gets every operand wrapped in the float type (binary32
  declarations are residue: precision would change).
- Single-bit `x.range(i, i)` becomes `x[i]`.

Catapult dialect rules — headers and math:
- `<stdint.h>` and `<ac_std_float.h>` inserted where needed.
- `hls_vector.h`, `hls_fft.h`, `hls_streamofblocks.h` are written next to the design as local stand-in headers
  (`hls::vector` is a plain struct there).
- `cos`/`sin`/`sqrt` on the float types come from a local `ac_float_trig.h`: fixed-point CORDIC from `ac_math` in
  the Catapult install (`$MGC_HOME/shared/include`, mounted into the container as `catapult_include`; nothing
  from the tool is vendored, ~1e-9 error). `std::cos` is unqualified so that overload is found.
- `abs(` becomes `cat_abs(` from a local shim (the C library `abs` has no body for synthesis, CIN-16).

Catapult dialect rules — channels, interfaces and files:
- Local `ac_channel`s made `static`. A local channel declared with `hls::stream`'s name argument, or as an
  array, is made `static` and the name argument removed.
- An unsized top parameter `T x[]` gets the size of the array the testbench passes at the call site (a static
  array, `malloc(n * sizeof)` or `new T[n]`), with `n` resolved through the testbench's defines and local `int`
  assignments. Without it Catapult assumes 1024 words and fails (CIN-84).
- A second source file defining `main()` next to `testbench.cpp` is not copied (it cannot be linked).

`DATAFLOW`:
- Every function called from the region whose parameters are channels or scalars gets
  `#pragma hls_design block` before its definition.
- A stage that receives an array or carries its own `INLINE` stays inline, since Catapult blocks may share
  nothing but channels (ASM-35).

Left as residue / not handled:
- Other float math, `.range(hi, lo)` with two different bounds, and the testbench's type boundary.
- No rule for fixed-point conversion.

## 4. Resource channel — `hlsfactory_agent/targets/catapult.py`, `catapult_directives.json`
- `ARRAY_PARTITION`, `BIND_STORAGE` and `STREAM` pragmas become lines in `run.tcl`; `directives.json` records
  rendered and unrendered (with reason).
- Paths are looked up, not predicted: `run.tcl` defines a `directive_on` Tcl proc that, after `go assembly`,
  globs Catapult's real paths and applies the directive to every match:
  - `:rsc` memories via `resource_directive`;
  - `:cns` channels via `channel_directive`;
  - nested per block, one per struct field.
- Partition rules are dimension-aware:
  - outer-dim split -> `BLOCK_SIZE`;
  - innermost -> `INTERLEAVE`;
  - register maps refused above `REGISTER_THRESHOLD` (256).
- `BIND_STORAGE type=ram_1p|ram_s2p|ram_2p|ram_t2p` -> `MAP_TO_MODULE` on the `ccs_sample_mem` single-port,
  1R1W or dual-port module (`impl=` is ignored).
- `STREAM depth=n` -> `FIFO_DEPTH n` on that channel.
- Dual-port default: every scanned array above `MEM_MAP_THRESHOLD` (32 words) with no `BIND_STORAGE`, not const
  and not register-mapped is mapped to `ccs_ram_sync_dualport`, the memory Vitis gives arrays implicitly.
  Catapult's default `ccs_ram_sync_1R1W` has one read port and fails two-reads-per-cycle schedules (SCHD-4).
- `run.tcl` sets `FIFO_DEPTH 8` globally: a channel between inlined functions has no legal unbuffered module
  (HIER-47).
- No rule for ROM binding, `INTERFACE`, `DEPENDENCE`.

## 5. Agent — `hlsfactory_agent/translate.py` (Pi in Docker)
- Receives the pre-passed code, the residue list, the target's rule text and a driver example.
- Fixes only the residue, fills `# UNRESOLVED` directive lines, writes `translation_report.md`.
- Up to `attempts` tries (`run.py --attempts`), with a retry prompt that lists the failing lines and, for edits
  outside the residue, the pre-pass text to restore.
- Never rewrites `run.tcl` from scratch.

## 6. Static checks — `translate.check_translated_design`
Independent of the agent's claims.

Whole design:
- Required files present.
- No leftover Vitis identifiers; top marker present.
- Driver has the required commands; every rendered directive present verbatim; no `# UNRESOLVED` left.
- Report covers every pragma kind from the scan.

DUT-only (testbench exempt):
- No float math calls, no `std::complex`.
- Local channels static; required includes present.

Fence (DUT diffed against `prepass_design`):
- A changed or deleted pre-pass line fails the check unless a residue item licenses it: within 3 lines of one,
  or the line itself is a residue trigger (comment, forbidden pattern, non-literal pragma argument, float math,
  vendor method).
- Before the agent runs, every pre-pass source is compiled in the container and each line the compiler rejects
  is added to the residue, so the fence never blocks a fix the pre-pass itself made necessary.
- Insertions are free.

## 7. Container checks — `translate.run_container_checks`
- Compile each file (clang, `-std=c++17`).
- Build and run the testbench with a timeout.
- Compare its output line by line with the original design's testbench output.
- Printed numbers within a 1e-4 relative/absolute tolerance count as matching and are reported separately
  (`close_within_tolerance`, `max_relative_difference`).

## 8. Synthesis — `hlsfactory_agent/synth.py`, `targets/vitis.py`
Running the tool (`run_synth_cached`):
- Copies a design to a work dir and runs the tool described by its `TargetSpec.synth_*` fields in a fresh
  container.
- The host install named by `TargetSpec.tool` is bind-mounted read-only (`hlsfactory_agent/tools.py`):
  - Catapult from `MGC_HOME` at `/opt/siemens/catapult`;
  - Vitis HLS plus its matching Vivado at `/opt/amd/hls` and `/opt/amd/vivado`;
  - licenses passed as environment.
- The work dir is mounted at `/work`; the process is owned by the calling user.
- The agent never shares this container; `run.py --agent-tools` (off by default) is the only way a tool reaches
  the agent.
- A missing install is a `tool-missing` failure, not a crash.
- The report is parsed into `SynthResult` (status, latency, throughput, area or per-tool `resources`, failure
  class, directive hits).
- Results are memoized under `<runs>/.synth_cache/<tool>-<content hash>.json`, so an unchanged design is never
  synthesized twice.
- `run.py --synth-timeout` overrides the target's budget (Catapult default 1200 s; a pipelined 1024-point FFT
  needs 25-30 min to reach a scheduling verdict).

Source and target sides:
- Source: `targets/vitis.py` is a synth-only spec (Vitis HLS 2024.1); the `csynth.rpt` top-level row is parsed
  into latency, interval, BRAM/DSP/FF/LUT.
- Target: the Catapult spec, with `FAILURE_CLASSES` matched on the log.
- `exp/run_translate/synth.py` re-runs the target gate over finished runs.
- Synthesis results do not feed the agent's retry loop.

Recovery (`run_synth_degraded`, `run.py --degrade N`, default 2 rounds):
- After a FAILED target run, the target's `degrade_step` copies the failed run's sources with one edit the log
  itself parameterizes and synthesizes again.
- Rounds are recorded under `synth_target_degraded`, never merged into `synth_target`.
- Catapult steps (`catapult_degrade_step`):
  - SCHD-87's minimum II raises every tighter `hls_pipeline_init_interval`.
  - SCHD-4's port shortfall splits the array SCHD-39 names: a 2-D array gets one bank per row
    (`BLOCK_SIZE` = inner size), a 1-D array `INTERLEAVE ceil(needed/have)`, compounded on a repeat round.
  - "Cannot pipeline a fully unrolled loop" names the loop's file and line; the pipeline pragma above it is
    removed.
- Any other failure class ends the loop with its class reported.

## Targets
| | Catapult | XLS (xlscc) |
|---|---|---|
| spec | `targets/catapult.py` | `targets/xlscc.py` |
| tool on chao-srv1 | Catapult Prime 2026.2, license `1717@ece-winlic` | xlscc not built (needs Bazel from source) |
| stages exercised | 0-8 | 0-7 |
