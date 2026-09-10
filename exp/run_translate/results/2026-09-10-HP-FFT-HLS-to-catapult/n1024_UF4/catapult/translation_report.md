# Translation Report: Vitis HLS to Catapult HLS
## Design: n1024_UF4, Top Function: FFT_TOP

## Table 1: Pragmas

| File | Original Pragmas | Action | Catapult Equivalent / Reason |
|------|-----------------|--------|------------------------------|
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_CY` | `#pragma HLS inline` | Translated to `#pragma hls_design inline` before function definition |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_CY` | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | DROPPED | No Catapult source-level equivalent. Resource binding done via TCL. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_CY` | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | DROPPED | No Catapult source-level equivalent. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_CY` | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | DROPPED | No Catapult source-level equivalent. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_CY` | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | DROPPED | No Catapult source-level equivalent. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_CY` | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | DROPPED | No Catapult source-level equivalent. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_CY` | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | DROPPED | No Catapult source-level equivalent. |
| FFT.cpp – `output_result_array_to_stream` | `#pragma HLS performance target_ti=... unit=cycle` (2 instances) | DROPPED | Catapult does not have source-level performance pragmas. Scheduling is constraint-driven via TCL. |
| FFT.cpp – `output_result_array_to_stream` – `PostP_Fwd_loop` | `#pragma HLS pipeline` (inside loop body) | Moved to `#pragma hls_pipeline_init_interval 1` (before loop) |
| FFT.cpp – `bit_reverse` | `#pragma HLS inline` | Translated to `#pragma hls_design inline` before template function definition |
| FFT.cpp – `bit_reverse` – inner loop | `#pragma HLS UNROLL` | Translated to `#pragma hls_unroll yes` (before inner for loop) |
| FFT.cpp – `reverse_input_stream_UF4` | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=1` | DROPPED | No source-level equivalent. Memory mapping is TCL in Catapult. |
| FFT.cpp – `reverse_input_stream_UF4` | `#pragma HLS array_partition variable=data_in_cyclic type=cyclic factor=UF dim=2` | DROPPED | Same reason. |
| FFT.cpp – `reverse_input_stream_UF4` – `READ_STREAM_INPUT` | `#pragma HLS pipeline` | Moved to `#pragma hls_pipeline_init_interval 1` (before loop) |
| FFT.cpp – `reverse_input_stream_UF4` – `READ_STREAM_INPUT` | `#pragma HLS performance target_ti=...` | DROPPED | No Catapult source equivalent. |
| FFT.cpp – `reverse_input_stream_UF4` – `READ_STREAM_INPUT` | `#pragma HLS array_partition variable=original type=complete dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `reverse_input_stream_UF4` – `READ_STREAM_INPUT` | `#pragma HLS array_partition variable=reversed type=complete dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `reverse_input_stream_UF4` – `FROM_BLOCK_TO_CYCLIC` | `#pragma HLS pipeline` | Moved to `#pragma hls_pipeline_init_interval 1` (before loop) |
| FFT.cpp – `reverse_input_stream_UF4` – `FROM_BLOCK_TO_CYCLIC` | `#pragma HLS performance target_ti=...` | DROPPED | No Catapult source equivalent. |
| FFT.cpp – `reverse_input_stream_UF4` – `FROM_BLOCK_TO_CYCLIC` | `#pragma HLS array_partition variable=offset type=complete dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `reverse_input_stream_UF4` – `FROM_BLOCK_TO_CYCLIC` | `#pragma HLS array_partition variable=cyclic_offset type=complete dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `reverse_input_stream_UF4` – `FROM_BLOCK_TO_CYCLIC` | `#pragma HLS array_partition variable=block_data type=complete dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `reverse_input_stream_UF4` – `FROM_BLOCK_TO_CYCLIC` | `#pragma HLS array_partition variable=cyclic_data type=complete dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `reverse_input_stream_UF4` – `STREAM_OUT_REVERSE` | `#pragma HLS pipeline` | Moved to `#pragma hls_pipeline_init_interval 1` (before loop) |
| FFT.cpp – `reverse_input_stream_UF4` – `STREAM_OUT_REVERSE` | `#pragma HLS performance target_ti=...` | DROPPED | No Catapult source equivalent. |
| FFT.cpp – `FFT_stage_spatial_unroll` – `L_Pair_loop` | `#pragma HLS performance target_ti=...` | DROPPED | No Catapult source equivalent. |
| FFT.cpp – `FFT_stage_spatial_unroll` – `L_Pair_loop` | `#pragma HLS unroll factor=UF>>(stage-1)` | Translated to `#pragma hls_unroll factor=UF>>(stage-1)` (before L_Pair_loop) |
| FFT.cpp – `FFT_stage_spatial_unroll` – `L_Pair_loop` | `#pragma HLS pipeline` | Moved to `#pragma hls_pipeline_init_interval 1` (before L_Group_loop) |
| FFT.cpp – `FFT_stage_spatial_unroll` | `#pragma HLS bind_op variable=index op=mul impl=fabric` (3 instances) | DROPPED | No Catapult source-level equivalent. |
| FFT.cpp – `FFT_stage_spatial_unroll` – final else branch | `#pragma HLS performance target_ti=...` | DROPPED | No Catapult source equivalent. |
| FFT.cpp – `FFT_stage_spatial_unroll` – final else branch | `#pragma HLS unroll factor=UF` | Translated to `#pragma hls_unroll factor=UF` (before inner loop) |
| FFT.cpp – `FFT_stage_spatial_unroll` – final else branch | `#pragma HLS pipeline` | Moved to `#pragma hls_pipeline_init_interval 1` (before inner loop) |
| FFT.cpp – `FFT_stage_spatial_unroll` – final else branch | `#pragma HLS array_partition variable=twiddles complete` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_stage_spatial_unroll` – bflySize==FFT_NUM branch | `#pragma HLS pipeline` | Moved to `#pragma hls_pipeline_init_interval 1` (before loop) |
| FFT.cpp – `FFT_stage_spatial_unroll` – bflySize==FFT_NUM branch | `#pragma HLS unroll factor=UF` | Translated to `#pragma hls_unroll factor=UF` (before loop) |
| FFT.cpp – `FFT_stage_spatial_unroll` – bflySize==FFT_NUM branch | `#pragma HLS performance target_ti=...` | DROPPED | No Catapult source equivalent. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_onlycompute` | `#pragma HLS inline` | Translated to `#pragma hls_design inline` before function definition |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_onlycompute` | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | DROPPED | No source-level equivalent. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_onlycompute` | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | DROPPED | No source-level equivalent. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_onlycompute` | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | DROPPED | No source-level equivalent. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_onlycompute` | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | DROPPED | No source-level equivalent. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_onlycompute` | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | DROPPED | No source-level equivalent. |
| FFT.cpp – `RADIX2_BFLY_double_buffer_quarter_onlycompute` | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_Stage1_vectorstream_parameterize` | `#pragma HLS dataflow disable_start_propagation` | Replaced with `#pragma hls_design block` before function definition | `disable_start_propagation` is not expressible in Catapult. |
| FFT.cpp – `FFT_Stage1_vectorstream_parameterize` | `#pragma HLS performance target_ti=...` | DROPPED | No Catapult source equivalent. |
| FFT.cpp – `FFT_Stage1_vectorstream_parameterize` | `#pragma HLS pipeline` (inside loop body) | Moved to `#pragma hls_pipeline_init_interval 1` (before loop) |
| FFT.cpp – `FFT_Stage2_vectorstreamIn_arrayOut_parametize` | `#pragma HLS performance target_ti=...` (2 instances) | DROPPED | No Catapult source equivalent. |
| FFT.cpp – `FFT_Stage2_vectorstreamIn_arrayOut_parametize` | `#pragma HLS pipeline` | Moved to `#pragma hls_pipeline_init_interval 1` (before loop) |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS dataflow disable_start_propagation` | Replaced with `#pragma hls_design block` before function definition | `disable_start_propagation` not expressible. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS performance target_ti=...` | DROPPED | No Catapult source equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS array_partition variable=data_0 type=cyclic factor=UF*2 dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS bind_storage variable=data_0 type=RAM_2P impl=LUTRAM` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS array_partition variable=data_1 type=cyclic factor=UF*2 dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS bind_storage variable=data_1 type=RAM_2P impl=LUTRAM` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS array_partition variable=data_2 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS array_partition variable=data_3 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS array_partition variable=data_4 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS array_partition variable=data_5 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS array_partition variable=data_6 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS array_partition variable=data_7 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_DIT_spatial_unroll_CY_stream_vector` | `#pragma HLS array_partition variable=data_8 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent. |
| FFT.cpp – `FFT_TOP` | `#pragma HLS dataflow disable_start_propagation` | DROPPED; `#pragma hls_design top` used instead | The top function is marked `hls_design top`. Catapult does not have fine-grained dataflow regions inside functions; hierarchy is set per function. |

