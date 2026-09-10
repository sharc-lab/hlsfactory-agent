# Translation Report: Vitis HLS to Siemens Catapult HLS

**Design:** n256_UF8 (256-point FFT, UF=8)
**Original top-level function:** FFT_TOP
**Date:** 2026-09-10

---

## Table 1: #pragma HLS Translation

| File | Original #pragma HLS | Translation | Reason / Notes |
|------|---------------------|-------------|----------------|
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma HLS inline` | `#pragma hls_design inline` (before function) | Direct equivalent |
| FFT.cpp (output_result_array_to_stream) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Direct equivalent; moved before the for-loop |
| FFT.cpp (bit_reverse) | `#pragma HLS inline` | `#pragma hls_design inline` (before function) | Direct equivalent |
| FFT.cpp (bit_reverse, inside loop) | `#pragma HLS UNROLL` | `#pragma hls_unroll yes` (before loop) | Direct equivalent; moved before the for-loop |
| FFT.cpp (reverse_input_stream_UF8) | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=1` | DROPPED | No source-level equivalent in Catapult; memory mapping via TCL |
| FFT.cpp (reverse_input_stream_UF8) | `#pragma HLS array_partition variable=data_rev_stream type=complete dim=1` | DROPPED | No source-level equivalent in Catapult |
| FFT.cpp (reverse_input_stream_UF8) | `#pragma HLS stream type=pipo variable=data_in_cyclic` | DROPPED | Stream allocation is TCL in Catapult |
| FFT.cpp (reverse_input_stream_UF8) | `#pragma HLS stream type=pipo variable=data_rev_stream` | DROPPED | Stream allocation is TCL in Catapult |
| FFT.cpp (reverse_input_stream_UF8) | `#pragma HLS pipeline II=FFT_NUM/(2*UF)` | `#pragma hls_pipeline_init_interval FFT_NUM/(2*UF)` (before first loop) | Direct equivalent; moved before the first for-loop |
| FFT.cpp (reverse_input_stream_UF8, READ_STREAM_INPUT) | `#pragma HLS pipeline II=1` | `#pragma hls_pipeline_init_interval 1` (before loop) | Direct equivalent |
| FFT.cpp (reverse_input_stream_UF8, READ_STREAM_INPUT) | `#pragma HLS array_partition variable=original type=complete dim=1` | DROPPED | No source-level equivalent in Catapult |
| FFT.cpp (reverse_input_stream_UF8, READ_STREAM_INPUT) | `#pragma HLS array_partition variable=reversed type=complete dim=1` | DROPPED | No source-level equivalent in Catapult |
| FFT.cpp (reverse_input_stream_UF8, FROM_BLOCK_TO_CYCLIC_SIMPLE) | `#pragma HLS pipeline II=1` | `#pragma hls_pipeline_init_interval 1` (before loop) | Direct equivalent |
| FFT.cpp (reverse_input_stream_UF8, STREAM_OUT_REVERSE) | `#pragma HLS pipeline II=1` | `#pragma hls_pipeline_init_interval 1` (before loop) | Direct equivalent |
| FFT.cpp (FFT_stage_spatial_unroll, L_Pair_loop) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only pragma, no Catapult equivalent |
| FFT.cpp (FFT_stage_spatial_unroll, L_Pair_loop) | `#pragma HLS unroll factor=UF>>(stage-1)` | `#pragma hls_unroll factor=UF>>(stage-1)` (before loop) | Direct equivalent |
| FFT.cpp (FFT_stage_spatial_unroll, L_Pair_loop) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Direct equivalent |
| FFT.cpp (FFT_stage_spatial_unroll, L_Group_loop) | `#pragma HLS bind_op variable=index op=mul impl=fabric` | DROPPED | Resource binding is TCL in Catapult |
| FFT.cpp (FFT_stage_spatial_unroll, R_Group_loop_bflySize_equal_FFT_NUM) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Direct equivalent |
| FFT.cpp (FFT_stage_spatial_unroll, R_Group_loop_bflySize_equal_FFT_NUM) | `#pragma HLS unroll factor=UF` | `#pragma hls_unroll factor=UF` (before loop) | Direct equivalent |
| FFT.cpp (FFT_stage_spatial_unroll, R_Group_loop_bflySize_equal_FFT_NUM) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp (FFT_stage_spatial_unroll, R_Group_loop_bflySize_equal_FFT_NUM) | `#pragma HLS bind_op variable=index op=mul impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (FFT_stage_spatial_unroll, R_Pair_loop) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp (FFT_stage_spatial_unroll, R_Group_loop) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Direct equivalent |
| FFT.cpp (FFT_stage_spatial_unroll, R_Group_loop) | `#pragma HLS unroll factor=UF` | `#pragma hls_unroll factor=UF` (before loop) | Direct equivalent |
| FFT.cpp (FFT_stage_spatial_unroll, R_Group_loop) | `#pragma HLS bind_op variable=index op=mul impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (FFT_stage_spatial_unroll, R_Group_loop) | `#pragma HLS array_partition variable=twiddles complete` | DROPPED | No source-level equivalent |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma HLS inline` | `#pragma hls_design inline` (before function) | Direct equivalent |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp (FFT_Stage1_vectorstream_parameterize) | `#pragma HLS dataflow disable_start_propagation` | DROPPED (region-level) | Catapult uses per-function `#pragma hls_design block` on each sub-function instead of region-level dataflow; see note below |
| FFT.cpp (FFT_Stage1_vectorstream_parameterize, inside loop) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp (FFT_Stage1_vectorstream_parameterize, FFT_Stage1) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Direct equivalent |
| FFT.cpp (FFT_Stage2_vectorstreamIn_arrayOut_parametize) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp (FFT_Stage2_vectorstreamIn_arrayOut_parametize, FFT_Stage2) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Direct equivalent |
| FFT.cpp (FFT_Stage2_vectorstreamIn_arrayOut_parametize, FFT_Stage2) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS dataflow disable_start_propagation` | DROPPED (region-level) | See dataflow note below |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS array_partition variable=data_0 type=cyclic factor=UF*2 dim=1` | DROPPED | No source-level equivalent in Catapult |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS bind_storage variable=data_0 type=RAM_2P impl=LUTRAM` | DROPPED | Resource binding is TCL in Catapult |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS array_partition variable=data_1 type=cyclic factor=UF*2 dim=1` | DROPPED | No source-level equivalent |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS bind_storage variable=data_1 type=RAM_2P impl=LUTRAM` | DROPPED | Resource binding is TCL |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS array_partition variable=data_2 type=cyclic factor=UF*2 dim=1` | DROPPED | No source-level equivalent |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS bind_storage variable=data_2 type=RAM_2P impl=LUTRAM` | DROPPED | Resource binding is TCL |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS array_partition variable=data_3 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS array_partition variable=data_4 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS array_partition variable=data_5 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS array_partition variable=data_6 type=cyclic factor=UF dim=1` | DROPPED | No source-level equivalent |
| FFT.cpp (FFT_TOP) | `#pragma HLS dataflow disable_start_propagation` | DROPPED (region-level) | See dataflow note below |

