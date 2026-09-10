# Translation Report: Vitis HLS → Siemens Catapult HLS

## Design: `n1024_UF8`
## Top Function: `FFT_TOP`

---

## Table 1: `#pragma HLS` Translation

| # | File | Original Vitis Pragma | Catapult Equivalent / Status | Notes |
|---|---|---|---|---|
| 1 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_CY | `#pragma HLS inline` | `#pragma hls_design inline` | Translated; placed before function definition |
| 2 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_CY | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | **DROPPED** | No source-level equivalent in Catapult; resource binding done in TCL |
| 3 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_CY | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | **DROPPED** | No source-level equivalent in Catapult |
| 4 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_CY | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | **DROPPED** | No source-level equivalent in Catapult |
| 5 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_CY | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | **DROPPED** | No source-level equivalent in Catapult |
| 6 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_CY | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | **DROPPED** | No source-level equivalent in Catapult |
| 7 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_CY | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | **DROPPED** | No source-level equivalent in Catapult |
| 8 | FFT.cpp: output_result_array_to_stream | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only; no Catapult source equivalent |
| 9 | FFT.cpp: output_result_array_to_stream | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` | Translated; moved before the `for` loop |
| 10 | FFT.cpp: bit_reverse | `#pragma HLS inline` | `#pragma hls_design inline` | Translated; placed before function definition |
| 11 | FFT.cpp: bit_reverse | `#pragma HLS UNROLL` | `#pragma hls_unroll yes` | Translated; moved before the `for` loop |
| 12 | FFT.cpp: reverse_input_stream_UF8 | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=1` | **DROPPED** | No source-level equivalent in Catapult; memory mapping via TCL |
| 13 | FFT.cpp: reverse_input_stream_UF8 | `#pragma HLS array_partition variable=data_in_cyclic type=cyclic factor=UF dim=2` | **DROPPED** | No source-level equivalent in Catapult |
| 14 | FFT.cpp: reverse_input_stream_UF8 | `#pragma HLS stream type=pipo variable=data_in_cyclic` | **DROPPED** | No source-level equivalent in Catapult |
| 15 | FFT.cpp: reverse_input_stream_UF8 | `#pragma HLS stream type=pipo variable=data_rev_stream` | **DROPPED** | No source-level equivalent in Catapult |
| 16 | FFT.cpp: reverse_input_stream_UF8 | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design block` | Translated; `disable_start_propagation` has no direct equivalent. Placed before the sub-function definition (reverse_input_stream_UF8 is a called function, not a dataflow block itself) |
| 17 | FFT.cpp: reverse_input_stream_UF8 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| 18 | FFT.cpp: reverse_input_stream_UF8 (READ loop) | `#pragma HLS pipeline II=1` | `#pragma hls_pipeline_init_interval 1` | Translated; moved before the `for` loop |
| 19 | FFT.cpp: reverse_input_stream_UF8 (READ loop) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (commented out) | **DROPPED** | Was already commented out in original |
| 20 | FFT.cpp: reverse_input_stream_UF8 (READ loop) | `#pragma HLS array_partition variable=original type=complete dim=1` | **DROPPED** | No source-level equivalent |
| 21 | FFT.cpp: reverse_input_stream_UF8 (READ loop) | `#pragma HLS array_partition variable=reversed type=complete dim=1` | **DROPPED** | No source-level equivalent |
| 22 | FFT.cpp: reverse_input_stream_UF8 (FROM_BLOCK loop) | `#pragma HLS pipeline II=1` | `#pragma hls_pipeline_init_interval 1` | Translated; moved before the `for` loop |
| 23 | FFT.cpp: reverse_input_stream_UF8 (FROM_BLOCK loop) | `#pragma HLS array_partition variable=offset type=complete dim=1` | **DROPPED** | No source-level equivalent |
| 24 | FFT.cpp: reverse_input_stream_UF8 (FROM_BLOCK loop) | `#pragma HLS array_partition variable=cyclic_offset type=complete dim=1` | **DROPPED** | No source-level equivalent |
| 25 | FFT.cpp: reverse_input_stream_UF8 (FROM_BLOCK loop) | `#pragma HLS array_partition variable=block_data type=complete dim=1` | **DROPPED** | No source-level equivalent |
| 26 | FFT.cpp: reverse_input_stream_UF8 (FROM_BLOCK loop) | `#pragma HLS array_partition variable=cyclic_data type=complete dim=1` | **DROPPED** | No source-level equivalent |
| 27 | FFT.cpp: reverse_input_stream_UF8 (STREAM_OUT loop) | `#pragma HLS pipeline II=1` | `#pragma hls_pipeline_init_interval 1` | Translated; moved before the `for` loop |
| 28 | FFT.cpp: FFT_stage_spatial_unroll | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| 29 | FFT.cpp: FFT_stage_spatial_unroll (L_Pair_loop) | `#pragma HLS UNROLL factor=UF>>(stage-1)` | `#pragma hls_unroll yes factor=UF>>(stage-1)` | Translated; moved before the `for` loop |
| 30 | FFT.cpp: FFT_stage_spatial_unroll (L_Pair_loop) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` | Translated; moved before the `for` loop |
| 31 | FFT.cpp: FFT_stage_spatial_unroll (L_Pair_loop) | `#pragma HLS bind_op variable=index op=mul impl=fabric` | **DROPPED** | No source-level equivalent |
| 32 | FFT.cpp: FFT_stage_spatial_unroll (bflySize==FFT_NUM) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` | Translated; moved before the `for` loop |
| 33 | FFT.cpp: FFT_stage_spatial_unroll (bflySize==FFT_NUM) | `#pragma HLS UNROLL factor=UF` | `#pragma hls_unroll yes factor=UF` | Translated; moved before the `for` loop |
| 34 | FFT.cpp: FFT_stage_spatial_unroll (bflySize==FFT_NUM) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| 35 | FFT.cpp: FFT_stage_spatial_unroll (bflySize==FFT_NUM) | `#pragma HLS bind_op variable=index op=mul impl=fabric` | **DROPPED** | No source-level equivalent |
| 36 | FFT.cpp: FFT_stage_spatial_unroll (else branch) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| 37 | FFT.cpp: FFT_stage_spatial_unroll (else branch) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` | Translated; moved before the `for` loop |
| 38 | FFT.cpp: FFT_stage_spatial_unroll (else branch) | `#pragma HLS UNROLL factor=UF` | `#pragma hls_unroll yes factor=UF` | Translated; moved before the `for` loop |
| 39 | FFT.cpp: FFT_stage_spatial_unroll (else branch) | `#pragma HLS bind_op variable=index op=mul impl=fabric` | **DROPPED** | No source-level equivalent |
| 40 | FFT.cpp: FFT_stage_spatial_unroll (else branch) | `#pragma HLS array_partition variable=twiddles complete` | **DROPPED** | No source-level equivalent |
| 41 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_onlycompute | `#pragma HLS inline` | `#pragma hls_design inline` | Translated; placed before function definition |
| 42 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_onlycompute | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | **DROPPED** | No source-level equivalent |
| 43 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_onlycompute | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | **DROPPED** | No source-level equivalent |
| 44 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_onlycompute | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | **DROPPED** | No source-level equivalent |
| 45 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_onlycompute | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | **DROPPED** | No source-level equivalent |
| 46 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_onlycompute | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | **DROPPED** | No source-level equivalent |
| 47 | FFT.cpp: RADIX2_BFLY_double_buffer_quarter_onlycompute | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | **DROPPED** | No source-level equivalent |
| 48 | FFT.cpp: FFT_Stage1_vectorstream_parameterize | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design block` | Translated; placed before function definition |
| 49 | FFT.cpp: FFT_Stage1_vectorstream_parameterize | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| 50 | FFT.cpp: FFT_Stage1_vectorstream_parameterize | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` | Translated; moved before the `for` loop |
| 51 | FFT.cpp: FFT_Stage2_vectorstreamIn_arrayOut_parametize | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| 52 | FFT.cpp: FFT_Stage2_vectorstreamIn_arrayOut_parametize (FFT_Stage2) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| 53 | FFT.cpp: FFT_Stage2_vectorstreamIn_arrayOut_parametize (FFT_Stage2) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` | Translated; moved before the `for` loop |
| 54 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design block` | Translated; placed before function definition |
| 55 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| 56 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS array_partition variable=data_0 type=cyclic factor=UF*2 dim=1` | **DROPPED** | No source-level equivalent |
| 57 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS bind_storage variable=data_0 type=RAM_2P impl=LUTRAM` | **DROPPED** | No source-level equivalent |
| 58 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS array_partition variable=data_1 type=cyclic factor=UF*2 dim=1` | **DROPPED** | No source-level equivalent |
| 59 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS bind_storage variable=data_1 type=RAM_2P impl=LUTRAM` | **DROPPED** | No source-level equivalent |
| 60 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS array_partition variable=data_2 type=cyclic factor=UF*2 dim=1` | **DROPPED** | No source-level equivalent |
| 61 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS bind_storage variable=data_2 type=RAM_2P impl=LUTRAM` | **DROPPED** | No source-level equivalent |
| 62 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS array_partition variable=data_3 type=cyclic factor=UF dim=1` | **DROPPED** | No source-level equivalent |
| 63 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS array_partition variable=data_4 type=cyclic factor=UF dim=1` | **DROPPED** | No source-level equivalent |
| 64 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS array_partition variable=data_5 type=cyclic factor=UF dim=1` | **DROPPED** | No source-level equivalent |
| 65 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS array_partition variable=data_6 type=cyclic factor=UF dim=1` | **DROPPED** | No source-level equivalent |
| 66 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS array_partition variable=data_7 type=cyclic factor=UF dim=1` | **DROPPED** | No source-level equivalent |
| 67 | FFT.cpp: FFT_DIT_spatial_unroll_CY_stream_vector | `#pragma HLS array_partition variable=data_8 type=cyclic factor=UF dim=1` | **DROPPED** | No source-level equivalent |
| 68 | FFT.cpp: FFT_TOP | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design top` | Translated; top-level function gets `#pragma hls_design top` |

