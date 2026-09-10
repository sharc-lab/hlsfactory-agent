# Translation Report: Vitis HLS to Catapult HLS

## Design: n1024_UF1 (FFT_TOP)

### Table 1: Pragma Translation

| # | File | Original Vitis HLS Pragma | Catapult Equivalent | Notes |
|---|------|--------------------------|---------------------|-------|
| 1 | FFT.cpp | `#pragma HLS inline` (in RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma hls_design inline` | Moved to before function definition |
| 2 | FFT.cpp | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | DROPPED | No Catapult source-level equivalent. Resource binding done in TCL. |
| 3 | FFT.cpp | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | DROPPED | Same as above |
| 4 | FFT.cpp | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | DROPPED | Same as above |
| 5 | FFT.cpp | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | DROPPED | Same as above |
| 6 | FFT.cpp | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | DROPPED | Same as above |
| 7 | FFT.cpp | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | DROPPED | Same as above |
| 8 | FFT.cpp | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` (in onlycompute) | DROPPED | Same as above |
| 9 | FFT.cpp | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` (in onlycompute) | DROPPED | Same as above |
| 10 | FFT.cpp | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` (in onlycompute) | DROPPED | Same as above |
| 11 | FFT.cpp | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` (in onlycompute) | DROPPED | Same as above |
| 12 | FFT.cpp | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` (in onlycompute) | DROPPED | Same as above |
| 13 | FFT.cpp | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` (in onlycompute) | DROPPED | Same as above |
| 14 | FFT.cpp | `#pragma HLS bind_op variable=index op=mul impl=fabric` (3 occurrences) | DROPPED | Same as above |
| 15 | FFT.cpp | `#pragma HLS pipeline` (in output_result_array_to_stream) | `#pragma hls_pipeline_init_interval 1` | Moved to before for-loop |
| 16 | FFT.cpp | `#pragma HLS pipeline` (in FFT_stage_spatial_unroll, L_Pair_loop) | `#pragma hls_pipeline_init_interval 1` | Moved to before inner for-loop |
| 17 | FFT.cpp | `#pragma HLS pipeline` (in FFT_stage_spatial_unroll, R_Group_loop) | `#pragma hls_pipeline_init_interval 1` | Moved to before inner for-loop |
| 18 | FFT.cpp | `#pragma HLS pipeline` (in FFT_stage_spatial_unroll, bflySize==FFT_NUM) | `#pragma hls_pipeline_init_interval 1` | Moved to before for-loop |
| 19 | FFT.cpp | `#pragma HLS pipeline II=1` (in reverse_input_stream_UF1, READ_STREAM_INPUT) | `#pragma hls_pipeline_init_interval 1` | Moved to before for-loop |
| 20 | FFT.cpp | `#pragma HLS pipeline II=1` (in reverse_input_stream_UF1, FROM_BLOCK_TO_CYCLIC) | `#pragma hls_pipeline_init_interval 1` | Moved to before for-loop |
| 21 | FFT.cpp | `#pragma HLS pipeline II=1` (in reverse_input_stream_UF1, STREAM_OUT_REVERSE) | `#pragma hls_pipeline_init_interval 1` | Moved to before for-loop |
| 22 | FFT.cpp | `#pragma HLS pipeline` (in FFT_Stage1_vectorstream_parameterize) | `#pragma hls_pipeline_init_interval 1` | Moved to before for-loop |
| 23 | FFT.cpp | `#pragma HLS UNROLL` (in bit_reverse, Loop_Reverse) | `#pragma hls_unroll yes` | Moved to before for-loop |
| 24 | FFT.cpp | `#pragma HLS unroll factor=UF>>(stage-1)` (in L_Pair_loop) | `#pragma hls_unroll (UF>>(stage-1))` | Moved to before inner for-loop |
| 25 | FFT.cpp | `#pragma HLS unroll factor=UF` (in R_Group_loop) | `#pragma hls_unroll UF` | Moved to before inner for-loop |
| 26 | FFT.cpp | `#pragma HLS unroll factor=UF` (in bflySize==FFT_NUM) | `#pragma hls_unroll UF` | Moved to before for-loop |
| 27 | FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (in reverse_input_stream_UF1) | `#pragma hls_design block` | On function definition. Dataflow region converted to hierarchical block. |
| 28 | FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (in FFT_Stage1_vectorstream_parameterize) | `#pragma hls_design block` | On function definition. |
| 29 | FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (in FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma hls_design block` | On function definition. |
| 30 | FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (in FFT_TOP) | `#pragma hls_design top` | On FFT_TOP function definition. |
| 31 | FFT.cpp | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=1` | DROPPED | No source-level equivalent in Catapult. Memory mapping set in TCL. |
| 32 | FFT.cpp | `#pragma HLS array_partition variable=data_in_cyclic type=cyclic factor=UF dim=2` | DROPPED | Same as above |
| 33 | FFT.cpp | `#pragma HLS array_partition variable=original type=complete dim=1` | DROPPED | Same as above |
| 34 | FFT.cpp | `#pragma HLS array_partition variable=reversed type=complete dim=1` | DROPPED | Same as above |
| 35 | FFT.cpp | `#pragma HLS array_partition variable=offset type=complete dim=1` | DROPPED | Same as above |
| 36 | FFT.cpp | `#pragma HLS array_partition variable=cyclic_offset type=complete dim=1` | DROPPED | Same as above |
| 37 | FFT.cpp | `#pragma HLS array_partition variable=block_data type=complete dim=1` | DROPPED | Same as above |
| 38 | FFT.cpp | `#pragma HLS array_partition variable=cyclic_data type=complete dim=1` | DROPPED | Same as above |
| 39 | FFT.cpp | `#pragma HLS array_partition variable=twiddles complete` | DROPPED | Same as above |
| 40 | FFT.cpp | `#pragma HLS array_partition variable=data_1..data_10 type=cyclic factor=UF dim=1` (10 instances) | DROPPED | Same as above |
| 41 | FFT.cpp | `#pragma HLS stream type=pipo variable=data_in_cyclic` | DROPPED | No Catapult equivalent for Vitis stream type. |
| 42 | FFT.cpp | `#pragma HLS stream type=pipo variable=data_rev_stream` | DROPPED | Same as above |
| 43 | FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (6 occurrences) | DROPPED | Performance target pragma; Catapult has similar via `#pragma hls_performance` but removed for translation simplicity. Can be re-added during optimization. |

### Table 2: Type and Header Changes

| Original Vitis Type/Header | Replacement | Notes |
|---------------------------|-------------|-------|
| `hls::stream<T>` | `ac_channel<T>` | Siemens Algorithmic C channel type |
| `hls::vector<T, N>` | `vect<T, N>` | Custom template wrapper (struct with array and operator[]) |
| `ap_uint<N>` | `ac_int<N, false>` | Unsigned integer; signedness preserved (false = unsigned) |
| `#include "hls_stream.h"` | `#include "ac_channel.h"` | AC channel header |
| `#include "hls_vector.h"` | (removed, replaced by custom `vect` template) | Defined directly in FFT.h |
| `#include "ap_fixed.h"` | (removed) | Not used in the design; no ap_fixed types present |
| `#include "hls_fft.h"` | (removed) | Not used in the design |
| `#include "hls_streamofblocks.h"` | (removed) | Not used in the design |
| `x.range(hi, lo)` | `x.slc<hi-lo+1>(lo)` | For ap_uint/bit-range. In this design, the `range(bit_i, bit_i)` was replaced with direct bit access `x[i]` since range width=1. |
| `reversed.range(bit_i, bit_i) = input.range(N-1-bit_i, N-1-bit_i)` | `reversed[bit_i] = input[N-1-bit_i]` | Single-bit access is simpler and equivalent |

### Table 3: Behavioral and Performance Impact Analysis

| Aspect | Impact | Details |
|--------|--------|---------|
| **Rounding/Overflow modes** | None | The original used default modes (AP_TRN/AP_WRAP) which match Catapult defaults (AC_TRN/AC_WRAP). No explicit rounding/overflow modes were specified. |
| **Array partitioning (DROPPED)** | Performance difference possible | 15+ `#pragma HLS array_partition` directives were dropped. These control memory banking for parallel access. Catapult handles memory mapping via TCL scripts and heuristics; the generated RTL may have different memory architecture. |
| **Bind_OP (DROPPED)** | Performance difference possible | 10+ `#pragma HLS bind_op` directives were dropped. These explicitly bind operations to fabric (LUT) vs DSP resources. Catapult's scheduler makes its own resource allocation decisions. |
| **HLS stream type=pipo (DROPPED)** | Minor | The `#pragma HLS stream type=pipo` directives were dropped. These only affect Vitis's FIFO implementation; Catapult infers channels from ac_channel usage automatically. |
| **Performance target (DROPPED)** | Minor | `#pragma HLS performance target_ti=... unit=cycle` directives dropped. These are analysis-only constraints in Vitis. Catapult has `#pragma hls_performance` as an equivalent. Can be re-added. |
| **Dataflow to hierarchical blocks** | Functional equivalence maintained | Vitis dataflow regions converted to `#pragma hls_design block` on each sub-function. Catapult's block-level design is coarser (per function vs per region). The function call chain matches the dataflow pipeline order. |
| **Loop unrolling and pipelining** | Equivalent | All UNROLL and PIPELINE pragmas were translated to Catapult equivalents and moved before the loop construct as required by Catapult syntax. |
| **Vector replacement** | Equivalent | `hls::vector<T,N>` replaced with custom `vect<T,N>` struct. Same memory layout and access pattern. |
| **Channel semantics** | Equivalent | `hls::stream` and `ac_channel` both provide blocking read/write FIFO semantics. No `empty()` calls existed in the original, so no translation needed. |

### Testbench Result
**Status: PASSED**

The testbench compiled and ran successfully with all checks passing. Maximum error was 0.0000 (well below the 1.0 threshold). The Catapult port produces bit-identical results to the Vitis original within floating-point precision.

### Vitis Identifiers Removed
The following Vitis-specific identifiers have been completely removed from the output:
- `ap_int`, `ap_uint`, `ap_fixed` — replaced with `ac_int` variants
- `hls::stream` — replaced with `ac_channel`
- `hls::vector` — replaced with custom `vect`
- `#pragma HLS` — all replaced with Catapult pragmas or dropped
- Vitis headers (`hls_stream.h`, `hls_vector.h`, `ap_fixed.h`, `hls_fft.h`, `hls_streamofblocks.h`) — all removed