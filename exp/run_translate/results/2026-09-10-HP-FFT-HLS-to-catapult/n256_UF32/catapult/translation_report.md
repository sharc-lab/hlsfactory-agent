# Translation Report: n256_UF32 (Vitis HLS → Catapult HLS)

## Summary
The original Vitis HLS design `n256_UF32` (FFT size N=256, unrolling factor UF=32, stream-based vector interface) has been translated to an equivalent Siemens Catapult HLS design. The testbench passes with a maximum error of approximately 0.0000 (floating-point precision differences).

## Table 1: #pragma HLS Translation

| File | Original #pragma HLS | Translation | Notes |
|------|---------------------|-------------|-------|
| FFT.cpp | `#pragma HLS inline` (RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma hls_design inline` | Placed before function definition |
| FFT.cpp | `#pragma HLS inline` (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma hls_design inline` | Placed before function definition |
| FFT.cpp | `#pragma HLS inline` (bit_reverse) | `#pragma hls_design inline` | Placed before function definition |
| FFT.cpp | `#pragma HLS pipeline` (output_result_array_to_stream, PostP_Fwd_loop) | `#pragma hls_pipeline_init_interval 1` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS UNROLL` (bit_reverse, Loop_Reverse) | `#pragma hls_unroll yes` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS pipeline II=1` (reverse_input_stream_UF32, READ_STREAM_INPUT) | `#pragma hls_pipeline_init_interval 1` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS pipeline II=1` (reverse_input_stream_UF32, FROM_BLOCK_TO_CYCLIC_SIMPLE) | `#pragma hls_pipeline_init_interval 1` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS pipeline II=1` (reverse_input_stream_UF32, STREAM_OUT_REVERSE) | `#pragma hls_pipeline_init_interval 1` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS unroll factor=UF>>(stage-1)` (FFT_stage_spatial_unroll, L_Pair_loop) | `#pragma hls_unroll (UF>>(stage-1))` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS pipeline` (FFT_stage_spatial_unroll, L_Pair_loop) | `#pragma hls_pipeline_init_interval 1` | Combined with unroll before loop |
| FFT.cpp | `#pragma HLS unroll factor=UF` (FFT_stage_spatial_unroll, R_Group_loop_bflySize_equal_FFT_NUM) | `#pragma hls_unroll UF` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS pipeline` (FFT_stage_spatial_unroll, R_Group_loop_bflySize_equal_FFT_NUM) | `#pragma hls_pipeline_init_interval 1` | Combined with unroll before loop |
| FFT.cpp | `#pragma HLS unroll factor=UF` (FFT_stage_spatial_unroll, R_Group_loop) | `#pragma hls_unroll UF` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS pipeline` (FFT_stage_spatial_unroll, R_Group_loop) | `#pragma hls_pipeline_init_interval 1` | Combined with unroll before loop |
| FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (FFT_Stage1_vectorstream_parameterize) | `#pragma hls_design block` | Placed before function definition; disable_start_propagation has no Catapult equivalent |
| FFT.cpp | `#pragma HLS pipeline` (FFT_Stage1_vectorstream_parameterize, FFT_Stage1) | `#pragma hls_pipeline_init_interval 1` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS pipeline` (FFT_Stage2_vectorstreamIn_arrayOut_parametize, FFT_Stage2) | `#pragma hls_pipeline_init_interval 1` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma hls_design block` | Placed before function definition |
| FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (FFT_TOP) | `#pragma hls_design top` | Placed before function definition |
| FFT.cpp | `#pragma HLS array_partition variable=data_rev_stream type=complete dim=1` | **DROPPED** | No source-level equivalent in Catapult; memory mapping done via TCL |
| FFT.cpp | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=1` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS array_partition variable=data_rev_stream type=complete dim=2` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=2` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS stream type=pipo variable=data_in_cyclic depth=3` | **DROPPED** | No source-level equivalent in Catapult; stream configuration via TCL |
| FFT.cpp | `#pragma HLS stream type=pipo variable=data_rev_stream depth=3` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS pipeline II=FFT_NUM/(2*UF)` (function-level, reverse_input_stream_UF32) | **DROPPED** | Function-level pipeline not directly mappable; inner loops already have own pipeline pragmas |
| FFT.cpp | `#pragma HLS pipeline II=1` (reverse_input_stream_UF32, READ_STREAM_INPUT inner) | `#pragma hls_pipeline_init_interval 1` | Placed before the `for` loop |
| FFT.cpp | `#pragma HLS array_partition variable=original type=complete dim=1` | **DROPPED** | No source-level equivalent |
| FFT.cpp | `#pragma HLS array_partition variable=reversed type=complete dim=1` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (multiple instances) | **DROPPED** | Analysis-only directive, no Catapult equivalent |
| FFT.cpp | `#pragma HLS bind_op variable=... op=fsub impl=fabric` (multiple) | **DROPPED** | Resource binding done via TCL/constraints in Catapult |
| FFT.cpp | `#pragma HLS bind_op variable=... op=fadd impl=fabric` (multiple) | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS bind_op variable=... op=mul impl=fabric` (multiple) | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS array_partition variable=data_0 type=cyclic factor=UF*2 dim=1` | **DROPPED** | No source-level equivalent |
| FFT.cpp | `#pragma HLS bind_storage variable=data_0 type=RAM_2P impl=LUTRAM` | **DROPPED** | Resource binding via TCL |
| FFT.cpp | `#pragma HLS array_partition variable=data_1 type=cyclic factor=UF*2 dim=1` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS bind_storage variable=data_1 type=RAM_2P impl=LUTRAM` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS array_partition variable=data_2 type=cyclic factor=UF*2 dim=1` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS bind_storage variable=data_2 type=RAM_2P impl=LUTRAM` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS array_partition variable=data_3 type=cyclic factor=UF*2 dim=1` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS bind_storage variable=data_3 type=RAM_2P impl=LUTRAM` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS array_partition variable=data_4 type=cyclic factor=UF*2 dim=1` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS bind_storage variable=data_4 type=RAM_2P impl=LUTRAM` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS array_partition variable=data_5 type=cyclic factor=UF dim=1` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS array_partition variable=data_6 type=cyclic factor=UF dim=1` | **DROPPED** | Same reason |
| FFT.cpp | `#pragma HLS array_partition variable=twiddles complete` | **DROPPED** | Same reason |

## Table 2: Type and Header Changes

| Original (Vitis) | Replacement (Catapult) | Notes |
|------------------|------------------------|-------|
| `#include <hls_stream.h>` | `#include <ac_channel.h>` | Standard stream-to-channel replacement |
| `#include <hls_vector.h>` | Custom `vec_cf64` struct | No `ac_vector` available; defined a struct with `complex<float> data[64]` and `operator[]` |
| `#include <hls_streamofblocks.h>` | **Removed** | Not used in the code |
| `#include <hls_fft.h>` | **Removed** | Not used in the code |
| `#include "ap_fixed.h"` | **Removed** | Not used in the code (no ap_fixed types used) |
| `hls::stream<hls::vector<complex<float>, UF*2>>` | `ac_channel<vec_cf64>` | UF*2 = 64, no template parameter needed in channel type |
| `hls::vector<complex<float>, UF*2>` | `vec_cf64` | Custom struct with `data[64]` array and `operator[]` |
| `ap_int<N>` | — | Not used in original code |
| `ap_uint<N>` | `ac_int<N, false>` | Used in `bit_reverse` template and `reverse_input_stream_UF32` |
| `ap_uint<EXP2_FFT>` | `ac_int<EXP2_FFT, false>` | Same translation for template parameter EXP2_FFT=8 |
| `x.range(bit_i, bit_i)` | `x[bit_i]` | Single-bit range replaced with bit-select |
| `stream.read()` | `channel.read()` | Semantics identical |
| `stream.write(v)` | `channel.write(v)` | Semantics identical |
| `stream.empty()` | — | Not used in original code |
| `#include <cmath>` | `#include <cmath>` | Kept unchanged (standard library) |

## Table 3: Behavioral / Performance Differences

| Item | Impact | Explanation |
|------|--------|-------------|
| Dropped `#pragma HLS array_partition ...` (all instances) | **Potential performance difference** | Array partitioning is critical for parallel access in Vitis. In Catapult, this is configured via TCL or the GUI. The algorithm itself is unchanged, but the synthesized hardware may have different memory access patterns and throughput. |
| Dropped `#pragma HLS bind_storage ...` | **Potential performance difference** | Memory implementation selection is moved to TCL in Catapult. |
| Dropped `#pragma HLS bind_op ...` | **Minor performance difference** | Operator binding (fabric vs DSP) is configured differently in Catapult. |
| Dropped `#pragma HLS stream type=pipo depth=3` | **Minor performance difference** | Stream depth and type configuration need Catapult-specific TCL. |
| Dropped `#pragma HLS performance target_ti=...` | **No behavioral impact** | Analysis-only directive for loop timing estimation. |
| Dropped `#pragma HLS pipeline II=FFT_NUM/(2*UF)` (function-level) | **Minor pipeline difference** | Vitis allowed function-level pipeline with computed II. In Catapult, each inner loop retains its own pipeline pragma. The sequential loop execution model is preserved. |
| `#pragma HLS dataflow` → `#pragma hls_design block` | **Structural difference** | Vitis DATAFLOW creates a dataflow region within a function. Catapult hierarchical blocks are per-function. The `disable_start_propagation` option has no equivalent in Catapult. |
| `hls::vector` → `vec_cf64` struct | **No behavioral impact** | Custom struct preserves element access semantics. The struct is trivially copyable and works with `ac_channel`. |

## Testbench Result
**PASSED** - The testbench compiled and ran successfully with max error ~0.0000 (floating-point precision). The design produces numerically identical results to the original.