---

## Table 2: Type and Header Changes

| Original (Vitis) | Replacement (Catapult) | Notes |
|---|---|---|
| `#include "ap_fixed.h"` | Removed (not used) | Code uses only `complex<float>`, no fixed-point types |
| `#include "hls_fft.h"` | Removed (not used) | Not referenced in the source |
| `#include <hls_stream.h>` | `#include "ac_channel.h"` | stream → channel |
| `#include "hls_vector.h"` | Template struct `ac_vector<T,N>` defined locally | hls::vector replaced by a simple struct with operator[] |
| `#include "hls_streamofblocks.h"` | Removed (not used) | Not referenced in the source |
| `hls::stream<T>` | `ac_channel<T>` | Stream communication |
| `hls::vector<complex<float>, UF*2>` | `vec16_cfloat` (typedef for `ac_vector<complex<float>,16>`) | Vector type equivalent |
| `ap_uint<N>` | `ac_int<N, false>` | Unsigned integer |
| `ap_int<N>` | `ac_int<N, true>` | (Not used in this design) |
| `x.range(hi, lo)` | `x.slc<hi-lo+1>(lo)` | Bit-select; but for single-bit `range(i,i)`, uses `x[i]` directly |

Note: The code uses `std::complex<float>` which is standard C++ and remains unchanged.

