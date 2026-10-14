# Dynamatic as a fourth target (assessment, nothing implemented)

Dynamatic 2.0 (EPFL-LAP, Apache 2.0, MLIR-based) compiles a C kernel into a dynamically scheduled dataflow circuit
(VHDL/Verilog/SMV). Sources read 2026-10-09: the GitHub README, `docs/UserGuide/{KernelCodeGuideLines,
CommandReference,OptimizationsAndDirectives,Dependencies,Verification,AnalyzingOutputFiles}.md`, the introduction
tutorial, `integration-test/{fir,gemm_float}`, `include/dynamatic/Integration.h`.

## What it is, against the three targets we have

| | Vitis HLS (source) | Catapult | Bambu | Dynamatic |
|---|---|---|---|---|
| scheduling | static | static | static | dynamic (handshake tokens; no II, no pipeline pragma) |
| input | C++ with `ap_*`, `hls::stream` | C++ with `ac_*`, `ac_channel` | C/C++ with ac types or plain C | plain C only (Polygeist front end; clang `_BitInt(N)` for widths) |
| types | arbitrary-width | arbitrary-width | arbitrary-width, native float | 8/16/32-bit ints, `float`, `double`, `_BitInt(N)`; no 64-bit, no `long` |
| streams / channels | yes | yes | FIFO via `ac_channel` | none: every array is one memory port; scalars are tokens |
| top | `set_top` | `hls_design top` | `--top-fname` | the function named like the `.c` file |
| testbench | `main()` returning 0/1 | compiled, never synthesized | compiled, never synthesized | `main()` must call `CALL_KERNEL(f, args)` with variables named like the parameters; drives C/RTL co-simulation |
| loop pragmas | `PIPELINE`, `UNROLL` | `hls_pipeline_init_interval`, `hls_unroll` | `HLS unroll`, no pipelining | `#pragma clang loop unroll_count(n)`; nothing for pipelining (dynamic) |
| memory directives | `ARRAY_PARTITION`, `BIND_STORAGE` | TCL `resource_directive` | none | `#pragma DYN array_partition array= dimension= style= factor=` inside the function body |
| functions | inline or hierarchical | `hls_design block/inline` | inlined | not supported inside the kernel: macros only; no recursion |
| arrays | local arrays, pointers | local arrays, pointers (CIN-71 on casts) | local arrays | no local arrays (pass them as parameters), no pointers, no dynamic memory, no globals (pass them) |
| driver | Tcl | `run.tcl` | `run_bambu.sh` | a command script replayed by `./bin/dynamatic --run <file>`: `set-src`, `compile [--buffer-algorithm fpga20\|fpl22] [--sharing]`, `write-hdl --hdl verilog`, `simulate --simulator xsim`, `synthesize` |
| PASS evidence | `csynth.rpt` | `rtl.rpt` | `<top>.v` | `out/hdl/<top>.v` (compile+write-hdl); `out/sim/report.txt` SUCCESS with cycle count; `out/synth` from Vivado |
| QoR | latency, II, area | latency, throughput, area | area, DSP, FF | cycles (co-sim), Vivado LUT/FF/DSP/period (`synthesize`); without Gurobi, buffers are placed naively and the numbers mean little |
| install | lab | lab | 4.4 GB AppImage | build from source (Ubuntu 24.04 `build.sh --release --use-prebuilt-llvm`, 6 GB disk; Dockerfile in the repo), optional Gurobi (MILP buffer placement), simulation through ModelSim, xsim (we have Vivado 2024.1), GHDL or Verilator |

Similarities: the harness pieces transfer one to one. It is a single-function kernel plus a `main` testbench in one
folder, the top is known, synthesis is a command script run in a container with the install mounted read-only, PASS
is a file on disk plus a report to parse (`out/hdl/<top>.v`, `report.txt`), and failure classes come from the
tool's messages. `TargetSpec` already has every field it needs (`tool`, `synth_command`, `synth_files`,
`synth_success_marker`, `parse_report`, `required_files`, `driver_file`, `top_marker=""` like Bambu).

Differences that matter: (1) it is C, not C++, so `ap_int`/`ac_int`/classes/templates/references have no target;
widths map to `_BitInt(N)` (no 64-bit); (2) no channels at all: every `hls::stream` design is out of scope, and
dataflow regions collapse to one kernel; (3) no functions inside the kernel: everything the top calls must be
inlined into it (macro or textual inlining), which is the one new mechanical transformation; (4) no local arrays:
they become extra parameters allocated in `main`, a signature change the testbench must follow (the same shape as
the Bambu `ret_out` rule); (5) `main` must call `CALL_KERNEL` with variables named like the parameters; (6)
pipelining has no equivalent and `UNROLL` becomes `#pragma clang loop unroll_count(n)` on the line before the
loop; `ARRAY_PARTITION` maps directly to `#pragma DYN array_partition` (the first target where it is not dropped);
(7) the QoR number is a co-simulated cycle count, which needs a simulator in the container.

## How it would sit in the repo

