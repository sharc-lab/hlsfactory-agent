# XLS[cc] Pragma Experiment Results

Everything here was checked by compiling code with our own XLS build. The pragma mapping tables are in [xls_pragma_notes.md](xls_pragma_notes.md); those come from reading source and docs, so where the two disagree, trust this file. Last updated 2026-10-29.

## Purpose

Each experiment is a small XLS C++ program pushed through xlscc → opt_main → codegen_main, to check one Vitis → XLS mapping from the notes. Priority order: `stream`, `pipeline`, `array_partition`, then everything rated ⚠️ or ❓ in the notes.

**Nothing has been simulated yet** (no `eval_proc_main`, `iverilog` or `yosys` on `chao-srv1`). Results are about the structure of the IR and Verilog, not cycle behavior or area.

## Experiment index

| # | Experiment | Probes | Result |
|---|---|---|---|
| 1 | [`hls_fifo_depth` on an internal channel](#experiment-1-hls_fifo_depth-on-an-internal-channel) | `fifo_depth_internal_4` | ✅ Depth reaches the IR and Verilog. |
| 2 | [Default depth / explicit depth 0](#experiment-2-default-depth--explicit-depth-0) | `fifo_depth_internal_{default,0}` | ⚠️ **Default depth is 0, not 2 as in Vitis.** Writing `hls_fifo_depth(0)` is an error. |
| 3 | [`hls_fifo_depth` on an external channel](#experiment-3-hls_fifo_depth-on-an-external-channel) | `fifo_depth_external_{in,out}` | ⚠️ Silently ignored. |
| 4 | [Producer → stream → consumer](#experiment-4-producer--stream--consumer) | `stream_two_blocks{,_depth4,_burst,_array}` | ✅ Maps to Vitis DATAFLOW + `hls::stream`: two modules joined by a FIFO. |

## Probe file map

Naming: `<what is tested>_<where>_<variant>`. Probe `X` = C++ in `probes/X.cc` (plus `probes/X.textproto` for `stream_*`), outputs in `out/X/`:

| File in `out/X/` | What it is |
|---|---|
| `X.ir` / `X.opt.ir` | Raw / optimized IR. Channel declarations (with `fifo_depth`) are at the top. |
| `X.v` | Verilog. FIFOs are `xls_fifo_wrapper` instances, **not defined** in the file. |
| `X.mat.v` | Self-contained Verilog (`--materialize_internal_fifos`), each FIFO its own module. |
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
| [stream_two_blocks_burst](../../exp/xls_pragma_probe/probes/stream_two_blocks_burst.cc) | 4 | 4 writes / 4 reads per call, depth 4 | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_burst/) |
| [stream_two_blocks_array](../../exp/xls_pragma_probe/probes/stream_two_blocks_array.cc) | 4 | `int[4]` sent as one item, depth 2 | [out/](../../exp/xls_pragma_probe/out/stream_two_blocks_array/) |

## How to run

```sh
cd exp/xls_pragma_probe
./run_all.sh                                                       # every probe (fifo_depth_internal_0 fails by design)
./run.sh probes/fifo_depth_internal_4.cc --block_from_class Block  # one class-style probe
./run.sh probes/stream_two_blocks.cc                               # one free-function probe (picks up the .textproto)
```

[run.sh](../../exp/xls_pragma_probe/run.sh) codegen settings: `--generator=pipeline --pipeline_stages=2 --delay_model=unit --reset=rst`.

## Setup facts

- `xlscc` is built from source (`e735697f0`, 2026-09-29) and symlinked as `~/bin/xlscc`; the release tarball doesn't include it. `opt_main`/`codegen_main` come from the older release build. Mixing them has worked so far.
- **Channels need block mode**, in one of two styles:
  - *Class style* (Exp 1–3): `--block_from_class <Class>`.
  - *Free-function style* (Exp 4): a `--block_pb <spec>.textproto --block_pb_text` input naming the top and its ports. Needed for multi-block designs; `hls_design block` on class methods is unimplemented.
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
| `_burst` (4 writes / 4 reads per call) | `.Depth(4)`; each block becomes a multi-state FSM with one channel op per state. |
| `_array` (`int[4]` in a struct, depth 2) | 128-bit-wide FIFO, `.Depth(2)`: the whole array moves in one handshake. |

- Each block is a separate RTL module, so this is also how to get the effect of Vitis `inline off`.
- `_array` behaves like a Vitis ping-pong (PIPO) buffer; `_burst` is like a Vitis array-to-FIFO stream, one element at a time. The translator picks between them through the channel's item type.
- Not shown: whether depth actually lets the producer run ahead. That needs simulation of `_burst` at depths 0/1/4.

**Takeaway:** Vitis DATAFLOW + `hls::stream` → `hls_design block` free functions, plus a `static ... _dir_InOut` channel in the top with an explicit `hls_fifo_depth`.
