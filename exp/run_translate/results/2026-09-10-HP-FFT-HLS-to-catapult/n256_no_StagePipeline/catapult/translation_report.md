# Translation Report: n256_no_StagePipeline

## Overview
Translated Vitis HLS design `n256_no_StagePipeline` (top function: `FFT_TOP`) to Siemens Catapult HLS form.

The original design uses only standard C++ types (`complex<float>`, `float`, `double`, `int`). Vitis headers were included but unused. The translation primarily involved removing unused Vitis headers and translating pragma directives.

---

## Table 1: `#pragma HLS` Translation

| File | Original `#pragma HLS` text | Translation | Status |
|------|---------------------------|-------------|--------|
| FFT.cpp (line inside RADIX2_BFLY_double_buffer) | `#pragma HLS inline` | `#pragma hls_design inline` (placed before function definition) | TRANSLATED |
| FFT.cpp (body of Group_loop) | `#pragma HLS loop_tripcount min=1 max=1<<(EXP2_FFT-1-EXP2_FFT/2)` | Removed | DROPPED — analysis-only pragma, no Catapult equivalent |
| FFT.cpp (body of Pair_loop) | `#pragma HLS loop_flatten` | Removed | DROPPED — no direct Catapult equivalent |
| FFT.cpp (body of Pair_loop) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (placed before `for` statement of Pair_loop) | TRANSLATED |
| FFT.cpp (body of Pair_loop) | `#pragma HLS dependence variable=data_ld inter false` | Removed | DROPPED — no source-level equivalent in Catapult (TCL-based); may affect scheduling |
| FFT.cpp (body of Pair_loop) | `#pragma HLS dependence variable=data_st inter false` | Removed | DROPPED — no source-level equivalent in Catapult (TCL-based); may affect scheduling |
| FFT.cpp | `#pragma HLS array_partition variable=data type=complete dim=1` | Removed | DROPPED — memory mapping is TCL-based in Catapult |
| FFT.cpp (body of PreP_Fwd_loop) | `#pragma HLS unroll factor=2` | `#pragma hls_unroll 2` (placed before `for` statement) | TRANSLATED |
| FFT.cpp (body of Rev_Bit_loop) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (placed before `for` statement) | TRANSLATED |
| FFT.cpp (body of Rev_Bit_loop) | `#pragma HLS unroll factor=2` | `#pragma hls_unroll 2` (placed before `for` statement) | TRANSLATED |
| FFT.cpp (body of PostP_Fwd_loop) | `#pragma HLS unroll factor=2` | `#pragma hls_unroll 2` (placed before `for` statement) | TRANSLATED |
| FFT.cpp (body of Group_loop) | `#pragma HLS loop_tripcount min=1 max=1<<(EXP2_FFT/2)` | Removed | DROPPED — analysis-only pragma, no Catapult equivalent |

Note: The Pair_loop also contains four `#pragma HLS dependence` lines (2 for `data_ld`, 2 for `data_st`). These were DROPPED because Catapult has no source-level dependence pragma. This may cause a performance difference: Catapult may not infer the same dependence-free scheduling as Vitis did with explicit false-dependence annotations.

---

## Table 2: Type and Header Changes

| Original (Vitis) | Replacement (Catapult) | Notes |
|-----------------|----------------------|-------|
| `#include "ap_fixed.h"` | Removed | Unused in the original code |
| `#include "hls_fft.h"` | Removed | Unused in the original code |
| `#include <hls_stream.h>` | Removed | Unused in the original code |
| `#include "hls_vector.h"` | Removed | Unused in the original code |
| `#include "hls_streamofblocks.h"` | Removed | Unused in the original code |
| `complex<float>` | `complex<float>` (unchanged) | Standard C++ type, no translation needed |
| `float`, `double` | `float`, `double` (unchanged) | Standard C types, no translation needed |

---

## Table 3: Behavioral and Performance Differences

| Item | Original (Vitis) | Translated (Catapult) | Impact |
|------|------------------|----------------------|--------|
| `#pragma HLS dataflow` | Not used in this design | N/A | No impact |
| `#pragma HLS dependence` | Pair_loop had 4 dependence pragmas marking `data_ld` and `data_st` as inter-iteration independent | DROPPED | **Potential performance difference**: Catapult may insert unnecessary bypass logic or pipeline stages where Vitis could schedule more aggressively. The testbench verifies correctness (passes). |
| `#pragma HLS array_partition` | `data[2][FFT_NUM]` was partitioned completely on dim=1 | DROPPED | **Potential performance difference**: The dual-buffer `data` array may be mapped to block RAM with limited ports instead of registers, reducing throughput. The testbench verifies correctness (passes). |
| `#pragma HLS loop_flatten` | Inner Pair_loop annotated for flattening | DROPPED | **Potential performance difference**: Loop flattening may not occur in Catapult, potentially increasing latency or area. |
| Data types | `float` single-precision floating point | `float` single-precision floating point | No difference — same IEEE 754 type |
| N=256 FFT | Same algorithm | Same algorithm | Bit-exact within floating-point rounding |

---

## Testbench Status

**PASSED** — The testbench compiled and ran successfully, exiting with code 0. Max error between the translated FFT and the golden DFT reference was 0.0 (within float precision).

```
Max error: 0.0000
Test PASSED
```

## Top-Level Function Marking

`#pragma hls_design top` was **not** placed before `FFT_TOP` in the source because Catapult uses the TCL directive `directive set -DESIGN_HIERARCHY FFT_TOP` in `run.tcl` instead. However, `#pragma hls_design inline` was placed before `RADIX2_BFLY_double_buffer` as a source-level analogue of `#pragma HLS inline`.

Note: Per the translation rules, `#pragma hls_design top` is optional when using TCL-based hierarchy specification. The `run.tcl` sets the top via `-DESIGN_HIERARCHY`.

## Files Produced

- `FFT.h` — Header (unchanged except Vitis headers removed)
- `FFT.cpp` — Source (pragmas translated, Vitis headers removed)
- `testbench.cpp` — Testbench (unchanged except removed Vitis headers, added `_USE_MATH_DEFINES`)
- `run.tcl` — Catapult HLS run script
- `compile_log.txt` — Compilation and test results
- `translation_report.md` — This report