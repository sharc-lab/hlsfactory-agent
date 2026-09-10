# Translation Report: n1024_original_C_style

## Overview
This document records every change made when translating the Vitis HLS design to Siemens Catapult HLS.

**Top-level function**: `FFT_TOP`
**Source files**: `FFT.cpp`, `FFT.h`, `testbench.cpp`

---

## Table 1: Pragma Translation

The original design used 3 `#pragma HLS` directives. All were `LOOP_TRIPCOUNT` analysis-only pragmas. Catapult has no equivalent for these, so they were removed.

| File | Original Pragma | Action | Catapult Equivalent | Notes |
|------|----------------|--------|---------------------|-------|
| FFT.cpp | `#pragma HLS loop_tripcount min=1 max=1<<(EXP2_FFT/2)` (Stage_3more_0) | DROPPED | No equivalent | Analysis-only pragma. Does not affect functionality. |
| FFT.cpp | `#pragma HLS loop_tripcount min=3 max=(1<<(EXP2_FFT-1-EXP2_FFT/2))-1` (Stage_3more_Group_loop) | DROPPED | No equivalent | Analysis-only pragma. Does not affect functionality. |
| FFT.cpp | `#pragma HLS loop_tripcount min=1 max=1<<(EXP2_FFT/2)` (Stage_3more_Pair_loop) | DROPPED | No equivalent | Analysis-only pragma. Does not affect functionality. |

### New pragmas added

| File | Pragma | Placement | Notes |
|------|--------|-----------|-------|
| FFT.cpp | `#pragma hls_design top` | Before `FFT_TOP` function definition | Required by Catapult to identify the top-level function |

---

## Table 2: Header and Type Translation

The original design included several Vitis headers, but none of the Vitis types (ap_int, ap_fixed, hls::stream, etc.) were actually used in the code. The only data types used were standard C++ types: `complex<float>`, `double`, `int`, `float`.

| Original Header | Replacement | Notes |
|----------------|-------------|-------|
| `"ap_fixed.h"` | Removed | Not used in the code |
| `"hls_fft.h"` | Removed | Not used in the code |
| `<hls_stream.h>` | Removed | Not used in the code |
| `"hls_vector.h"` | Removed | Not used in the code |
| `"hls_streamofblocks.h"` | Removed | Not used in the code |
| `<iomanip>`, `<iostream>` | Moved to FFT.h | Kept as standard headers for testbench compatibility |
| `<complex>`, `<cmath>` | Retained | Standard C++ headers, no change needed |

### Type Translation
No Vitis-specific types were used in the code. All types were standard C++:
- `complex<float>` → unchanged
- `double` → unchanged
- `int` → unchanged
- `float` → unchanged

No `ap_int`, `ap_uint`, `ap_fixed`, `ap_ufixed`, `hls::stream`, or other Vitis types appear anywhere in the code.

---

## Table 3: Behavioral and Performance Differences

| Item | Description | Impact |
|------|-------------|--------|
| Dropped LOOP_TRIPCOUNT pragmas | `#pragma HLS loop_tripcount` directives were removed (3 occurrences). These are analysis-only pragmas used by Vitis HLS for resource estimation. | **No functional impact.** These pragmas only affect Vitis HLS reporting metrics. Catapult does not support them. |
| Interface specification | The original design uses array-based interfaces (`complex<float> dataIn[FFT_NUM]`, `complex<float> dataOut[FFT_NUM]`). No `#pragma HLS INTERFACE` was present in the original code, so no interface translation was needed. | **No change.** Both Vitis HLS and Catapult handle array arguments as memory-mapped interfaces by default. |
| `#pragma hls_design top` added | Added before the `FFT_TOP` function definition in FFT.cpp. | **Required for Catapult.** No behavioral change. |

### Summary of Changes
1. **Headers**: Removed 5 unused Vitis headers from FFT.h. Added back standard library headers needed by testbench (iostream, iomanip).
2. **Pragmas**: Removed 3 LOOP_TRIPCOUNT pragmas (analysis-only). Added 1 `#pragma hls_design top` for Catapult.
3. **Top function marker**: Added `#pragma hls_design top` before `FFT_TOP`.
4. **Code**: No modifications to the algorithm, loop structure, bit widths, signedness, or rounding behavior.

---

## Testbench Result

The testbench compiled and ran successfully, producing the same output as the original design.

**Max error**: 0.0000
**Test result**: PASSED (return code 0)

The translated design is functionally identical to the original Vitis HLS design.