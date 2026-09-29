# XLS[cc] Pragma Notes

XLS paths are relative to `xls/xls/contrib/xlscc/` unless noted. Vitis facts come from the AMD UG1399 pages listed under [Sources](#sources), fetched 2026-09-28. **The fetch tool gave summaries of those pages, not the raw text, so check exact option spellings on the pages themselves.**

## Legend

- ⭐ = likely to be used or seen often in a Vitis → XLS translation
- **Vitis mapping confidence:**
  - ✅ **Verified**: the Vitis doc page's description matches the XLS behavior found in the repo.
  - ⚠️ **Partial / plausible**: related, but the semantics differ or aren't fully confirmed. The reason is noted.
  - ❓ **Unknown / none**: no equivalent found. **Research these.**

## How pragmas work in XLS[cc]

- **Two syntaxes, same meaning.** `#pragma hls_unroll yes` equals `[[hls_unroll(yes)]]` equals `[[xlscc::hls_unroll(yes)]]`.
  - The pragma handler rewrites the pragma into attribute tokens (`GenerateAnnotation`, `cc_parser.cc:52`).
- **Where they're defined:** `cc_parser.cc`. Search for `ParsedAttrInfoRegistry::Add` (attributes) and `PragmaHandlerRegistry::Add` (pragmas).
- **Where to see them used:** `unit_tests/`. For example, search `translator_proc_test.cc` for a pragma's name.
- **Only some are available as `#pragma`:**
  - As pragmas: `hls_top`, `hls_design`, `hls_unroll`, `hls_pipeline_init_interval`, `hls_array_allow_default_pad`.
  - Using `hls_no_tuple` or `hls_synthetic_int` as a pragma is an **error**. They must be attributes.
  - Everything else is attribute-only.
- **Vitis pragmas are ignored silently.** `#pragma HLS ...` is not recognized, and no error is raised.
  - `UnknownPragmaHandler` (`cc_parser.cc:1012`) exists but is never registered.
  - A translator **must** rewrite these pragmas explicitly.
- **Name rules:** names must be lowercase and use the `hls_` prefix. `#pragma unroll` or `#pragma HLS_UNROLL` only produces a warning (`cc_parser.cc:1012-1033`).

---

## Part 1: XLS[cc] pragmas (every registered one)

### Loop pragmas

| | Pragma / attribute | What it does | Usage | Defined | Vitis equivalent |
|---|---|---|---|---|---|
| ⭐ | `hls_unroll yes` | Fully unrolls the loop into N copies of the body. Z3 proves when the loop ends; it fails after `--max_unroll_iters` (default 1000) if it can't. | `#pragma hls_unroll yes` before a `for`/`while`/`do` | `cc_parser.cc:413`, `:997`; logic in `translate_loops.cc:66` | ✅ `#pragma HLS unroll` with no `factor=`. The Vitis doc says: "If factor= is not specified, the loop is fully unrolled." |
| ⭐ | `hls_unroll N` | Partial unroll, implemented as a **pipelined loop** with II=1 that runs N iterations per activation (`translate_loops.cc:136-139`). XLS removes the exit checks automatically when a trial unroll proves the trip count is a multiple of N (`translate_loops.cc:176-203`). | `#pragma hls_unroll 4` | same as above | ✅ `#pragma HLS unroll factor=N`: "The loop body is repeated the specified number of times." ⚠️ Vitis's `skip_exit_check` has no XLS flag, because XLS decides this itself. |
| | `hls_unroll no` | Ignored with a warning ("has no effect"). The loop then follows the no-pragma rules below. | `#pragma hls_unroll no` | `cc_parser.cc:472-480` | ✅ `#pragma HLS unroll off=true` ("Disable unroll for the specified loop"). ⚠️ In XLS function mode, a loop without a pragma is an error, so this only behaves like "don't unroll" in block mode. |
| ⭐ | `hls_pipeline_init_interval N` | Pipelines the loop: one iteration per activation, loop variables become proc state. **Only N=1 is supported.** Other values warn and use 1, or error with `--error_on_init_interval` (`translate_loops.cc:1687`). | `#pragma hls_pipeline_init_interval 1` | `cc_parser.cc:320`, `:981` | ✅ `#pragma HLS pipeline II=N` on a **loop**. ⚠️ Only II=1. ❓ No XLS equivalent for Vitis's `off`, `rewind` or `style=stp/flp/frp`. Vitis also allows PIPELINE on a **function**; see Part 2. |
| | `xlscc::asap` / `xlscc_asap` | Declares that the loop has no cross-iteration dependencies, so it can be scheduled ASAP. Attribute only. | `[[xlscc::asap]] for (...)` | `cc_parser.cc:662` | ⚠️ Closest is `#pragma HLS dependence type=inter false`, which removes assumed dependencies "between different loop iterations." But Vitis DEPENDENCE is per variable and per direction (RAW/WAR/WAW), while `asap` covers the whole loop. |

**Loops with no pragma** (`translate_loops.cc:96-150`, `translator.cc:894-895`):
- In **block/proc mode**: pipelined with the top-level II (`--top_level_init_interval`, default 1).
- Nested inside a pipelined loop: inherits that loop's II.
- Nested inside an unrolled loop: unrolled.
- In **function mode**: an error, `"Loop statement missing #pragma or attribute"`.
- Inside compiler-generated (defaulted) functions: unrolled.
- A loop can't have both `hls_unroll` and `hls_pipeline_init_interval` (`translate_loops.cc:90`).

### Design hierarchy pragmas

| | Pragma / attribute | What it does | Usage | Defined | Vitis equivalent |
|---|---|---|---|---|---|
| ⭐ | `hls_top` | Marks the top function or method, the entry point for translation. You can also pass `--top` on the command line. | `#pragma hls_top` before the function | `cc_parser.cc:170`, `:913` | ⚠️ **Not `#pragma HLS top`.** In Vitis, the top is chosen with `set_top` (Tcl) or the config file. `#pragma HLS top name=<string>` only "assigns an alternate identifier to a function so it can be referenced by the `set_top` command." So the translator must learn the top from the Vitis project config, not from a pragma. |
| ⭐ | `hls_design top` | Pragma shorthand for `hls_top`. Used in `examples/mux3.cc`. | `#pragma hls_design top` | `cc_parser.cc:950` | ❓ Not a Vitis pragma. The syntax looks like Siemens Catapult HLS style (matching the `ac_*` types); unverified. |
| | `hls_design block` | Pragma shorthand for `hls_block`. | `#pragma hls_design block` | `cc_parser.cc:950` | ❓ same as above |
| ⭐ | `hls_block` | Makes a function its **own sub-block** (a separate proc) below the top, called through I/O ops. | `#pragma hls_design block` or `[[hls_block]]` on a function | `cc_parser.cc:734`; used at `translator.cc:1650` | ⚠️ Two partial overlaps: `#pragma HLS inline off` keeps a function "as a separate RTL module," and `#pragma HLS dataflow` runs functions as concurrent processes connected by channels. `hls_block` produces a separate, channel-connected proc, which is closer to a DATAFLOW process, but the semantics aren't confirmed equal. |
| | `hls_shared_function` | Uses **one shared hardware instance** for all calls to the function, reached through I/O ops. Requires `--generate_new_fsm` (`translator.cc:1278-1281`). | `[[hls_shared_function]]` on a function | `cc_parser.cc:1037` | ✅ `#pragma HLS allocation function instances=<f> limit=1`. ALLOCATION with `limit=1` forces "multiple C-level function calls to share a single RTL implementation." ⚠️ No XLS equivalent for `limit>1`, or for the `operation` type (e.g. limiting multipliers). |
| | `hls_control_channel_depth(N)` | Sets the depth of the control-channel FIFO for a sub-block. Default 0. | `[[hls_control_channel_depth(2)]]` on a function | `cc_parser.cc:770` | ❓ none found |
| | `hls_propagate_barrier_scopes` | Advanced. Makes a function pass its activation-barrier scopes up to the caller (see `xls/docs_src/design_docs/xlscc_activation_barriers.md`). Not yet supported together with `hls_shared_function`. | `[[hls_propagate_barrier_scopes]]` on a function | `cc_parser.cc:1075`; used at `translator.cc:1284`, `:4709` | ❓ none. This is an XLS-internal FSM concept. |

### Channel pragmas

| | Pragma / attribute | What it does | Usage | Defined | Vitis equivalent |
|---|---|---|---|---|---|
| ⭐ | `hls_fifo_depth(N)` | Sets the FIFO depth of an **internal** `__xls_channel`. | `[[hls_fifo_depth(4)]] static __xls_channel<int> internal;` (test `LocalChannel` in `unit_tests/translator_proc_test.cc`) | `cc_parser.cc:619` | ⚠️ `#pragma HLS stream variable=<v> depth=N`. **Caution:** the Vitis doc text says depth is "relevant only for array streaming in DATAFLOW channels," which suggests it targets C arrays converted to FIFOs, not `hls::stream` variables. Check how Vitis sets the depth of an `hls::stream`. The Vitis `type=fifo/pipo/shared/unsync` option has no XLS equivalent (❓). |
| | `hls_channel_strictness(mode)` | Controls how multiple reads/writes on one channel are legalized. Modes (`xls/xls/ir/channel.h:338`): `proven_mutually_exclusive` (default), `runtime_mutually_exclusive`, `total_order`, `runtime_ordered`, `arbitrary_static_order`. Can also be set with `--channel_strictness` / `--default_channel_strictness`. | `[[hls_channel_strictness(total_order)]]` on a channel declaration | `cc_parser.cc:516`; used at `translate_block.cc:96` | ❓ none found. The Vitis pragma list has nothing similar. |

### Data and type pragmas

| | Pragma / attribute | What it does | Usage | Defined | Vitis equivalent |
|---|---|---|---|---|---|
| | `hls_array_allow_default_pad` | Allows an array initializer list with fewer elements than the array. Missing elements are zero-filled instead of raising an error. | `#pragma hls_array_allow_default_pad` before the declaration | `cc_parser.cc:205`, `:931`; used at `translator.cc:1983` | ❓ none. This is a C++-strictness knob, not an HLS directive. |
| | `hls_no_tuple` | Represents a struct with **exactly one** field as that field alone in the IR, not as a 1-tuple. Mostly used internally (e.g. `__xls_token` in the built-in header). Attribute only. | `struct [[hls_no_tuple]] S { int x; };` | `cc_parser.cc:283`; checked at `translator.cc:561` | ❓ none. Vitis `aggregate`/`disaggregate` also control struct packing, but they aren't this. |
| | `hls_synthetic_int` | Marks a class as a custom integer implementation, for metadata. Used by `synth_only/xls_int.h` on `XlsInt`. Attribute only. | `class [[hls_synthetic_int]] MyInt {...};` | `cc_parser.cc:243` | ❓ none. Internal to XLS's integer headers. |

### Type annotations (not registered pragmas)

These use Clang's standard `annotate_type` attribute. `translate_io.cc` checks for them by string.

| | Annotation | What it does | Usage | Where | Vitis equivalent |
|---|---|---|---|---|---|
| ⭐ | `hls_memory` | Treats a plain C array as an external **memory** (same as `__xls_memory<T, N>`), instead of an IR array value. | `static short store[21] [[clang::annotate_type("hls_memory")]];` (`unit_tests/translator_memory_test.cc:1698`) | `translate_io.cc:373`, `:495` | ⚠️ Closest is `#pragma HLS bind_storage variable=<v> type=ram_... impl=bram/uram/...`, which assigns an array to a RAM type. In XLS the RAM *kind* is chosen later, at `opt_main --ram_rewrites_pb` time. Only `RAM_1RW` and `RAM_1R1W` exist (`xls/xls/ir/ram_rewrite.proto:26-28`); a mapping such as `ram_1p`→1RW or `ram_s2p`→1R1W is **my guess**, so verify it. |
| | `hls_array_as_tuple` | Represents an array as a tuple in the IR instead of an array type. | `long long arr[4] [[clang::annotate_type("hls_array_as_tuple")]];` (`unit_tests/translator_logic_test.cc:5489`) | `translate_io.cc:491` | ❓ Unclear. It may be loosely related to `array_partition type=complete` ("decomposes the array into individual elements"). See the ARRAY_PARTITION note in Part 2. |

### Related built-ins (not pragmas, but they fill a similar role)

Defined in the virtual header `/xls_builtin.h` (`cc_parser.cc:1409-1540`).

- `__xlscc_activation_barrier<bool conditional>()` forces the FSM to start a new activation at that point.
- `__xlscc_on_reset` is `true` on the first activation after reset.
- `__xls_channel<T, Dir>` is a FIFO stream with ready/valid. The Vitis equivalent is `hls::stream<T>`.
- `__xls_memory<T, N>` is an external SRAM.

### Command-line flags that act like pragmas (`main.cc`)

- `--top`: select the top function without a pragma.
- `--top_level_init_interval`: II of the top-level `Run()`. Only 1 is supported.
- `--max_unroll_iters` (default 1000) and `--warn_unroll_iters` (default 100).
- `--split_states_on_channel_ops` (default true): puts two ops on the same channel in separate FSM states.
- `--channel_strictness`, `--default_channel_strictness`.
- `--generate_new_fsm`: required for `hls_shared_function`.

---

## Part 2: All 29 Vitis pragmas → where they could go in XLS

This is the translation-direction view. ⭐ marks the pragmas most common in real Vitis code, based on my general HLS familiarity, not on these docs.

| | Vitis pragma | What it does (per the Vitis docs) | XLS-side lead | Confidence |
|---|---|---|---|---|
| ⭐ | `unroll [factor=N] [skip_exit_check] [off=true]` | Makes copies of the loop body. | `hls_unroll yes/N/no` (Part 1) | ✅ |
| ⭐ | `pipeline II=N [off] [rewind] [style=]` | Pipelines a loop **or function** at interval II. | On loops: `hls_pipeline_init_interval 1`. On the top function: the top body is already a proc that runs every activation; `--top_level_init_interval` only accepts 1. **Function-level II>1:** maybe codegen's `--worst_case_throughput=N` (`xls/docs_src/codegen_options.md:106`), but that applies to the whole proc. `off`, `rewind`, `style`: none found. | ⚠️ |
| ⭐ | `stream variable= type= depth=` | Implements an **array** as a FIFO or ping-pong buffer instead of RAM. | Local `__xls_channel` + `hls_fifo_depth(N)`. No `type=pipo/shared/unsync`. | ⚠️ See the caution on `hls_fifo_depth` above. |
| ⭐ | `dataflow [disable_start_propagation]` | Task-level pipelining: sequential functions and loops overlap, connected by "channels (based on ping pong RAMs or FIFOs)". | Likely `hls_block` sub-blocks and/or local channels between procs. Concurrent procs linked by channels are XLS's native model. | ❓ Needs research. Probably the biggest structural translation problem. |
| ⭐ | `interface mode= port=` | Maps top-level arguments to RTL port protocols. | HLSBlock channel types (`hls_block.proto`), or the class-member conventions of `--block_from_class`. My guesses: `ap_fifo`/`axis` ≈ `CHANNEL_TYPE_FIFO` (ready/valid), `ap_none` ≈ `CHANNEL_TYPE_DIRECT_IN`, `ap_memory`/`bram` ≈ `CHANNEL_TYPE_MEMORY`. **No AXI support was found in XLS[cc] or in `xls/xls/modules/`**, so `m_axi`, `s_axilite` and `ap_ctrl_*` have no known equivalent. | ⚠️ / ❓ Signal-level protocol details (valid/ack naming, AXI) are unverified. |
| ⭐ | `array_partition variable= type=block/cyclic/complete factor= dim=` | Splits an array into smaller arrays or single elements to get more ports. | Plain C arrays in XLS[cc] become IR array *values* (e.g. `ArrayParam` in `unit_tests/translator_logic_test.cc:255`), not RAM. Only `__xls_memory` / `hls_memory` become SRAM. So `complete` may already be XLS's default. `block`/`cyclic`: none found. | ⚠️ |
| | `array_reshape` | Array reshaping (combines partitioning with wider words). | none found | ❓ |
| | `array_stencil` | Array stencil optimization. | none found | ❓ |
| ⭐ | `bind_storage variable= type= impl= latency=` | Binds an array to a memory type/implementation, with latency. | `hls_memory` / `__xls_memory` + `opt_main --ram_rewrites_pb` (RAM_1RW / RAM_1R1W) + codegen `--ram_configurations`. The memory tutorial assumes a "fixed 1-cycle latency" (`xls/docs_src/tutorials/xlscc_memory.md`). `impl=`: none found. | ⚠️ |
| | `bind_op` | Binds an operator to a specific implementation. | none found | ❓ |
| | `allocation function/operation instances= limit=` | Limits how many instances of a function or operator are built. | `hls_shared_function` = function with `limit=1` only | ✅ for function `limit=1`, ❓ otherwise |
| ⭐ | `inline [off] [recursive]` | Dissolves a function into its caller. By default Vitis inlines small functions. | XLS has inlining passes in `opt_main` (`xls/docs_src/passes_list.md`, "inlining"). For `off` (keep as a separate module), the closest lead is `hls_block`. | ⚠️ |
| | `function_instantiate variable=` | Builds a separate RTL copy per call site, each specialized for a constant argument. | Possibly unnecessary: XLS inlines calls, after which constants propagate. **Unverified**. | ❓ |
| | `dependence variable= type=inter/intra direction= distance= true/false` | Declares or removes loop dependencies. | `xlscc::asap` for the whole loop, with no per-variable control | ⚠️ |
| ⭐ | `loop_tripcount min= max= avg=` | "For analysis only, and does not impact the results of synthesis." | **Safe to drop** in translation. | ✅ (drop it) |
| | `loop_flatten` | Flattens nested loops. | none found | ❓ |
| | `loop_merge` | Merges consecutive loops. | none found | ❓ |
| | `latency min= max=` | Constrains the latency of a function, loop or region, in cycles. | Only whole-design scheduling flags: codegen `--pipeline_stages`, `--clock_period_ps` (`xls/docs_src/scheduling.md`). No per-loop or per-region control found. | ⚠️ |
| | `performance target_ti= [unit=sec/cycle]` | Target transaction interval for a function or loop. | Possibly codegen `--worst_case_throughput` (whole proc) | ⚠️ |
| | `reset [variable=] [off]` | Controls which static/global/class state variables get reset. | XLS state gets reset to its initial value (see the counter Verilog in `xls/docs_src/tutorials/xlscc_state.md`). Reset behavior is set by codegen flags (`--reset`, `--reset_data_path`, …). No per-variable control found. `__xlscc_on_reset` is related, but it's a different thing. | ❓ |
| | `aggregate` | Packs struct fields. | XLS structs become IR tuples (`GetStructXLSType`, `translator.cc:556`). No packing knob found. | ❓ |
| | `disaggregate` | Unpacks a struct into its fields. | none found | ❓ |
| | `alias` | Memory aliasing hint. | none found | ❓ |
| | `cache` | Caching optimization. | none found | ❓ |
| | `expression_balance` | Expression tree balancing. | none as a pragma; XLS `opt_main` has its own optimization passes | ❓ |
| | `occurrence` | Pipeline occurrence control. | none found | ❓ |
| | `protocol` | Protocol-region specification. | none found | ❓ |
| | `stable` | Marks a stable array (its value doesn't change during execution). | Maybe related to XLS direct-ins (`CHANNEL_TYPE_DIRECT_IN`: a plain wire that isn't handshaked); unverified | ❓ |
| | `top name=` | Gives a function an alternate name for `set_top`. | See `hls_top` above: take the top from the Vitis project config and emit `#pragma hls_top` or `--top`. | ⚠️ |

---

## Sources

Vitis HLS UG1399 (AMD):
- Index: https://docs.amd.com/r/en-US/ug1399-vitis-hls/HLS-Pragmas-and-Directives
- Pages actually fetched: Unroll, Pipeline, Stream, Top, Dependence, Allocation, Inline, Dataflow, Bind_storage, Array_partition, Reset, Performance, Latency, Interface, Function_instantiate, Loop_tripcount.
  - Pattern: `https://docs.amd.com/r/en-US/ug1399-vitis-hls/<Name>`
- **Not fetched** (their rows rely only on the index's one-line descriptions): Aggregate, Alias, Array_reshape, Array_stencil, Bind_op, Cache, Disaggregate, Expression_balance, Loop_flatten, Loop_merge, Occurrence, Protocol, Stable.
- The index also lists **Resource** and **Shared** pages that weren't in the main pragma list. Probably deprecated or related; not checked.
