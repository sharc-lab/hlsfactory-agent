# XLS[cc] Pragma Experiment Results

Everything here was checked by compiling (and, from Exp 6 on, simulating) code with our own XLS build. The pragma mapping tables are in [xls_pragma_notes.md](xls_pragma_notes.md); those come from reading source and docs, so where the two disagree, trust this file. Last updated 2026-10-29.

## Purpose

Each experiment is a small XLS C++ program pushed through xlscc → opt_main → codegen_main (and, where it has test vectors, a cycle-accurate block simulation), to check one Vitis → XLS mapping from the notes. Priority order: `stream`, `pipeline`, `array_partition`, then everything rated ⚠️ or ❓ in the notes.

Exp 1–5 are structural (IR and Verilog only). Exp 6–7 are simulated with `eval_proc_main`. There are no area numbers yet (no `yosys` on `chao-srv1`).

## Experiment index

| # | Experiment | Probes | Result |
|---|---|---|---|
| 1 | [`hls_fifo_depth` on an internal channel](#experiment-1-hls_fifo_depth-on-an-internal-channel) | `fifo_depth_internal_4` | ✅ Depth reaches the IR and Verilog. |
| 2 | [Default depth / explicit depth 0](#experiment-2-default-depth--explicit-depth-0) | `fifo_depth_internal_{default,0}` | ⚠️ **Default depth is 0, not 2 as in Vitis.** Writing `hls_fifo_depth(0)` is an error. |
| 3 | [`hls_fifo_depth` on an external channel](#experiment-3-hls_fifo_depth-on-an-external-channel) | `fifo_depth_external_{in,out}` | ⚠️ Silently ignored. |
| 4 | [Producer → stream → consumer](#experiment-4-producer--stream--consumer) | `stream_two_blocks{,_depth4,_array}` | ✅ Maps to Vitis DATAFLOW + `hls::stream`: two modules joined by a FIFO. |
| 5 | [Plain array vs `__xls_memory` (array_partition)](#experiment-5-plain-array-vs-__xls_memory-array_partition) | `array_{plain,memory}_sum8` | ✅ A plain array is already fully partitioned (registers, all reads in one cycle). `__xls_memory` is a RAM: one access per FSM state. |
| 6 | [Does stream depth matter? (simulated)](#experiment-6-does-stream-depth-matter-simulated) | `stream_two_blocks_burst*`, `stream_two_blocks_reorder*` | ⚠️ Depth only matters when one side must run ahead. **Codegen's port registers add hidden buffering**: depth 0 works with them and deadlocks without. |
| 7 | [Loop and function pipelining](#experiment-7-loop-and-function-pipelining) | `pipeline_loop_{ii1,ii2,nopragma,wct2}` | ✅ Loops run at II=1 (with or without the pragma). II=2 is ignored with a warning. Function-level II=N → codegen `--worst_case_throughput=N`. |

## Probe file map

Naming: `<what is tested>_<where>_<variant>`. Probe `X` = C++ in `probes/X.cc`, plus optional files next to it: `X.textproto` (block spec, `stream_*`), `X.codegen_flags` (extra codegen flags) and `X.sim/` (test vectors: `<channel>.in` / `<channel>.out`). Outputs go in `out/X/`:

| File in `out/X/` | What it is |
|---|---|
| `X.ir` / `X.opt.ir` | Raw / optimized IR. Channel declarations (with `fifo_depth`) are at the top. |
| `X.v` | Verilog. FIFOs are `xls_fifo_wrapper` instances, **not defined** in the file. |
| `X.mat.v` | Self-contained Verilog (`--materialize_internal_fifos`), each FIFO its own module. |
| `X.block.ir`, `X.sig.textproto` | Block IR and signature, the simulator's input. |
| `sim_cycles.txt`, `sim.log` | Simulation result: the cycle of the last output, and the simulator's log. |
| `*.log` | Tool output; check `xlscc.log` when a probe fails. |

`out/` is gitignored; its links only resolve on `chao-srv1` (`/export/hdd/scratch/jzhou652/hlsfactory-agent/`).

| Probe | Exp | Varies | Outputs |
|---|---|---|---|
| [fifo_depth_internal_4](../../exp/xls_pragma_probe/probes/fifo_depth_internal_4.cc) | 1 | internal channel, depth 4 | [out/](../../exp/xls_pragma_probe/out/fifo_depth_internal_4/) |
| [fifo_depth_internal_default](../../exp/xls_pragma_probe/probes/fifo_depth_internal_default.cc) | 2 | no pragma | [out/](../../exp/xls_pragma_probe/out/fifo_depth_internal_default/) |
| [fifo_depth_internal_0](../../exp/xls_pragma_probe/probes/fifo_depth_internal_0.cc) | 2 | `hls_fifo_depth(0)` | [xlscc.log](../../exp/xls_pragma_probe/out/fifo_depth_internal_0/xlscc.log) |
| [fifo_depth_external_in](../../exp/xls_pragma_probe/probes/fifo_depth_external_in.cc) | 3 | pragma on input port | [out/](../../exp/xls_pragma_probe/out/fifo_depth_external_in/) |
| [fifo_depth_external_out](../../exp/xls_pragma_probe/probes/fifo_depth_external_out.cc) | 3 | pragma on output port | [out/](../../exp/xls_pragma_probe/out/fifo_depth_external_out/) |
| [stream_two_blocks](../../exp/xls_pragma_probe/probes/stream_two_blocks.cc) | 4 | no pragma on the stream | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks/) |
| [stream_two_blocks_depth4](../../exp/xls_pragma_probe/probes/stream_two_blocks_depth4.cc) | 4 | depth 4 | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_depth4/) |
| [stream_two_blocks_array](../../exp/xls_pragma_probe/probes/stream_two_blocks_array.cc) | 4 | `int[4]` sent as one item, depth 2 | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_array/) |
| [array_plain_sum8](../../exp/xls_pragma_probe/probes/array_plain_sum8.cc) | 5 | `static int arr[8]`: 1 write + 8 reads per call | [out/](../../exp/xls_pragma_probe/out/array_plain_sum8/) |
| [array_memory_sum8](../../exp/xls_pragma_probe/probes/array_memory_sum8.cc) | 5 | same with `__xls_memory<int, 8>` | [out/](../../exp/xls_pragma_probe/out/array_memory_sum8/) |
| [stream_two_blocks_burst](../../exp/xls_pragma_probe/probes/stream_two_blocks_burst.cc) | 6 | 4 writes / 4 reads per call, no pragma | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_burst/) |
| [stream_two_blocks_burst_depth4](../../exp/xls_pragma_probe/probes/stream_two_blocks_burst_depth4.cc) | 6 | same, depth 4 | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_burst_depth4/) |
| [stream_two_blocks_reorder](../../exp/xls_pragma_probe/probes/stream_two_blocks_reorder.cc) | 6 | burst of 4, then the consumer reads a second channel first; no pragma | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_reorder/) |
| [stream_two_blocks_reorder_depth3](../../exp/xls_pragma_probe/probes/stream_two_blocks_reorder_depth3.cc) | 6 | same, depth 3 | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_reorder_depth3/) |
| [stream_two_blocks_reorder_depth4](../../exp/xls_pragma_probe/probes/stream_two_blocks_reorder_depth4.cc) | 6 | same, depth 4 | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_reorder_depth4/) |
| [stream_two_blocks_reorder_noflops](../../exp/xls_pragma_probe/probes/stream_two_blocks_reorder_noflops.cc) | 6 | no pragma, port registers off | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_reorder_noflops/) |
| [stream_two_blocks_reorder_depth3_noflops](../../exp/xls_pragma_probe/probes/stream_two_blocks_reorder_depth3_noflops.cc) | 6 | depth 3, port registers off | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_reorder_depth3_noflops/) |
| [pipeline_loop_ii1](../../exp/xls_pragma_probe/probes/pipeline_loop_ii1.cc) | 7 | loop of 8 reads, `hls_pipeline_init_interval 1` | [out/](../../exp/xls_pragma_probe/out/pipeline_loop_ii1/) |
| [pipeline_loop_ii2](../../exp/xls_pragma_probe/probes/pipeline_loop_ii2.cc) | 7 | same, II=2 | [out/](../../exp/xls_pragma_probe/out/pipeline_loop_ii2/) |
| [pipeline_loop_nopragma](../../exp/xls_pragma_probe/probes/pipeline_loop_nopragma.cc) | 7 | same, no pragma | [out/](../../exp/xls_pragma_probe/out/pipeline_loop_nopragma/) |
| [pipeline_loop_wct2](../../exp/xls_pragma_probe/probes/pipeline_loop_wct2.cc) | 7 | `_ii1` source + codegen `--worst_case_throughput=2` | [out/](../../exp/xls_pragma_probe/out/pipeline_loop_wct2/) |

