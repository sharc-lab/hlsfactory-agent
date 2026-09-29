# XLS overview

Source: the XLS docs (google.github.io/xls: tools index, codegen options, xlscc overview) and the repository
github.com/google/xls. Items marked *verify* depend on the version installed on the research server; check them
against `--help` there before relying on them.

Why it matters for us: XLS is our second translation target (`--target xlscc`). It works nothing like Catapult, so
the synth rung and any diagnostics reporter need an XLS-specific shape.

## What XLS is

An open-source HLS toolchain from Google. Catapult is one tool that takes C++ plus a Tcl script and writes its own
reports. XLS is a chain of small command-line tools linked by an intermediate representation (XLS IR, text files
ending in `.ir`). No single tool produces a Catapult-style latency, throughput, and area report.

Two frontends produce IR:

- **xlscc** (XLS[cc]): C++ in, IR out. This is the one we use, since our designs start as Vitis C++.
- **DSLX**: XLS's own Rust-like language, converted with `ir_converter_main`. Not relevant unless we ever hand-write
  reference designs.

## The command chain

```
xlscc design.cpp --top <top> > design.ir          # C++ -> IR
opt_main design.ir > design.opt.ir                 # IR optimization
codegen_main design.opt.ir \                       # schedule + Verilog
    --generator=pipeline \
    --delay_model=asap7 \
    --clock_period_ps=1000 \
    --output_verilog_path=design.v \
    --output_schedule_path=design.schedule.textproto \
    --output_signature_path=design.sig.textproto \
    --output_block_ir_path=design.block.ir
```

1. **xlscc** compiles the function marked `#pragma hls_top` into IR. Include paths for its bundled headers
   (ac_int/ac_fixed compatibility headers ship with xlscc) are passed on the command line. *Verify* the exact flag
   name and header location on the server install.
2. **opt_main** runs the optimization pipeline. Useful flags: `--top`, `--opt_level`, `--ir_dump_path` to dump
   intermediate IR between passes.
3. **codegen_main** schedules the IR into pipeline stages and writes Verilog.

### codegen_main flags that matter

| flag | meaning |
|---|---|
| `--generator=pipeline` or `combinational` | pipelined design with registers between stages, or one combinational block |
| `--delay_model` | timing model used for scheduling: `unit`, `asap7`, `sky130` |
| `--clock_period_ps` | target clock period; the scheduler picks the number of stages to fit it |
| `--pipeline_stages` | fix the number of stages instead of a clock period |
| `--flop_inputs`, `--flop_outputs` | register the module boundary |
| `--reset=<name>` plus reset polarity and sync/async options | reset port |
| `--module_name` | name of the generated Verilog module |
| `--use_system_verilog` | emit SystemVerilog (on by default) |
| `--output_verilog_path` | generated Verilog |
| `--output_schedule_path` | textproto: which pipeline stage each IR operation landed in |
| `--output_block_ir_path` | IR after scheduling, with the pipeline registers in it |
| `--output_signature_path` | textproto: ports, channels, and pipeline info of the generated module |

`--delay_model=unit` treats every operation as the same delay. It shows whether the flow completes and says nothing
about timing. Our generated driver (`render_xlscc_driver` in `hlsfactory_agent/targets/xlscc.py`) uses
`--delay_model=unit --pipeline_stages=1`, so its results carry no timing information.

## Pragmas (xlscc)

| pragma | placement | meaning |
|---|---|---|
| `#pragma hls_top` | line before the function definition | the top-level function to synthesize; exactly one |
| `#pragma hls_unroll yes` | line before the `for` | fully unroll the loop |
| `#pragma hls_pipeline_init_interval N` | line before the `for` | pipeline the loop with initiation interval N |

Notes:

- Placement matches Catapult (before the loop), unlike Vitis (inside the loop body). This is why our pre-pass
  reuses the Catapult placement logic for XLS.
- Our target spec treats partial unroll (`UNROLL factor=n`) as lossy and emits `hls_unroll yes`. *Verify* whether
  the installed xlscc accepts a numeric factor.
