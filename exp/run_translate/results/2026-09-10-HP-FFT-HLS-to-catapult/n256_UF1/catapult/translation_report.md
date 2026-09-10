# Translation Report: n256_UF1 (Vitis HLS -> Catapult HLS)

## Overview
- **Design name**: n256_UF1
- **Original tool**: Vitis HLS
- **Target tool**: Siemens Catapult HLS
- **Top-level function**: `FFT_TOP`
- **FFT size**: 256 (EXP2_FFT=8)
- **Unrolling factor (UF)**: 1

## Table 1: #pragma HLS Translation

Every original `#pragma HLS` directive from FFT.cpp is accounted for below.

| # | File | Original Vitis Pragrma | Replacement | Notes |
|---|------|----------------------|-------------|-------|
| 1 | FFT.cpp | `#pragma HLS inline` (RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma hls_design inline` before function def | Placement changed from inside body to before function |
| 2 | FFT.cpp | `#pragma HLS inline` (bit_reverse) | `#pragma hls_design inline` before function def | Placement changed from inside body to before function |
| 3 | FFT.cpp | `#pragma HLS inline` (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma hls_design inline` before function def | Placement changed from inside body to before function |
| 4 | FFT.cpp | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` (x4 across 2 functions) | **DROPPED** | No direct Catapult source-level equivalent; resource binding done via TCL |
| 5 | FFT.cpp | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` (x4 across 2 functions) | **DROPPED** | Same as above |
| 6 | FFT.cpp | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` (x2 across 2 functions) | **DROPPED** | Same as above |
| 7 | FFT.cpp | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` (x2 across 2 functions) | **DROPPED** | Same as above |
| 8 | FFT.cpp | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` (x2 across 2 functions) | **DROPPED** | Same as above |
| 9 | FFT.cpp | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` (x2 across 2 functions) | **DROPPED** | Same as above |
| 10 | FFT.cpp | `#pragma HLS bind_op variable=index op=mul impl=fabric` (x3 in FFT_stage_spatial_unroll) | **DROPPED** | Same as above |
| 11 | FFT.cpp | `#pragma HLS pipeline` (no II specified, multiple occurrences) | `#pragma hls_pipeline_init_interval 1` before the for-loop | Default II=1 assumed. Placement moved to line before loop. |
| 12 | FFT.cpp | `#pragma HLS pipeline II=1` (in reverse_input_stream_UF1, 3 occurrences) | `#pragma hls_pipeline_init_interval 1` before the for-loop | Direct translation |
| 13 | FFT.cpp | `#pragma HLS UNROLL` (bit_reverse inner loop) | `#pragma hls_unroll yes` before the for-loop | Direct translation |
| 14 | FFT.cpp | `#pragma HLS UNROLL factor=UF>>(stage-1)` (FFT_stage_spatial_unroll) | `#pragma hls_unroll (UF>>(stage-1))` before the for-loop | Factor expression preserved |
| 15 | FFT.cpp | `#pragma HLS UNROLL factor=UF` (FFT_stage_spatial_unroll, 2 occurrences) | `#pragma hls_unroll UF` before the for-loop | Direct translation |
| 16 | FFT.cpp | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=1` | **DROPPED** | No source-level equivalent in Catapult; memory mapping done via TCL |
| 17 | FFT.cpp | `#pragma HLS array_partition variable=data_in_cyclic type=cyclic factor=UF dim=2` | **DROPPED** | Same as above |
| 18 | FFT.cpp | `#pragma HLS array_partition variable=data_rev_stream type=complete dim=1` (implied by stream) | **DROPPED** | Same as above |
| 19 | FFT.cpp | `#pragma HLS array_partition variable=original type=complete dim=1` | **DROPPED** | Same as above |
| 20 | FFT.cpp | `#pragma HLS array_partition variable=reversed type=complete dim=1` | **DROPPED** | Same as above |
| 21 | FFT.cpp | `#pragma HLS array_partition variable=offset type=complete dim=1` | **DROPPED** | Same as above |
| 22 | FFT.cpp | `#pragma HLS array_partition variable=cyclic_offset type=complete dim=1` | **DROPPED** | Same as above |
| 23 | FFT.cpp | `#pragma HLS array_partition variable=block_data type=complete dim=1` | **DROPPED** | Same as above |
| 24 | FFT.cpp | `#pragma HLS array_partition variable=cyclic_data type=complete dim=1` | **DROPPED** | Same as above |
| 25 | FFT.cpp | `#pragma HLS array_partition variable=data_1..data_8 type=cyclic factor=UF dim=1` (8 lines) | **DROPPED** | Same as above |
| 26 | FFT.cpp | `#pragma HLS array_partition variable=twiddles complete` | **DROPPED** | Same as above |
| 27 | FFT.cpp | `#pragma HLS stream type=pipo variable=data_in_cyclic` | **DROPPED** | Streaming config done via TCL in Catapult |
| 28 | FFT.cpp | `#pragma HLS stream type=pipo variable=data_rev_stream` | **DROPPED** | Same as above |
| 29 | FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (reverse_input_stream_UF1) | Replaced with `#pragma hls_design block` before function def | Catapult uses per-function block hierarchy |
| 30 | FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (FFT_Stage1_vectorstream_parameterize) | Replaced with `#pragma hls_design block` before function def | Same as above |
| 31 | FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (FFT_DIT_spatial_unroll_CY_stream_vector) | Replaced with `#pragma hls_design block` before function def | Same as above |
| 32 | FFT.cpp | `#pragma HLS dataflow disable_start_propagation` (FFT_TOP) | Replaced with `#pragma hls_design top` before function def | Top level gets `hls_design top` |
| 33 | FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (multiple occurrences) | **DROPPED** | Analysis-only directive; no Catapult source equivalent |

## Table 2: Type and Header Changes

| Original | Replacement | Notes |
|----------|-------------|-------|
| `#include "ap_fixed.h"` | Removed | ap_fixed types not used in the design (only float) |
| `#include "hls_fft.h"` | Removed | Not used in the design |
| `#include "hls_stream.h"` | `#include <ac_channel.h>` | Standard replacement |
| `#include "hls_vector.h"` | Custom `ac_vector` template | No standard ac_types vector; defined locally in FFT.h |
| `#include "hls_streamofblocks.h"` | Removed | Not used |
| `#include <ap_int.h>` (via ap_fixed.h) | `#include <ac_int.h>` | Explicitly added for ac_int usage |
| `hls::stream<T>` | `ac_channel<T>` | Direct replacement |
| `hls::vector<complex<float>, N>` | `ac_vector<complex<float>, N>` | Custom struct with std::array<T,N> and operator[] |
| `ap_uint<N>` | `ac_int<N, false>` | Signedness preserved (unsigned) |
| `x.range(hi, lo)` (single-bit access) | `x[i]` | For single-bit ranges, replaced with bit-select operator |
| `stream.read()` | `channel.read()` | Same API (returns by value) |
| `stream.write(v)` | `channel.write(v)` | Same API |
| `hls::stream.empty()` | Not used in original | N/A |

## Table 3: Behavioral / Performance Impact Analysis

| Item | Impact | Explanation |
|------|--------|-------------|
| Dropped `#pragma HLS bind_op` | Medium - Performance | Catapult can apply resource binding via TCL directives (`directive set -REGISTER` / `-RESOURCE`). The source-level bind_op hints are lost but can be re-applied in the TCL flow. |
| Dropped `#pragma HLS array_partition` | Medium - Performance | Vitis array partitions are critical for memory bandwidth. In Catapult, memory partitioning is specified via TCL (`memory-map`). Arrays will be mapped to standard memory unless the TCL flow configures them. This is the most significant performance difference. |
| Dropped `#pragma HLS DEPENDENCE` | None | Not present in original design. |
| Dropped `#pragma HLS stream type=pipo` | Medium - Performance | Streaming configuration affects FIFO vs. ping-pong implementation. Must be set via TCL in Catapult. |
| Dropped `#pragma HLS performance` | Low | Analysis-only directives, no effect on synthesized logic. |
| Rounding/overflow modes | None | All computations use `float` (IEEE 754), not fixed-point. No rounding/overflow mode changes. |
| Signedness | None | Preserved exactly. `ap_int` -> `ac_int<N, true>`, `ap_uint` -> `ac_int<N, false>`. |
| Bit widths | None | All widths preserved exactly. |
| Loop structure | None | All loops and their bodies are identical. |
| DATAFLOW -> hierarchical blocks | Low | Catapult's `hls_design block` per function is equivalent but coarser-grained than Vitis dataflow regions. The same sub-functions are marked as blocks. |
| `ac_vector` custom type | Low | Behaves like `hls::vector<T,N>` with array-style access. Element access and stream read/write semantics are identical. |
| `#pragma HLS UNROLL` factor expression `UF>>(stage-1)` | None | Preserved exactly with parenthesized expression. |
| `#pragma HLS PIPELINE` default II | None | Vitis default II=1 matches Catapult default init_interval=1. |

## Testbench Result

**Status: PASSED**

- Compilation: All .cpp files passed syntax check with clang++ -std=c++17.
- Build: Linked successfully.
- Run: Testbench executed, computed FFT, compared against golden DFT model.
- Max error: 0.0000 (threshold: 1.0)
- Exit code: 0 ("Test PASSED")

The testbench checks and comparison logic are identical to the original Vitis version, with only type name substitutions applied.

## Summary

All Vitis-specific headers, types, and pragmas have been removed and replaced with their Catapult equivalents. The algorithm, loop structure, arithmetic, function signatures, and interface semantics are preserved. The testbench passes with identical numerical results. The primary performance differences arise from dropped array partitioning and resource binding hints, which must be reconfigured via Catapult TCL directives for optimal synthesis results.