## How to run

```sh
cd exp/xls_pragma_probe
./run_all.sh                                                       # build every probe, then simulate those with a .sim/
./run.sh probes/fifo_depth_internal_4.cc --block_from_class Block  # one class-style probe
./run.sh probes/stream_two_blocks.cc                               # one free-function probe (picks up the .textproto)
./sim.sh probes/stream_two_blocks_reorder.cc                       # simulate one probe (after run.sh)
```

Expected failures: `fifo_depth_internal_0` (xlscc error) and the simulation of `stream_two_blocks_reorder_noflops` (deadlock).

[run.sh](../../exp/xls_pragma_probe/run.sh) codegen settings: `--generator=pipeline --pipeline_stages=2 --delay_model=unit --reset=rst`, plus the probe's `.codegen_flags`. [sim.sh](../../exp/xls_pragma_probe/sim.sh) runs `eval_proc_main --backend=block_interpreter` on the `.mat.v` design's block IR. Inputs are always valid and outputs always ready.

## Setup facts

- `xlscc` is built from source (`e735697f0`, 2026-09-29) and symlinked as `~/bin/xlscc`; the release tarball doesn't include it. `opt_main`/`codegen_main` come from the older release build. Mixing them has worked so far. `eval_proc_main` is built from the same checkout as `xlscc` (2026-10-29) and symlinked as `~/bin/eval_proc_main`.
- **Channels need block mode**, in one of two styles:
  - *Class style* (Exp 1–3, 5, 7): `--block_from_class <Class>`.
  - *Free-function style* (Exp 4, 6): a `--block_pb <spec>.textproto --block_pb_text` input naming the top and its ports. Needed for multi-block designs; `hls_design block` on class methods is unimplemented.
  - With neither, `opt_main` rejects the IR ("channel has no associated send node").

