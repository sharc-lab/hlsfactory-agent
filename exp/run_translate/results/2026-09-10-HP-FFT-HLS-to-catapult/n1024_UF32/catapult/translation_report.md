# Translation Report: n1024_UF32 (Vitis HLS → Catapult HLS)

## Overview
**Design name:** n1024_UF32  
**Top-level function:** FFT_TOP  
**Description:** FFT with spatial unrolling factor UF=32, N=1024 points

---

## Table 1: Pragma Translation

Every `#pragma HLS ...` found in the original source and its disposition in the Catapult output.

| File | Original Pragma | Translation | Notes |
|------|----------------|-------------|-------|
| FFT.cpp | `#pragma HLS inline` (in `RADIX2_BFLY_double_buffer_quarter_CY`) | `#pragma hls_design inline` | Moved before function definition |
| FFT.cpp | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | DROPPED | No Catapult source-level equivalent; resource binding is done via TCL |
| FFT.cpp | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (in `output_result_array_to_stream`) | DROPPED | Catapult has performance constraints but not the same syntax |
| FFT.cpp | `#pragma HLS pipeline` (in `output_result_array_to_stream`) | `#pragma hls_pipeline_init_interval 1` | Moved before the loop |
| FFT.cpp | `#pragma HLS inline` (in `bit_reverse`) | `#pragma hls_design inline` | Moved before function definition |
| FFT.cpp | `#pragma HLS UNROLL` (in `bit_reverse`) | `#pragma hls_unroll yes` | Moved before the loop |
| FFT.cpp | `#pragma HLS array_partition variable=data_rev_stream type=complete dim=1` | DROPPED | Partitioning set in Catapult TCL |
| FFT.cpp | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=1` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=data_rev_stream type=complete dim=2` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=2` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS stream type=pipo variable=data_in_cyclic depth=3` | DROPPED | Streaming directives are TCL in Catapult |
| FFT.cpp | `#pragma HLS stream type=pipo variable=data_rev_stream depth=3` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS pipeline II=FFT_NUM/(2*UF)` (function-level in `reverse_input_stream_UF32`) | `#pragma hls_pipeline_init_interval 1` on READ_STREAM_INPUT | Vitis function-level pipeline; placed on first loop in Catapult |
| FFT.cpp | `#pragma HLS pipeline II=1` (in `READ_STREAM_INPUT` loop) | (merged with above) | Covered by the pipeline pragma on the loop |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (in loops in `reverse_input_stream_UF32`) | DROPPED | Performance analysis directive, no Catapult equivalent |
| FFT.cpp | `#pragma HLS array_partition variable=original type=complete dim=1` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=reversed type=complete dim=1` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS pipeline II=1` (in `FROM_BLOCK_TO_CYCLIC_SIMPLE`) | `#pragma hls_pipeline_init_interval 1` | Moved before the loop |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (in `FROM_BLOCK_TO_CYCLIC_SIMPLE`) | DROPPED | Performance analysis directive |
| FFT.cpp | `#pragma HLS pipeline II=1` (in `STREAM_OUT_REVERSE`) | `#pragma hls_pipeline_init_interval 1` | Moved before the loop |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (in `STREAM_OUT_REVERSE`) | DROPPED | Performance analysis directive |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (in `FFT_stage_spatial_unroll`) | DROPPED | Performance analysis directive |
| FFT.cpp | `#pragma HLS unroll factor=UF>>(stage-1)` (in `FFT_stage_spatial_unroll`, bflyStep < UF case) | `#pragma hls_unroll (UF>>(stage-1))` | Moved before the loop |
| FFT.cpp | `#pragma HLS pipeline` (in `FFT_stage_spatial_unroll`, bflyStep < UF case) | `#pragma hls_pipeline_init_interval 1` | Moved before the loop |
| FFT.cpp | `#pragma HLS bind_op variable=index op=mul impl=fabric` (in `FFT_stage_spatial_unroll`) | DROPPED | Resource binding is TCL in Catapult |
| FFT.cpp | `#pragma HLS pipeline` (in `FFT_stage_spatial_unroll`, bflySize==FFT_NUM case) | `#pragma hls_pipeline_init_interval 1` | Moved before the loop |
| FFT.cpp | `#pragma HLS unroll factor=UF` (in `FFT_stage_spatial_unroll`, bflySize==FFT_NUM case) | `#pragma hls_unroll UF` | Moved before the loop |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (bflySize==FFT_NUM case) | DROPPED | Performance analysis directive |
| FFT.cpp | `#pragma HLS pipeline` (in `FFT_stage_spatial_unroll`, else case, inner loop) | `#pragma hls_pipeline_init_interval 1` | Moved before the inner loop |
| FFT.cpp | `#pragma HLS unroll factor=UF` (in `FFT_stage_spatial_unroll`, else case, inner loop) | `#pragma hls_unroll UF` | Moved before the inner loop |
| FFT.cpp | `#pragma HLS array_partition variable=twiddles complete` (in `FFT_stage_spatial_unroll`) | DROPPED | Partitioning set in Catapult TCL |
| FFT.cpp | `#pragma HLS inline` (in `RADIX2_BFLY_double_buffer_quarter_onlycompute`) | `#pragma hls_design inline` | Moved before function definition |
| FFT.cpp | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | DROPPED | Resource binding is TCL in Catapult |
| FFT.cpp | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS dataflow` (in `FFT_Stage1_vectorstream_parameterize`) | `#pragma hls_design block` | Catapult hierarchical blocks per function |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Performance analysis directive |
| FFT.cpp | `#pragma HLS pipeline` (in `FFT_Stage1`) | `#pragma hls_pipeline_init_interval 1` | Moved before the loop |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (in `FFT_Stage2`) | DROPPED | Performance analysis directive |
| FFT.cpp | `#pragma HLS pipeline` (in `FFT_Stage2`) | `#pragma hls_pipeline_init_interval 1` | Moved before the loop |
| FFT.cpp | `#pragma HLS dataflow` (in `FFT_DIT_spatial_unroll_CY_stream_vector`) | `#pragma hls_design block` | Catapult hierarchical blocks per function |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Performance analysis directive |
| FFT.cpp | `#pragma HLS array_partition variable=data_0 type=cyclic factor=UF*2 dim=1` | DROPPED | Partitioning set in Catapult TCL |
| FFT.cpp | `#pragma HLS bind_storage variable=data_0 type=RAM_2P impl=LUTRAM` | DROPPED | Storage binding is TCL in Catapult |
| FFT.cpp | `#pragma HLS array_partition variable=data_1 type=cyclic factor=UF*2 dim=1` | DROPPED | Partitioning set in Catapult TCL |
| FFT.cpp | `#pragma HLS bind_storage variable=data_1 type=RAM_2P impl=LUTRAM` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=data_2 type=cyclic factor=UF*2 dim=1` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_storage variable=data_2 type=RAM_2P impl=LUTRAM` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=data_3 type=cyclic factor=UF*2 dim=1` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_storage variable=data_3 type=RAM_2P impl=LUTRAM` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=data_4 type=cyclic factor=UF*2 dim=1` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS bind_storage variable=data_4 type=RAM_2P impl=LUTRAM` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=data_5 type=cyclic factor=UF dim=1` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=data_6 type=cyclic factor=UF dim=1` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=data_7 type=cyclic factor=UF dim=1` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=data_8 type=cyclic factor=UF dim=1` | DROPPED | Same as above |
| FFT.cpp | `#pragma HLS dataflow` (in `FFT_TOP`) | `#pragma hls_design top` | FFT_TOP is the top function |