## Table 2: Header and Type Changes

| Original (Vitis) | Replacement (Catapult) | Notes |
|------------------|------------------------|-------|
| `#include "ap_fixed.h"` | Removed | Not used in design. Only `complex<float>` and `ap_uint` were used. |
| `#include "hls_fft.h"` | Removed | Not used in design. |
| `#include <hls_stream.h>` | `#include "ac_channel.h"` | |
| `#include "hls_vector.h"` | Replaced by custom `vec_wrapper<T,N>` template | Catapult ac_types has no vector type; a simple struct wrapper is used. |
| `#include "hls_streamofblocks.h"` | Removed | Not used in design. |
| `hls::stream<T>` | `ac_channel<T>` | |
| `hls::vector<T, N>` | `vec_wrapper<T, N>` | Custom template defined in FFT.h. Supports `operator[]`. |
| `ap_uint<N>` | `ac_int<N, false>` | Signedness preserved (unsigned). |
| `x.range(hi, lo)` (for single-bit range bit_i,bit_i) | `x[bit_i]` (single bit access) | `slc<1>(bit_i)` also valid; direct bit access is simpler. |
| `#include "hls_fft.h"` | Removed | Not used in the design code. |

## Table 3: Behavioral/Performance Impact Analysis

| Change | Impact | Notes |
|--------|--------|-------|
| All `#pragma HLS bind_op` (12 instances) dropped | Performance: Could affect datapath resource allocation | Catapult scheduling heuristics differ; resource bindings must be specified via TCL |
| All `#pragma HLS bind_storage` (2 instances) dropped | Performance: Could affect memory implementation | Catapult memory mapping is TCL-driven |
| All `#pragma HLS array_partition` (~20 instances) dropped | Performance: Could affect memory bandwidth | Catapult partitions memory at mapping time via TCL directives, not source code |
| `#pragma HLS performance` (10+ instances) dropped | Performance: Scheduling targets lost | Catapult uses clock constraints and loop bounds instead |
| `#pragma HLS dataflow disable_start_propagation` dropped | Performance: Dataflow pipelining behavior may differ | Catapult hierarchical block scheduling differs from Vitis dataflow. Sub-functions marked as `block` but inter-block pipelining behavior may not match exactly |
| `#pragma HLS DEPENDENCE` | Not used in this design | N/A |
| `#pragma HLS INTERFACE` | Not used in this design | N/A |
| `#pragma HLS LOOP_TRIPCOUNT` | Not used in this design | N/A |
| `#pragma HLS RESOURCE` | Not used in this design | N/A |
| `hls::vector<T,N>` → `vec_wrapper<T,N>` | No behavioral impact | Functionally identical array-style access wrapper |
| `ap_uint<N>` → `ac_int<N, false>` | No behavioral impact | Same signedness, same bit width |
| Stream types: `hls::stream` → `ac_channel` | No behavioral impact | `read()` and `write()` APIs are identical |
| Moving pipeline pragmas from inside loop to before loop | No behavioral impact | Placement change only; semantics preserved |
| Catapult default C++11 | No behavioral impact | Code is C++11 compatible |

## Testbench Result

**PASSED** — The testbench compiled and ran successfully with max error = 0.0000, returning exit code 0.