- `hlsfactory_agent/targets/dynamatic.py`: `DYNAMATIC = TargetSpec(name="dynamatic", include_dir=<the
  `include/` of the install, for `dynamatic/Integration.h`>, cxx_std_check="c17" (clang, not g++; the integration
  header refuses gcc), header_map `{ap_int.h: <stdint.h>, ap_fixed.h: none, hls_stream.h: none}`, type_map
  `ap_int<N> -> _BitInt(N)`/`int32_t` family, `ap_fixed -> none` (unsupported; feasibility pre-check rejects),
  pragma_rules (UNROLL -> clang loop unroll_count, PIPELINE dropped/absent, ARRAY_PARTITION -> DYN
  array_partition translated, INTERFACE/DATAFLOW/STREAM dropped), forbidden `#pragma HLS`, `hls::`, `ap_`,
  `class`/`template`, required_files `("testbench.c"?, "run_dynamatic.dyn", "translation_report.md")`,
  driver `run_dynamatic.dyn` (the `--run` script), tool="dynamatic", synth_command `("$DYNAMATIC/bin/dynamatic",
  "--run", "run_dynamatic.dyn")`, marker `out/hdl/*.v`, `parse_dynamatic_report` reading `out/sim/report.txt`
  (SUCCESS, cycles) and `out/synth` if `synthesize` ran, failure classes from the compile log (array flattening,
  unsupported type, non-affine access, Polygeist errors), no degrade step at first (candidates later: drop
  `--buffer-algorithm`, add `--sharing`).
- `hlsfactory_agent/tools.py`: `dynamatic_installation(root)` mounting the built tree read-only at
  `/opt/dynamatic` with `DYNAMATIC=/opt/dynamatic`; `simulate --simulator xsim` needs the Vivado mount we already
  have (`vitis_installation`), so `run_synth` would mount both (a small change: a spec may name a second tool).
- `hlsfactory_agent/rewrite.py`: `DIALECT_TYPES["dynamatic"]` (ap/ac widths to `_BitInt`/stdint, float kept),
  `PRAGMA_FORMS["dynamatic"]` (`unroll` -> `#pragma clang loop unroll_count({v})`, `unroll_full` -> same with the
  trip count or `unroll(full)`, `pipeline` -> None, `inline` -> None), one new rule to hoist local arrays into
  parameters (mirror of `_void_class_return`), one new rule to inline the kernel's callees or mark them as
  residue, `main` rewritten to `CALL_KERNEL`. The testbench is C: `testbench.cpp` becomes `main` inside the
  kernel file (Dynamatic wants one `.c` with both), which touches the fixed `testbench.cpp` convention in
  `translate.py` (required files, `_iter_source_files`, the container checks compile with g++ today; Dynamatic
  needs clang).
- `scan.py` feasibility: a design is out of scope when it has streams, classes, 64-bit types, `ap_fixed`, or
  recursion; that pre-check exists for XLS and would get a Dynamatic profile.
- Docker: a second image or a stage in `docker_images/Dockerfile` with clang 18 (Ubuntu 24.04) for the compile
  check; the Dynamatic tree itself is mounted, not copied, like the other tools.
- Docs: a column in `docs/ARCHITECTURE.md`'s target table, a README section, funnels in `docs/WEEKLY_FINDINGS.md`.

## Expected yield on the sets we have

- Fits: the `.c` kernel families (ProgSG 23, MachSuite-style, CLINK 2, the polybench kernels in ScaleHLS-HIDA,
  S2CBench's C-style ones), the MAC/FFT variants with array ports (n*_no_StagePipeline, original_C_style) after
  the width and inlining rules.
- Does not fit: everything on `hls::stream`/`ac_channel` (the 14 FFT variants, NN2FGPA, pasta, ThunderGP, the
  streaming vadd), everything on `ap_fixed` (fixed-point sqrt, window conv, hamming window), anything with
  64-bit arithmetic, classes or templates (balor's templated kernels, gemm_hls).
- Rough count from the census: about 60 of 181 designs are plain-C or C-like with array ports; that is the
  ceiling before any rule.

## Cost and risk

- Build: one afternoon on the lab machine (prebuilt LLVM path), plus the Vivado xsim mount for co-simulation;
  Gurobi is optional but without it the cycle counts are not comparable to the other tools (naive buffers).
- Translation: the C-only dialect is a larger rewrite than Bambu's (inlining, local-array hoisting, `_BitInt`
  widths, `CALL_KERNEL` main); expect the agent to carry more of it and the fence to catch more.
- Research value: it is the first target where scheduling is not static, so the "synthesizes" count measures
  a different thing (compile + co-sim match rather than a schedule); `ARRAY_PARTITION` finally maps instead of
  being dropped, and the Dynamatic repo ships an XLS bridge (`--experimental-enable-xls`), which is a link to
  Justin's target worth knowing about.

Recommendation: worth doing after the census second pass, scoped to the C-kernel families first (ProgSG,
CLINK, S2CBench C-style, ScaleHLS polybench), with `simulate --simulator xsim` as the PASS gate and
`synthesize` off until Gurobi is settled.