---

## Table 2: Type and Header Translation

| Original (Vitis) | Replacement (Catapult) | Notes |
|------------------|------------------------|-------|
| `#include "ap_fixed.h"` | Removed | Not used in the design |
| `#include "hls_fft.h"` | Removed | Not used in the design |
| `#include <hls_stream.h>` | `#include "ac_channel.h"` | |
| `#include "hls_vector.h"` | Custom `ac_vector<T,N>` struct | No direct Catapult equivalent; defined in FFT.h |
| `#include "hls_streamofblocks.h"` | Removed | Not used in the design |
| `ap_uint<N>` | `ac_int<N, false>` | Unsigned integer type |
| `hls::stream<T>` | `ac_channel<T>` | Stream channel type |
| `hls::vector<T,N>` | `ac_vector<T,N>` | Custom replacement struct with `operator[]` |
| `stream.read()` | `channel.read()` | Same API |
| `stream.write(v)` | `channel.write(v)` | Same API |
| `x.range(hi, lo)` | `x.slc<width>(lo)` or `x[i]` for single bit | Used in `bit_reverse`; single-bit access works directly with `operator[]` |

---

## Table 3: Behavioral and Performance Impact Notes

| Item | Impact | Explanation |
|------|--------|-------------|
| Dropped `#pragma HLS array_partition` | **Potential performance difference** | Array partitioning guides FPGA memory banking. Catapult sets these in TCL; the default behavior may differ. The static arrays `data_0` through `data_8` and `data_in_cyclic`/`data_rev_stream` may have different memory access patterns. |
| Dropped `#pragma HLS bind_storage` | **Potential resource difference** | RAM type binding (RAM_2P, LUTRAM) must be set in Catapult TCL. Default inference may choose different memory implementations. |
| Dropped `#pragma HLS bind_op` | **Potential performance difference** | Operation binding to fabric vs. DSP is set in Catapult TCL. Default operator scheduling may differ. |
| Dropped `#pragma HLS stream type=pipo` | **Loss of streaming hint** | Catapult channels (`ac_channel`) inherently implement FIFO-based streaming. The PIPO hint is not needed. |
| Dropped `#pragma HLS DATAFLOW` (replaced with `#pragma hls_design block`) | **Lossy** | Vitis DATAFLOW allows fine-grained dataflow regions within a function. Catapult hierarchical blocks are set per function, not per region. The design has been restructured so each dataflow region maps to a function with `#pragma hls_design block`. |
| Dropped `#pragma HLS performance target_ti=...` | **No functional impact** | Analysis-only directive; does not affect synthesis results. |
| `#pragma HLS pipeline II=FFT_NUM/(2*UF)` (function-level) | **Translates to pipeline on first loop** | Vitis allows function-level pipeline; Catapult requires pipeline on a specific loop. The first loop in the function gets the pipeline pragma. |

---

## Testbench Result

**Status: PASSED**

The testbench compiled and ran successfully. It produced the correct FFT output with a maximum error of **0.0000** (compared against the software golden model), well below the failure threshold of 1.0. The testbench returned exit code 0 with the message "Test PASSED".

**Compilation commands used:**
```
clang++ -std=c++17 -I/workspace/run_area/ac_types_include -I/workspace/run_area/output_design -w -fsyntax-only FFT.cpp
clang++ -std=c++17 -I/workspace/run_area/ac_types_include -I/workspace/run_area/output_design -w -fsyntax-only testbench.cpp
cd /workspace/run_area/output_design && clang++ -std=c++17 -I/workspace/run_area/ac_types_include -I/workspace/run_area/output_design -w *.cpp -o testbench.out && timeout 30s ./testbench.out
```