# Translation Report: n256_original_C_style (Vitis HLS → Catapult HLS)

## Overview

- **Design**: n256_original_C_style — 256-point FFT (Radix-2, in-place)
- **Top function**: `FFT_TOP`
- **Algorithm**: Cooley–Tukey Radix-2 DIT FFT, fully sequential (no pipelining/unrolling)
- **Input interface**: `complex<float> dataIn[256]`, `complex<float> dataOut[256]` (arrays)

---

## Table 1: #pragma HLS Translation

| File | Original #pragma | Translation | Reason |
|---|---|---|---|
| FFT.cpp: Stage_3more_0 loop | `#pragma HLS loop_tripcount min=1 max=1<<(EXP2_FFT/2)` | **DROPPED** | `loop_tripcount` is an analysis-only hint in Vitis HLS. Catapult has no source-level equivalent; loop bounds are inferred from TCL or analysis. |
| FFT.cpp: Stage_3more_Group_loop | `#pragma HLS loop_tripcount min=3 max=(1<<(EXP2_FFT-1-EXP2_FFT/2))-1` | **DROPPED** | Same as above. Analysis-only pragma, no functional effect. |
| FFT.cpp: Stage_3more_Pair_loop | `#pragma HLS loop_tripcount min=1 max=1<<(EXP2_FFT/2)` | **DROPPED** | Same as above. Analysis-only pragma, no functional effect. |

No PIPELINE, UNROLL, DATAFLOW, INLINE, ARRAY_PARTITION, INTERFACE, BIND_STORAGE, RESOURCE, BIND_OP, or DEPENDENCE pragmas were present in the original design.

---

## Table 2: Type and Header Changes

| Original | Replacement | Notes |
|---|---|---|
| `#include "ap_fixed.h"` | **Removed** | Not used in any code; no ap_fixed types were instantiated. |
| `#include "hls_fft.h"` | **Removed** | Vitis FFT library header; not used (custom FFT implementation). |
| `#include <hls_stream.h>` | **Removed** | Not used; no hls::stream instances. |
| `#include "hls_vector.h"` | **Removed** | Not used; no hls::vector instances. |
| `#include "hls_streamofblocks.h"` | **Removed** | Not used; no stream_of_blocks instances. |
| `#include <complex>` | **Kept** | Standard C++ header, unchanged. |
| `#include <iomanip>` | **Kept** | Standard C++ header, unchanged. |
| `#include <iostream>` | **Kept** | Standard C++ header, unchanged. |
| `#include <cmath>` | **Kept** | Standard C++ header, unchanged. |
| `complex<float>`, `complex<double>` | **Kept** | Standard C++ complex types; no change needed. |
| `dtype_test` (= `float`) | **Kept** | User typedef, unchanged. |
| `dtype_gold` (= `double`) | **Kept** | User typedef, unchanged. |

**Summary**: The original design used only standard C++ types (`std::complex<float>`, `std::complex<double>`) for computation. All Vitis-specific headers were included but **not used** in the actual logic. No Vitis-specific types (ap_int, ap_fixed, hls::stream, etc.) appear in any function body.

---

## Table 3: Behavioral / Performance Differences

| Item | Status | Impact |
|---|---|---|
| Algorithm | **Identical** | Same Radix-2 DIT FFT, same loop structure, same stage sequence. |
| Numeric types | **Identical** | All computation uses `std::complex<float>` (32-bit float) and `std::complex<double>` (64-bit double), unchanged. |
| Rounding/overflow modes | **No change** | No ap_fixed types used, so rounding/overflow modes are not applicable. |
| Loop tripcount hints | **DROPPED** | These are analysis-only hints in Vitis; they do not affect synthesized hardware. Catapult will infer loop bounds during analysis. |
| Top-level marking | `#pragma hls_design top` added before `FFT_TOP` | Required by Catapult to identify the design hierarchy. |
| Testbench | **Unchanged** | Identical algorithm, same checks, same pass/fail criteria. |

---

## Testbench Result

**PASSED** — The translated design compiles and runs, producing output that matches the golden reference within floating-point precision. The testbench returns 0 ("Test PASSED") with a maximum error of 0.0000.