---

## Experiment 1: `hls_fifo_depth` on an internal channel

**Question:** does `[[hls_fifo_depth(N)]]` set a real FIFO depth, like Vitis `#pragma HLS stream depth=N`?
**Result:** ✅ yes. It appears in the IR as `fifo_depth=4` and in the Verilog as `.Depth(4)`.

```cpp
class Block {
  __xls_channel<int, __xls_channel_dir_In>  in;    // external -> ports
  __xls_channel<int, __xls_channel_dir_Out> out;   // external -> ports
  #pragma hls_top
  void foo() {
    [[hls_fifo_depth(4)]] static __xls_channel<int> internal;   // internal -> FIFO
    internal.write(in.read() * 2);
    out.write(internal.read());
  }
};
```
```verilog
xls_fifo_wrapper #(.Width(32'd32), .Depth(32'd4), .EnableBypass(1'd1),
                   .RegisterPushOutputs(1'd0), .RegisterPopOutputs(1'd0))
  fifo___internal_foo_internal (.push_data(...), .pop_data(...), ...);
```

- Only internal channels (`static __xls_channel` inside the function) get a FIFO. External channels (direction-tagged class members) become ready/valid ports.
- Width comes from the channel type (`int` → 32). Depth counts items.
- `xls_fifo_wrapper` is **not defined** in `.v`; use `.mat.v` or supply the module before synthesis.
- The materialized FIFO allocates depth + 1 slots (`buf__1[5]`). Whether synthesis trims the extra slot is untested.
- `bypass` / `register_*_outputs` (the nearest knobs to Vitis `type=`) are hard-coded by xlscc (`translator.cc:6120`) and can't be set from C++.

**Takeaway:** Vitis `stream depth=N` on an internal stream → `[[hls_fifo_depth(N)]]` on a `static __xls_channel`.

---

## Experiment 2: default depth / explicit depth 0

**Question:** what depth does an internal channel get without the pragma, and can you write depth 0?
**Result:** ⚠️ **the default is depth 0, not Vitis's 2.** An explicit `hls_fifo_depth(0)` is a compile error.