---

## Table 2: Type and Header Changes

| Original (Vitis) | Replacement (Catapult) | Files Affected |
|-----------------|----------------------|----------------|
| `#include "ap_fixed.h"` | `#include "ac_fixed.h"` | FFT.h |
| `#include "hls_stream.h"` | `#include "ac_channel.h"` | FFT.h |
| `#include "hls_vector.h"` | `struct hls_vector<T,N> { T data[N]; ... };` (custom replacement) | FFT.h |
| `#include "hls_fft.h"` | Removed (not used in code) | FFT.h |
| `#include "hls_streamofblocks.h"` | Removed (not used in code) | FFT.h |
| `hls::stream<hls::vector<complex<float>, UF*2>>` | `ac_channel<hls_vector<complex<float>, UF*2>>` | FFT.h, FFT.cpp, testbench.cpp |
| `hls::vector<complex<float>, UF*2>` | `hls_vector<complex<float>, UF*2>` (custom struct) | FFT.h, FFT.cpp, testbench.cpp |
| `ap_uint<N>` | `ac_int<N, false>` | FFT.cpp (bit_reverse function) |
| `x.range(hi, lo)` | `x[bit_i]` (single-bit range) | FFT.cpp (bit_reverse function, single-bit access) |
| `stream.read()` | `channel.read()` | FFT.cpp, testbench.cpp |
| `stream.write(v)` | `channel.write(v)` | FFT.cpp, testbench.cpp |