---

## Table 3: Behavioral / Performance Differences

| Aspect | Vitis Original | Catapult Translation | Impact |
|---|---|---|---|
| **Array partitioning** | ~15 `#pragma HLS array_partition` directives | All **DROPPED** | Catapult can set memory mapping via TCL; without TCL equivalents, memory access patterns may differ and throughput could be lower. The algorithm and results remain identical. |
| **Bind storage** | 3 `#pragma HLS bind_storage` (RAM_2P, LUTRAM) | All **DROPPED** | Catapult uses default memory mapping; resource type may differ |
| **Bind operations** | ~12 `#pragma HLS bind_op` directives | All **DROPPED** | Catapult uses default operator implementation; implementation may differ (fabric vs DSP) |
| **Stream type** | `#pragma HLS stream type=pipo` | **DROPPED** | Catapult channel defaults to FIFO behavior |
| **Performance annotations** | Multiple `#pragma HLS performance target_ti=...` | All **DROPPED** | Analysis-only in Vitis; no functional impact |
| **Dataflow** | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design block` | Catapult dataflow blocks are set per function, not per region. `disable_start_propagation` has no direct equivalent. The overall dataflow structure is preserved. |
| **PIPELINE II=1** | Running loops at II=1 | `#pragma hls_pipeline_init_interval 1` | Equivalent behavior |
| **UNROLL** | Full unroll or factor-based | `#pragma hls_unroll yes` or `#pragma hls_unroll yes factor=N` | Equivalent behavior |

---

## Testbench Result

**Status: Testbench passed successfully.**

The translated testbench compiles and runs without errors. The FFT_TOP function computes the FFT and the maximum error compared to the golden DFT reference is below the threshold of 1.0.

---

## Verification Checklist

- [x] No Vitis identifiers remaining: `ap_int`, `ap_uint`, `ap_fixed`, `hls::stream`, `#pragma HLS` removed
- [x] Top function `FFT_TOP` marked with `#pragma hls_design top`
- [x] All headers translated
- [x] All pragmas translated or dropped with documented reason
- [x] Algorithm, loop structure, bit widths, signedness, rounding unchanged
- [x] Function names and argument order preserved
- [x] Testbench checks unchanged; returns 0 on success, 1 on failure
- [x] All .cpp files pass clang++ syntax check
- [x] Testbench binary runs with timeout and exits 0