```cpp
static __xls_channel<int> internal;                         // _default: no pragma
[[hls_fifo_depth(0)]] static __xls_channel<int> internal;   // _0: rejected
```
```verilog
// fifo_depth_internal_default.v
xls_fifo_wrapper #(.Width(32'd32), .Depth(32'd0), .EnableBypass(1'd1), ...)
```
```
// out/fifo_depth_internal_0/xlscc.log
Error: INVALID_ARGUMENT: the argument to the 'hls_fifo_depth' attribute must be an integer >= 0
```

- Depth 0 is a combinational pass-through, not a buffer: a push only succeeds when the consumer pops in the same cycle (`pop_data = push_data`).
- Source: `fifo_depth.value_or(0)` (`translator.cc:6120`).
- The error message says `>= 0`, but the check is `isStrictlyPositive()` (`translator.cc:7558`). This looks like an XLS bug. Depth 0 is reachable only by omitting the pragma.

**Takeaway:** the translator must always emit `[[hls_fifo_depth(N)]]` on internal channels, using 2 when Vitis gives no depth. Never emit 0.

---

## Experiment 3: `hls_fifo_depth` on an external channel

**Question:** what happens when the pragma is put on a channel that becomes module ports?
**Result:** ⚠️ it is silently ignored: no error, no warning, no depth in the IR, and no FIFO in the Verilog.

```cpp
class Block {
  [[hls_fifo_depth(4)]] __xls_channel<int, __xls_channel_dir_In> in;   // _out: on `out` instead
  __xls_channel<int, __xls_channel_dir_Out> out;
  #pragma hls_top
  void foo() { out.write(in.read() * 2); }
};
```
```verilog
module Block_proc(input wire clk, rst, input wire [31:0] in, input wire in_vld, output wire in_rdy,
                  output wire [31:0] out, output wire out_vld, input wire out_rdy);
  // ... no FIFO anywhere; _in.v and _out.v are byte-identical
```

- The parser accepts the attribute on any variable or field (`cc_parser.cc:634`), but only the internal-channel code path reads it (`translator.cc:6112`).
- XLS top-level ports have no buffering. Any buffering has to come from outside the module.

**Takeaway:** a Vitis stream depth on a top-level port has no XLS equivalent. The translator should drop it **and report that it did**, since XLS won't.

---

## Experiment 4: producer → stream → consumer

**Question:** can the Vitis pattern of two DATAFLOW functions joined by an `hls::stream` be written in XLS, with a controllable depth?
**Result:** ✅ yes. Each `#pragma hls_design block` function becomes its own module, and they are joined by a FIFO set by `hls_fifo_depth`.

```cpp
#pragma hls_design block
void producer(__xls_channel<int, __xls_channel_dir_In>& in,
              __xls_channel<int, __xls_channel_dir_InOut>& xfer) { xfer.write(in.read() + 1); }

#pragma hls_design block
void consumer(__xls_channel<int, __xls_channel_dir_Out>& out,
              __xls_channel<int, __xls_channel_dir_InOut>& xfer) { out.write(xfer.read() * 10); }

#pragma hls_top
void top(__xls_channel<int, __xls_channel_dir_In>& in,
         __xls_channel<int, __xls_channel_dir_Out>& out) {
  [[hls_fifo_depth(4)]] static __xls_channel<int, __xls_channel_dir_InOut> xfer;
  producer(in, xfer);
  consumer(out, xfer);
}
```
```verilog
module top_proc(...);
  producer_proc_proc producer_proc_proc_inst1 (.in(in), .__internal_top_xfer(...), ...);
  consumer_proc_proc consumer_proc_proc_inst0 (.__internal_top_xfer(...), .out(...), ...);
  xls_fifo_wrapper #(.Width(32'd32), .Depth(32'd4), ...) fifo___internal_top_xfer (...);
  top_proc__1 top_proc__1_inst2 (.clk(clk), .rst(rst));   // empty module, harmless
endmodule
```

| Variant | Result |
|---|---|
| `stream_two_blocks` (no pragma) | `.Depth(0)`: the blocks are directly handshake-coupled (see Exp 2). |
| `_depth4` | `.Depth(4)`. Otherwise identical. |
| `_array` (`int[4]` in a struct, depth 2) | 128-bit-wide FIFO, `.Depth(2)`: the whole array moves in one handshake. |