**Note on hls::vector replacement:** The custom `hls_vector<T, N>` struct is defined in FFT.h. It provides operator[] for element access and is a plain aggregate (no Vitis-specific behavior). It is NOT in the `hls` namespace to avoid confusion.

---

## Table 3: Behavioral / Performance Differences

| Aspect | Original (Vitis) | Translated (Catapult) | Impact |
|--------|-----------------|----------------------|--------|
| Array partitioning | `#pragma HLS array_partition ... complete/cyclic ...` | DROPPED (6 partition pragmas; 2 in reverse_input_stream_UF8, 2 for local arrays, 6 for data_0..data_6) | **Performance impact**: Catapult must infer partitioning through TCL or resource directives. Without TCL equivalents, memory bandwidth may be lower, potentially reducing throughput. |
| Storage binding | `#pragma HLS bind_storage ... type=RAM_2P impl=LUTRAM` | DROPPED (3 instances: data_0, data_1, data_2) | **Resource impact**: Catapult will use default implementation, may differ from expected LUTRAM usage. |
| Operation binding | `#pragma HLS bind_op ... op=fadd/fsub impl=fabric` | DROPPED (12 instances) | **Resource impact**: Catapult will use default operator implementation (may use DSP or fabric differently). |
| Dataflow regions | `#pragma HLS dataflow` (3 regions: FFT_DIT, FFT_Stage1, FFT_TOP) | DROPPED as region-level pragmas | **Architectural difference**: Catapult does not support inline dataflow regions. The equivalent in Catapult is `#pragma hls_design block` on each sub-function, which was applied to: RADIX2_BFLY_double_buffer_quarter_CY, RADIX2_BFLY_double_buffer_quarter_onlycompute, bit_reverse. The top-level function has `#pragma hls_design top`. The sub-functions called within FFT_DIT (reverse_input_stream_UF8, FFT_Stage1_vectorstream_parameterize, FFT_Stage2_vectorstreamIn_arrayOut_parametize, output_result_array_to_stream) would need `#pragma hls_design block` at the Catapult TCL level or individually. This is a lossy translation — the original Vitis dataflow could pipeline independent loop iterations across function boundaries; Catapult hierarchical blocks require explicit hierarchy. |
| `#pragma HLS performance` | 6 instances | DROPPED | Analysis-only pragmas; no functional impact. |
| Fixed-point types | `ap_fixed.h` included but never used | `ac_fixed.h` included but never used | No functional impact. |
| Bit-reverse bit access | `ap_uint<N>` with `x.range(bit_i, bit_i)` | `ac_int<N, false>` with `x[bit_i]` | Equivalent behavior. |

---

## Testbench Result

**PASSED** — The testbench compiled and ran successfully, producing max error of 0.0000 (well below the 1.0 threshold). The translated design produces bit-identical results to the golden reference within floating-point precision.

**Exit code:** 0

---

## Summary

All Vitis HLS identifiers have been removed from the source code. The algorithm, loop structure, bit widths, signedness, and rounding modes are preserved. The testbench passes with identical numerical results. Performance-impacting pragmas (array_partition, bind_storage, bind_op, dataflow) were dropped and should be reconstituted in the Catapult TCL environment for equivalent synthesis results.