- II=1 is the reliable case; larger II values may be ignored.
- No equivalents for `ARRAY_PARTITION`, `DATAFLOW`, `INTERFACE`, `BIND_STORAGE`: XLS decides memories versus
  registers itself. We drop these and record them in the report.
- xlscc has further pragmas beyond these three; check the installed version's documentation before relying on any.

## Types and channels

- Arbitrary-precision integers and fixed point: `ac_int<N, signed>` and `ac_fixed<W, I, signed>`, through
  xlscc's ac-compatible headers. Same mapping as Catapult, which is why both targets share `ac_types_include`.
- Streams: `__xls_channel<T>`, with `read()` and `write()`. The type is provided by xlscc itself, not a header.
  For host-side testbenches we compile against `xls_emu.h` (`hlsfactory_agent/xls_emu_include/`), a small
  emulation that is empty under xlscc.
- Channel direction and top-level port wiring: older xlscc versions needed a separate block definition file
  (textproto, `--block_pb`) describing the top's channels; newer versions can infer it, including from a class
  whose method is marked top. *Verify* which form the server install expects; it decides what our driver must
  generate for any streaming design.

## Unsupported in C++

- Floating point (`float`, `double`). Our feasibility pre-check rejects these before any model call
  (`blocking_types` on the `XLSCC` target).
- Pointers and function pointers; rewrite array accesses with indexing.
- Virtual methods.
- Dynamic memory and recursion.
- Loops with no pragma: must be fully unrolled with a constant bound, or pipelined.

## Diagnostics and reporting

| tool | what it gives |
|---|---|
| `benchmark_main <ir>` | total delay, critical path, pipeline stage information, codegen information, optimization time. The closest thing to Catapult's `rtl.rpt`. |
| `delay_info_main <ir>` | delay of each operation and the critical path |
| `ir_stats_main <ir>` | summary statistics of the IR, such as operation counts |
| `print_bom` | bill of materials from signature textprotos, with CSV export |
| codegen `--output_schedule_path` | which stage each operation is in |
| codegen `--output_signature_path` | ports, channels, and pipeline depth of the result |

Simulation and debugging: `eval_ir_main` evaluates an IR function on given or random inputs, `eval_proc_main`
simulates procs (channel-based designs) for a number of cycles, `simulate_module_main` runs the generated Verilog in
a Verilog simulator, and `ir_minimizer_main` shrinks IR to a minimal reproducer.

**Area:** XLS does not report real area. The usual route is Yosys on the generated Verilog for cell and flop counts,
optionally with OpenSTA or OpenROAD against a cell library such as ASAP7 or SKY130 for timing and area.

## Comparing with Catapult

| | Catapult | XLS |
|---|---|---|
| run | one tool, `catapult -shell -file run.tcl` | chain: xlscc, opt_main, codegen_main |
| scheduling target | clock period in `run.tcl` | `--clock_period_ps` or `--pipeline_stages` |
| latency | cycles, from `rtl.rpt` | pipeline stage count from codegen or benchmark_main |
| throughput | cycles, from `rtl.rpt` | II of the pipelined design |
| timing | from Catapult's library | critical path under the chosen delay model |
| area | area score from `rtl.rpt` | none from XLS; Yosys cell count on the Verilog |
| failures | message codes in `catapult.log` (ASM-35, SCHD-20, CIN-15, ...) | xlscc and codegen error text on stderr |

The numbers are not directly comparable. A cross-tool report should present the fields side by side, not merge them.

## Where this repo stands

- Present: `XLSCC` target spec (type, header, pragma maps, blocking types), `xls_emu.h` for host emulation, a
  generated `run_xlscc.sh` driver, translation through emulation checks.
- Missing: synth fields on the `XLSCC` target (`synth_command`, `synth_success_marker`, `parse_report`), a parser
  for XLS output, a real delay model and clock period in the driver, any run of real xlscc on our translations.

## Open questions for the server install

1. Bazel build (binaries under `bazel-bin/xls/...`) or release tarball? Path and version.
2. Include flag for xlscc's bundled ac headers, and where those headers live.
3. Block definition file or inferred channels for streaming tops.
4. Does `hls_unroll` accept a numeric factor?
5. Is Yosys installed, for area?
