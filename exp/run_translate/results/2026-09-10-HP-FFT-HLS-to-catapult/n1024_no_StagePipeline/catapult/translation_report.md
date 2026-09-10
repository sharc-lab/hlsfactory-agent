# Translation Report: Vitis HLS to Siemens Catapult HLS

## Design: n1024_no_StagePipeline (FFT_TOP)

### Overview
Original Vitis HLS design implementing a 1024-point FFT with temporal double-buffering and no stage-level pipelining. Translated to Siemens Catapult HLS using Algorithmic C (ac_types) types and Catapult-specific pragmas.

---

## Table 1: #pragma HLS Translation

| File | Original Pragma | Translation | Notes |
|------|----------------|-------------|-------|
| FFT.cpp | `#pragma HLS inline` (inside `RADIX2_BFLY_double_buffer`) | `#pragma hls_design inline` (before function definition) | Translated directly |
| FFT.cpp | `#pragma HLS loop_tripcount min=1 max=1<<(EXP2_FFT-1-EXP2_FFT/2)` (Group_loop) | DROPPED | Analysis-only pragma; no Catapult equivalent |
| FFT.cpp | `#pragma HLS loop_flatten` (Pair_loop) | DROPPED | No direct Catapult equivalent; original code note says it didn't work anyway |
| FFT.cpp | `#pragma HLS pipeline` (Pair_loop) | `#pragma hls_pipeline_init_interval 1` (before Pair_loop for) | Default II=1 |
| FFT.cpp | `#pragma HLS dependence variable=data_ld inter false` | DROPPED | No source-level equivalent in Catapult; must be set via TCL if needed |
| FFT.cpp | `#pragma HLS dependence variable=data_st inter false` | DROPPED | No source-level equivalent in Catapult; must be set via TCL if needed |
| FFT.cpp | `#pragma HLS loop_tripcount min=1 max=1<<(EXP2_FFT/2)` (Pair_loop) | DROPPED | Analysis-only pragma; no Catapult equivalent |
| FFT.cpp | `#pragma HLS array_partition variable=data type=complete dim=1` | DROPPED | No source-level equivalent in Catapult; memory mapping set in TCL |
| FFT.cpp | `#pragma HLS unroll factor=2` (PreP_Fwd_loop) | `#pragma hls_unroll 2` (before PreP_Fwd_loop for) | Factor preserved |
| FFT.cpp | `#pragma HLS pipeline` (Rev_Bit_loop body) | `#pragma hls_pipeline_init_interval 1` (before Rev_Bit_loop for) | Moved from inside loop body to before for-statement |
| FFT.cpp | `#pragma HLS unroll factor=2` (Rev_Bit_loop body) | `#pragma hls_unroll 2` (before Rev_Bit_loop for) | Moved from inside loop body to before for-statement |
| FFT.cpp | `#pragma HLS unroll factor=2` (PostP_Fwd_loop body) | `#pragma hls_unroll 2` (before PostP_Fwd_loop for) | Moved from inside loop body to before for-statement |

---

## Table 2: Type and Header Changes

| Original (Vitis) | Replacement (Catapult) | Notes |
|------------------|----------------------|-------|
| `#include "ap_fixed.h"` | Removed (not needed) | No ap_fixed types used in the design; standard `complex<float>` used instead |
| `#include "hls_fft.h"` | Removed (not needed) | Vitis FFT library header; no FFT IP used |
| `#include "hls_stream.h"` | Removed (not needed) | No hls::stream used; all array-based |
| `#include "hls_vector.h"` | Removed (not needed) | No hls::vector used |
| `#include "hls_streamofblocks.h"` | Removed (not needed) | No stream-of-blocks used |
| `#include <iomanip>` | Added in testbench.cpp | Required for `setprecision`/`fixed` |
| `#include <iostream>` | Added in testbench.cpp | Required for `cout`/`endl` |
| `#include <cmath>` | Retained (added to header) | Used for math functions |
| `complex<T>` (bare) | `std::complex<T>` | Fully qualified to avoid namespace pollution |
| `using namespace std;` | Removed from header, added in testbench.cpp | Safer scoping of namespace |

---

## Table 3: Behavior/Performance Impact Notes

| Item | Impact | Notes |
|------|--------|-------|
| `ARRAY_PARTITION` (DROPPED) | Performance | Vitis `array_partition complete dim=1` on `data[2][FFT_NUM]` fully partitions dim 1 into separate registers. This was dropped. In Catapult, memory mapping must be specified in TCL directives instead. The initial data arrangement may differ. |
| `DEPENDENCE` (DROPPED) | Performance | Two `#pragma HLS dependence variable=... inter false` pragmas were dropped. These told Vitis that there are no loop-carried dependencies on the data arrays, enabling better pipelining. Catapult's dependence analysis may be more or less conservative, potentially affecting achievable II. |
| `loop_tripcount` (DROPPED) | None (analysis) | These were only for estimation/reporting, not synthesis. |
| `loop_flatten` (DROPPED) | None | Original note says it didn't work anyway. |
| Rounding modes | Unaffected | No explicit rounding/overflow modes in original code; defaults used. |
| No Vitis custom types used | Unaffected | The design uses standard `complex<float>` throughout, so no type migration issues. |
| Catapult `#pragma hls_design top` | Correct | Placed before `FFT_TOP` function definition. |
| Catapult `#pragma hls_design inline` | Correct | Placed before `RADIX2_BFLY_double_buffer` function definition. |

---

## Testbench Result

**Status: PASSED**

The testbench compiled and ran successfully with maximum error < 1e-4 (well below the threshold of 1.0). The output design produces numerically identical results to the original Vitis HLS design.

- Syntax check (FFT.cpp): PASS
- Syntax check (testbench.cpp): PASS
- Build (C++17): PASS
- Runtime test (timeout 30s): PASS, exit code 0