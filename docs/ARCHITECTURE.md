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
- Script repair: `translate.normalize_vitis_script` then sets the Tcl variables the script uses but never
  defines (to the design name), adds `-I[pwd]/<dir>` for each folder that resolves an otherwise unresolvable
  quoted include (plus `-DHLSLIB_SYNTHESIS` when the design carries hlslib), and drops options 2024.1 removed.
- Extraction repair (`hlsfactory_agent/extraction_repair.py`, `run_repo.py --repair-extraction --repo-clone DIR`,
  log in `<design>/extraction_repair.json`), in place on the design before the oracle: `repair_includes` points a
  quoted include that escapes the folder (`../../include/x.h`) at the copy the extraction placed next to the
  includer, else recovers the file from a clone of the repository (by path suffix; several hits resolved by
  content, then the design name in the path); `repair_data_files` recovers data files named in string literals
  the same way (a leading `/` joins the design's `DATA_DIR` define). Tool and system headers are left alone.
- Standard includes (`extraction_repair.repair_std_includes`): a file using `uintN_t`, `INT_MAX`-style limits or
  `std::stringstream` without the header gets `<stdint.h>`, `<limits.h>` or `<sstream>` prepended; the original
  had them transitively, Vitis compiles each unit alone (BitBlender MurmurHash3.h, ThunderGP l2.h).
- `ap_cint.h` (`extraction_repair.repair_ap_cint`): Vitis 2024.1 dropped the C integer types, so a design
  including it gets the harness stand-in next to its sources and its `.c` units become `.cpp` (in `synth.tcl`
  and includes too), as the oracle already compiled them (CLINK).
- Two `set_top` lines (ThunderGP: two kernels in one script): `normalize_vitis_script` keeps the last, as Vitis
  does, and marks the unit defining the dropped top `-tb` so the headers' function bodies reach llvm-link once.
- Oracle sources (`extraction_repair.oracle_sources`): the units `run_oracle_check` compiles are the ones
  `synth.tcl` lists (`add_files`, `-tb`, `-cflags`/`-csimflags` kept as `-I`/`-D` flags, `[pwd]` the design dir),
  minus a source another unit `#include`s and minus extra `main`s (the `-tb` one wins); the script-less fallback
  is every top-level source. When separate compilation fails on duplicate symbols the units are built once more
  as one translation unit, testbench last (`build_mode: unity`). `ap_cint.h` (retired C types) has a stand-in
  in `vitis_hls_include/`.
- Fixed seed (`translate.seed_random_sources`): the oracle's copy and the agent's `input_design` copy both get
  `std::random_device` and `srand(time(..))` replaced by `FIXED_SEED`, so an original that draws random inputs
  (DP-HLS) prints the same lines as its translation; the extracted design itself is untouched (`oracle.json`
  records `seeded`).
- Verdict: the oracle passes on exit code 0 unless a line of its output is a bare FAIL verdict (`FAIL`,
  `*** Test FAILED ***`; `translate.oracle_fail_line`, recorded as `fail_line`). Spector's matrix_mul_int prints
  `FAIL` and returns 0. The oracle, the check and the agent's own run share one budget, `TB_RUN_TIMEOUT_S`
  (120 s; the check's 60 s cut matrix_mul_float's 1024^3 float product short).
- Run layout: `run_repo.py` puts each repository under `<runs-dir>/<owner__repo>/` (extraction, run dirs,
  `.synth_cache`, `stats`), so two repositories with a design of the same name (fft, viterbi) never share a run dir;
  `--skip-extract` still finds an extraction one level up from the older flat layout.
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
- `exp`/`log`/`pow` come from a local `ac_float_exp.h`: hyperbolic CORDIC (`ac_math/ac_hcordic.h`), which takes
  the float types directly (~2e-7 worst error on float32). `tan`/`atan` stay residue.
- `hls_math.h` becomes `<cmath>` (both C++ targets). A copied Vitis library header (`_AP_W` template parameters)
  is not copied; its includes already map to the target's types.
- `abs(` becomes `cat_abs(` from a local shim (the C library `abs` has no body for synthesis, CIN-16).
- Byte-level idioms Catapult rejects (CIN-71) map to typed helpers in a local `cat_bytes.h`: `memcpy` ->
  `cat_memcpy` (an element loop between arrays of one type, a bit pack or unpack between an array and wide
  `ac_int` words at a stride of `8*sizeof(T)`), `memset(p, 0, n)` -> `cat_memset0`, `*reinterpret_cast<V*>(p) = v`
  -> `cat_store<V>(p, v)` and the read form -> `cat_load<V>(p)`; a `void*` parameter becomes a template
  parameter so the copy sees the element type. Any left in the DUT fails the checks.

Catapult dialect rules — channels, interfaces and files:
- Local `ac_channel`s made `static`. A local channel declared with `hls::stream`'s name argument, or as an
  array, is made `static` and the name argument removed, also when its template argument holds a macro call or a
  `#define` declares it. A channel member of a struct or class is left alone.
- An unsized top parameter `T x[]` (or `x[/*expr*/]`), or a pointer port `T *x`, gets the size of the array the
  testbench passes at the call site (a static array, a local `T a[expr]`, `malloc(n * sizeof)`,
  `malloc(sizeof(T) * n)`, `calloc(n, ..)`, `new T[n]`, or `malloc(n)` for a byte element), with `n` resolved
  through the testbench's defines (backslash continuations joined, `/* */` stripped, `/` as C integer division),
  `constexpr`/`const` integers and local `int` assignments, including a chain of them (`int w = (D + 15) / 16;
  int total = w + N * w;`) as long as the local is never written again (two test functions may declare it alike).
  A function the top calls with a sized port gets the size on its own `T *x` parameter (`_size_callee_params`:
  a block's pointer parameter is otherwise 1024 words, CIN-84). A pointer port
  becomes `T x[n]` in the definition (the same type after decay). Without a size Catapult assumes 1024 words and
  cannot reduce it for a runtime index (CIN-84).
- `log2(N)` and `ceil(log2(N))` with a constant `N` in the DUT become the number (`_fold_log2`): Catapult has no
  synthesizable body for the builtin (CIN-16, Spector's `#define Mlog log2(M)`); a variable argument is residue.
- The one source file defining `main()` when there is no `testbench.cpp` (`add_files -tb k_test.cpp` kept its
  name) is copied as `testbench.cpp`, so the sizing above and the checks find it; a second `main()` next to
  `testbench.cpp` is not copied (it cannot be linked).

`DATAFLOW`:
- Every function called from the region whose parameters are channels, scalars or ports of the region gets
  `#pragma hls_design block` before its definition. A port of the region may be owned by one block (ECASLab's
  `load_input(in1, ..)` / `store_result(out, ..)`); a top whose stages are not all blocks fails with "logic
  mixed with interconnect".
- A stage that receives a local array, or a port another stage also receives, returns a value, or carries its
  own `INLINE` stays inline, since Catapult blocks may share nothing but channels (ASM-35). Comments are blanked
  before the scan (a doc comment's `in1 (input) --> ..` used to pass for a function).
- A region may be a macro that expands to the pragma (`#define DF PRAGMA( hls dataflow )`), and stages may be
  defined in any design file.
- A parent of blocks may hold only the calls and static channels (ASM-59/62). A loop around the region is not
  moved mechanically; the residue note tells the agent to push it into the stages.
- A stage called under an `if`/`else`/`while`/`do`/`switch`, or a `for` whose bound is a run-time value
  (`_conditional_call`), stays inline: Catapult rejects a conditional call to a sub-block (ASM-59, SERI's
  `for (i_n < n)` around `merge_stream`). A `for` with a constant or macro bound keeps its blocks. A region
  that is itself called conditionally (PQC's `if (kem_cfg == 0) sign(..) else verify(..)`) keeps no blocks:
  inlined into the top, each stage call would be conditional.
- A block called n times in its region (ECASLab's `load_input(in1, ..); load_input(in2, ..)`, PQC's
  `duplicate<uint8_t>`) loses its block marker and gets n wrapper blocks `stage__k` that call it, the k-th
  call renamed to its wrapper (`_instance_blocks`): Catapult allows one call per block with channel I/O
  (HIER-43), so the stage inlines into each wrapper (a block wrapper calling a block still instantiates it
  twice). A template stage's wrapper repeats its template head.

Left as residue / not handled:
- Other float math, `.range(hi, lo)` with two different bounds, and the testbench's type boundary.
- No rule for fixed-point conversion.

Per-target forms: `PRAGMA_FORMS` spells the loop and inline pragmas (Catapult `hls_pipeline_init_interval` /
`hls_unroll` / `hls_design inline`; Bambu `#pragma HLS unroll [factor=n]` / `#pragma HLS inline`, no loop
pipelining, so `PIPELINE` is dropped) and `DIALECT_TYPES` the type substitutions (Bambu keeps native
`float`/`double`; only `std::complex` becomes `ac_complex`; Dynamatic maps `ap_int<N>` to `int8/16/32_t` or
`_BitInt(N)` and `hls_math.h` to `<math.h>`, pipelines and inlines are dropped, unroll becomes
`#pragma clang loop unroll_count(n)`, and every print in the test program moves to stderr because the tool reads the program's stdout as the profiling vectors). The float shims, `cat_abs`, static local channels,
dataflow blocks and the top marker are Catapult-only; sizing unsized parameters applies to both. Bambu-only:
a loop label moves to its own line above the pragma (Bambu wants the loop right after it), and a top that
returns a class by value becomes `void top(..., T &ret_out)` with its `return x;` rewritten (Bambu aborts on
the hidden return pointer); the testbench call sites are residue for the agent.

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
- `run.tcl` sets `Input/CppStandard c++14` (relaxed `constexpr`, CRD-2388 under the c++11 default).
- After each attempt, before the checks: a file missing a required include gets it on top (the pre-pass rule,
  applied to files the agent wrote or edited).
- Fence repair: when the checks fail with fenced edits, the pre-pass text is put back mechanically, at no agent
  cost: all hunks at once, then file by file, then hunk by hunk (at most `FENCE_REPAIR_ROUNDS` single-hunk
  check rounds), keeping every restore whose compile and testbench results are no worse. A hunk whose restore
  makes them worse shows the pre-pass text is not legal in that design, so it is licensed as residue (a line-0
  item licenses a removed file) and the agent's edit stands. The attempt log records `fence_repair` with the
  hunks restored and licensed. Missing headers are licensed up front too: the pre-pass compile check reads
  clang's `fatal error` lines as well as `error` lines.

## 6. Static checks — `translate.check_translated_design`
Independent of the agent's claims.

Whole design:
- Required files present.
- No leftover Vitis identifiers; top marker present.
- Driver has the required commands; every rendered directive present verbatim; no `# UNRESOLVED` left.
- Report covers every pragma kind from the scan.

DUT-only (testbench and testbench-only headers exempt):
- No `tan`/`atan` calls, no `std::complex`, no conversion back to native float (`.to_float()`, `(float)`), whose
  arithmetic Catapult rejects (CIN-290).
- Local channels static; required includes present (comments ignored).

Fence (DUT diffed against `prepass_design`):
- A changed or deleted pre-pass line fails the check unless a residue item licenses it: within 3 lines of one,
  or the line itself is a residue trigger (comment, forbidden pattern, non-literal pragma argument, float math,
  vendor method). A replace that maps line for line is judged per line.
- Testbench code is not fenced: `testbench.cpp` and every header only it reaches through quoted includes
  (`translate.testbench_files`). The same set is exempt from the DUT-only rules, in the checks and the pre-pass.
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
  - Bambu from an extracted AppImage (`BAMBU_APPDIR`) at `/opt/panda/bambu`, run through its `AppRun`
    (`$BAMBU` in the driver), which puts the bundled clang, ac_types and soft libm on the path;
  - Dynamatic is the exception: it only ships as a from-source build, so `ToolInstallation.image` names its
    own image (`DYNAMATIC_IMAGE`, default `hlsfactory/dynamatic:latest`, tree at `/opt/dynamatic`) and
    `run_synth` runs there instead of in `hlsfactory-agent`;
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
- Source: `targets/vitis.py` is a synth-only spec (Vitis HLS 2024.1, 900 s budget: in census3 every passing run took under 4 min and the 14 that ran 40 min all timed out); the `csynth.rpt` top-level
  row is parsed into latency, interval, BRAM/DSP/FF/LUT.
- Target: the Catapult spec, with `FAILURE_CLASSES` matched on the log.
- `exp/run_translate/synth.py` re-runs the target gate over finished runs.
- Synthesis results do not feed the agent's retry loop.

Recovery (`run_synth_degraded`, `run.py --degrade N`, default 2 rounds):
- After a FAILED or TIMEOUT target run, the target's `degrade_step` copies the failed run's sources with one edit the log
  itself parameterizes and synthesizes again.
- Rounds are recorded under `synth_target_degraded`, never merged into `synth_target`.
- Catapult steps (`catapult_degrade_step`):
  - SCHD-87's minimum II raises every tighter `hls_pipeline_init_interval`; so does a failed feedback loop
    (SCHD-85: II x delay / budget, rounded up).
  - SCHD-4's port shortfall splits the array SCHD-39 names with `INTERLEAVE ceil(needed/have)`, compounded on
    a repeat round; the array is the last scope segment of the name (`Align::f:dp_mem.data:rsc` is `dp_mem`).
    A row-wise `BLOCK_SIZE` split for 2-D arrays rescued none of the 6 designs it ran on in census3 and is gone.
    The `resource_directive` proc is added when `run.tcl` has none.
  - An array already split (`INTERLEAVE` or `BLOCK_SIZE`, carried or added) that is still short collides in one
    bank (strided or data-dependent index): every pipelined loop that indexes it gets its II multiplied by
    `ceil(needed/have)` instead.
  - "Cannot pipeline a fully unrolled loop" names the loop's file and line; the pipeline pragma above it is
    removed.
  - SIF-4 "multicycle component prohibited" (a 32-bit divider, blaze) adds `directive set -SCHED_USE_MULTICYCLE true`.
  - MEM-71 "connected to multiple sub-blocks not mapped to [DirectInput]" names the scalar port's resource path
    (ECASLab elementwise's `size`, read by every block), which is mapped `-MAP_TO_MODULE {[DirectInput]}` after
    `go assembly` (`direct_input:<port>`).
  - A run that hit its time budget (`synth_result.json`, written by `run_synth`) loses its partial
    `hls_unroll N` pragmas, then on a repeat its `hls_unroll yes` ones (`unroll_cap:partial|full:N`).
  - A minimum II above `MAX_RELAXED_II` (8), or a schedule failure the log does not parameterize (SCHD-26),
    doubles the clock period once (`clock:10`); the loop-carried chain does not fit 5 ns.
  - Last resort, once the unroll caps or the clock are spent: every `hls_pipeline_init_interval` pragma goes
    (`unpipeline:all:N`), so the loops run sequentially (HIDA's 16-way replicated float bodies keep the
    architect step busy past 2400 s; a read-modify-write through memory never schedules at any II).
- Any other failure class ends the loop with its class reported.

## 9. Run statistics — `hlsfactory_agent/stats.py`, `exp/run_translate/stats_report.py`
- `run_repo.py` times each design's stages (`row["stage_s"]`: oracle, translate, synth_source, synth_target,
  recovery), keeps the agent's attempt list with per-attempt agent and check seconds (`row["attempts"]`), samples
  the host every 30 s (load, memory, running pipeline containers) and writes `<runs>/stats/<repo>.json` and
  `.resources.jsonl` per repository (wall, jobs, attempts, extraction time or reuse, host, resource summary).
- `stats_report.py` reads those plus the results rows and writes `docs/RUN_STATISTICS.md`: method, per-repo
  time and resources, time per stage, the retry curve (first pass per attempt, rescued share per retry, optional
  baseline pass), failure-class transitions between attempts, thrashing designs (identical failures every
  attempt), synthesis and recovery counts.

## Targets
| | Catapult | XLS (xlscc) | Bambu (PandA) | Dynamatic (EPFL) |
|---|---|---|---|---|
| spec | `targets/catapult.py` | `targets/xlscc.py` | `targets/bambu.py` | `targets/dynamatic.py` |
| tool on chao-srv1 | Catapult Prime 2026.2, license `1717@ece-winlic` | xlscc not built (needs Bazel from source) | Bambu 2024.10 AppImage, extracted under `/export/hdd/scratch/tshukla31/tools` | built into the image `hlsfactory/dynamatic` (`docker_images/Dockerfile.dynamatic`, `main` of 2026-10-25, prebuilt LLVM, CBC solver, GHDL) |
| stages exercised | 0-8 | 0-7 | 0-8 (no directives) | 0-8 (no directives, no fence: the pre-pass output is still C++) |
| driver | `run.tcl` | `run_xlscc.sh` | `run_bambu.sh` (`--top-fname`, `--generate-interface=INFER`, `-lm`) | `run_dynamatic.dyn` (`set-src /work/<top>.c`, `compile --buffer-algorithm fpga20 --milp-solver cbc`, `write-hdl --hdl vhdl`, `simulate --simulator ghdl`) |
| PASS means | `rtl.rpt` written | not run | `<top>.v` written; area/DSP/FF from the log, no latency (no simulation) | `out/hdl/<top>.vhd` written and the GHDL C/RTL co-simulation matches; latency = simulated cycles |
| recovery | relax II, unpipeline, block/interleave | none | `--generate-interface=INFER` -> `MINIMAL` after `unexpected pattern` | `on-merges` buffering after a profiling or MILP failure; `compile --disable-lsq` when the log names the LSQ |

Dynamatic is plain C with one file: the kernel `<top>.c` holds `main()`, which calls the kernel through
`CALL_KERNEL` so the tool can dump co-simulation inputs (the harness's host stand-in for
`dynamatic/Integration.h` is `hlsfactory_agent/dynamatic_include`). `TargetSpec.language="c"` switches the
compile checks to `clang -std=c17` for the syntax check and `clang++ -x c++` for the testbench build (the tool
compiles the same file both ways), drops `testbench.cpp` from the required files and rewords the prompt;
The pre-pass never rewrites inside a string literal (`rewrite.outside_strings`), and the static check
`testbench_strings_kept` fails an output whose testbench lost a literal of the input's testbench (a Vitis type
name inside a message, a "fixed" escape): the outputs are compared line by line, so printed text must stay.
`compare_outputs` drops the ap_int library's runtime `WARNING: Bitsize mismatch` lines with the simulator noise.
Catapult's `run.tcl` adds sources by their path inside the design (`device/k.cpp`), not the bare file name.

`fence=False` leaves the agent free to collapse the C++ files into the one C file. The kernel rules the
prompt carries (no calls, no local arrays, no pointers or globals, no 64-bit types, `_BitInt(N)` for other
widths, `#pragma clang loop unroll_count(n)`, `#pragma DYN array_partition`) come from Dynamatic's
`KernelCodeGuideLines.md`. Designs with `ap_fixed`, `hls::stream` or `hls::vector` are blocked before the
pre-pass (`blocking_types`). `TargetSpec.extra_checks` holds target-specific static checks; Dynamatic's requires exactly
one `CALL_KERNEL`, naming the function the kernel file is named after, since the co-simulation records a single
transaction (lstm_n called it three times and failed only after a full synthesis).