- Each block is a separate RTL module, so this is also how to get the effect of Vitis `inline off`.
- `_array` behaves like a Vitis ping-pong (PIPO) buffer. Sending elements one at a time (Exp 6's `_burst`) is like a Vitis array-to-FIFO stream. The translator picks between them through the channel's item type.
- Whether depth changes behavior is in [Exp 6](#experiment-6-does-stream-depth-matter-simulated).

**Takeaway:** Vitis DATAFLOW + `hls::stream` → `hls_design block` free functions, plus a `static ... _dir_InOut` channel in the top with an explicit `hls_fifo_depth`.

---

## Experiment 5: plain array vs `__xls_memory` (array_partition)

**Question:** does a plain C array become registers that can all be read in one cycle (like Vitis `array_partition complete`), or a RAM (like Vitis's default BRAM, which has 2 ports)?
**Result:** ✅ a plain array is already "completely partitioned": 8 registers, all 8 reads in a single pass. Only `__xls_memory` gives a RAM, and then each access gets its own FSM state.

```cpp
class Block {
  __xls_channel<unsigned, __xls_channel_dir_In> idx;
  __xls_channel<int, __xls_channel_dir_In> val;
  __xls_channel<int, __xls_channel_dir_Out> out;
  // _memory: __xls_memory<int, 8> arr;   (class member, replaces the static below)
  #pragma hls_top
  void foo() {
    static int arr[8] = {0, 0, 0, 0, 0, 0, 0, 0};    // _plain
    arr[idx.read() & 7] = val.read();
    int sum = 0;
    #pragma hls_unroll yes
    for (int i = 0; i < 8; ++i) sum += arr[i];
    out.write(sum);
  }
};
```
```verilog
// array_plain_sum8.v: the array is a register array, read in parallel by an adder tree
reg [31:0] p0_ctx_2__arr_output__3[8];
assign out_op0_0_associative_element__12 = p0_ctx_2__arr_output__3[3'h0] + p0_ctx_2__arr_output__3[3'h1];
assign out_op0_0_associative_element__13 = p0_ctx_2__arr_output__3[3'h2] + p0_ctx_2__arr_output__3[3'h3];  // ...

// array_memory_sum8.v: RAM request/response ports, one access per FSM state
output wire [2:0]  arr_read_request,   output wire arr_read_request_vld,   input wire arr_read_request_rdy,
input  wire [31:0] arr_read_response,  input  wire arr_read_response_vld, output wire arr_read_response_rdy,
output wire [34:0] arr_write_request,  // {addr[2:0], data[31:0]}
reg [3:0] ____fsm_foo_impl_state_0;    // states 0..8: 1 write + 8 reads
```

- Plain array: IR proc state `bits[32][8]`, 8 `array_index` reads, no FSM. Verilog is 97 lines.
- `__xls_memory`: four channels (`arr_read_request/response`, `arr_write_request/response`) become ports to a RAM **outside** the module, plus a 9-state FSM (340 lines). Reads are serialized because xlscc puts each memory op in its own state.
- The RAM interface is abstract: separate read and write channels, no RAM model. Mapping it onto a real 1RW/1R1W SRAM is the `opt_main --ram_rewrites_pb` step (untested here).
- No pragma for Vitis `block`/`cyclic` partitioning was found, and none is needed for `complete`.

**Takeaway:** Vitis `array_partition complete` → drop it, since a plain array already behaves that way. `block`/`cyclic factor=N` → rewrite by hand into N separate arrays (indexing `i / (size/N)` or `i % N`). Vitis's default (a BRAM-backed array) → `__xls_memory`, but only where a RAM is really wanted, because it serializes accesses.

---

## Experiment 6: does stream depth matter? (simulated)

**Question:** in a cycle-accurate simulation, does `hls_fifo_depth` change throughput or prevent the classic dataflow deadlock?
**Result:** ⚠️ depth only matters when one side has to run ahead of the other. Even then, **codegen's port registers add hidden buffering**: the deadlock case works at depth 0 with them, and deadlocks without them.

```cpp
// stream_two_blocks_reorder: producer writes 4 to xfer THEN 1 to side;
// consumer reads side FIRST, then 4 from xfer -> xfer must buffer the burst.
void producer(...) { const int x = in.read();
                     for (int i = 0; i < 4; ++i) xfer.write(x + i);   // unrolled
                     side.write(x); }
void consumer(...) { int sum = side.read();
                     for (int i = 0; i < 4; ++i) sum += xfer.read();  // unrolled
                     out.write(sum); }
// top: [[hls_fifo_depth(N)]] static __xls_channel<int, __xls_channel_dir_InOut> xfer;  (+ side, no pragma)
```
```verilog
// What the port registers are: every block port gets a register stage, e.g. in consumer_proc_proc
reg [31:0] ____internal_top_xfer_reg;   reg ____internal_top_xfer_valid_reg;
// _noflops probes: codegen --flop_inputs=false --flop_outputs=false removes them
```

8 inputs per run; the number is the cycle of the last output.

| Probe | xfer depth | Port registers | Cycles |
|---|---|---|---|
| `stream_two_blocks_burst` / `_depth4` (no reorder) | 0 / 4 | on | 38 / 38 |
| `stream_two_blocks_reorder` | 0 | on | 53 |
| `stream_two_blocks_reorder_depth3` / `_depth4` | 3 / 4 | on | 39 / 39 |
| `stream_two_blocks_reorder_noflops` | 0 | off | ❌ **deadlock** (no output for 100 cycles) |
| `stream_two_blocks_reorder_depth3_noflops` | 3 | off | 35 |

- When the producer and consumer go at the same rate (`_burst`), depth changes nothing. (The same holds for `stream_two_blocks` vs `_depth4`: 14 cycles each.)
- In the reorder case, depth 0 doesn't deadlock with the default codegen settings. It's just slower (53 vs 39), because the port registers (about one per side) hold part of the burst.
- Without the port registers, depth 0 deadlocks, exactly as in Vitis. So the effective buffer is `hls_fifo_depth` + port registers (+ pipeline registers). The pragma alone doesn't decide whether a design deadlocks.
- A burst of 4 needs only depth 3: the producer's `side` write doesn't wait for its last `xfer` write (xlscc's default `--io_op_token_ordering none`). This explanation is a guess; we haven't confirmed it in the IR.

**Takeaway:** emit the Vitis depth (at least the burst size) and don't count on port registers. They depend on codegen flags, and a later flag change could turn a working design into a deadlocked one.

---

## Experiment 7: loop and function pipelining

**Question:** does `hls_pipeline_init_interval` give Vitis `pipeline II=N` behavior on a loop, and is there a function-level II?
**Result:** ✅ loops are pipelined at II=1 with or without the pragma. ⚠️ II=2 is ignored (warning, falls back to 1). Function-level II=N works through codegen `--worst_case_throughput=N`, for the whole block.

```cpp
class Block {
  __xls_channel<int, __xls_channel_dir_In> in;
  __xls_channel<int, __xls_channel_dir_Out> out;
  #pragma hls_top
  void foo() {
    int acc = 0;
    #pragma hls_pipeline_init_interval 1      // _ii2: 2;  _nopragma: line removed
    for (int i = 0; i < 8; ++i) acc += in.read();
    out.write(acc);
  }
};
```
```
// xlscc.log for pipeline_loop_ii2 (with --error_on_init_interval it is an UNIMPLEMENTED error instead)
W... translate_loops.cc:1697] Only initiation interval 1 supported, 2 requested, defaulting to 1
```
```verilog
// _ii1.v, _ii2.v and _nopragma.v are identical apart from file names
```

32 inputs (4 sums) per run:

| Probe | Cycles | Effective II |
|---|---|---|
| `pipeline_loop_ii1` / `_ii2` / `_nopragma` | 34 / 34 / 34 | 1 |
| `pipeline_loop_wct2` (`_ii1` + `--worst_case_throughput=2`) | 65 | 2 |

- In block mode every loop is pipelined at the top-level II (1) anyway, so `hls_pipeline_init_interval 1` changes nothing.
- `xlscc --top_level_init_interval=2` gives the same "only 1 supported" warning, so xlscc offers no II>1 at all.
- `--worst_case_throughput=N` is an **upper bound** ("no slower than once per N cycles"), applied to the whole block. In a quick check that kept no probe, `=4` also gave 65 cycles (II=2): the scheduler doesn't have to use all N.

**Takeaway:** Vitis `pipeline II=1` → drop it or emit `hls_pipeline_init_interval 1` (same result). `pipeline II=N>1` has no per-loop equivalent. The closest is codegen `--worst_case_throughput=N`, which applies to the whole block and is an upper bound. Vitis `pipeline off` / `rewind` / `style`: still none.
