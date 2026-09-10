# Translation Report: n256_UF16 (Vitis HLS → Siemens Catapult HLS)

## Top-Level Function
`FFT_TOP` (from `synth.tcl` `set_top FFT_TOP`)

## Table 1: `#pragma HLS` Directive Translation

| File | Original `#pragma HLS` | Catapult Equivalent | Notes |
|------|----------------------|-------------------|-------|
| FFT.cpp | `#pragma HLS inline` (RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma hls_design inline` | Placed before function definition |
| FFT.cpp | `#pragma HLS inline` (bit_reverse) | `#pragma hls_design inline` | Placed before function definition |
| FFT.cpp | `#pragma HLS inline` (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma hls_design inline` | Placed before function definition |
| FFT.cpp | `#pragma HLS PIPELINE II=1` (various loops) | `#pragma hls_pipeline_init_interval 1` | Moved to line before `for` loop |
| FFT.cpp | `#pragma HLS PIPELINE II=FFT_NUM/(2*UF)` | `#pragma hls_pipeline_init_interval FFT_NUM/(2*UF)` | Moved before loop |
| FFT.cpp | `#pragma HLS pipeline` (no II) | `#pragma hls_pipeline_init_interval 1` | Moved before loop |
| FFT.cpp | `#pragma HLS UNROLL` (full) | `#pragma hls_unroll yes` | Moved before loop |
| FFT.cpp | `#pragma HLS UNROLL factor=n` | `#pragma hls_unroll n` | Moved before loop |
| FFT.cpp | `#pragma HLS dataflow disable_start_propagation` | DROPPED | Dataflow replaced with `#pragma hls_design block` on each sub-function |
| FFT.cpp | `#pragma HLS array_partition variable=... type=... dim=...` | DROPPED | No source-level equivalent in Catapult; memory mapping set in TCL |
| FFT.cpp | `#pragma HLS bind_storage variable=... type=RAM_2P impl=LUTRAM` | DROPPED | Resource binding done in TCL in Catapult |
| FFT.cpp | `#pragma HLS bind_op variable=... op=... impl=fabric` | DROPPED | Resource binding done in TCL in Catapult |
| FFT.cpp | `#pragma HLS stream type=pipo variable=... depth=3` | DROPPED | Stream configuration done in TCL |
| FFT.cpp | `#pragma HLS performance target_ti=... unit=cycle` | DROPPED | Analysis-only pragma; no equivalent |
| FFT.cpp | `#pragma inline off` | DROPPED | No Catapult equivalent |

## Table 2: Type, Header, and API Translation

| Original (Vitis) | Replacement (Catapult) |
|------------------|----------------------|
| `#include "ap_fixed.h"` | `#include <ac_int.h>` |
| `#include <hls_stream.h>` | `#include <ac_channel.h>` |
| `#include "hls_vector.h"` | `#include <array>` |
| `#include "hls_fft.h"` | Removed (not used) |
| `#include "hls_streamofblocks.h"` | Removed (not used) |
| `hls::stream<hls::vector<complex<float>, UF*2>>` | `ac_channel<vec_cfloat>` where `vec_cfloat = std::array<complex<float>, 32>` |
| `hls::vector<complex<float>, UF*2>` | `vec_cfloat` (std::array typedef) |
| `ap_uint<N>` | `ac_int<N, false>` |
| `ap_uint<EXP2_FFT>` | `ac_int<EXP2_FFT, false>` |
| `x.range(hi, lo)` | `x[bit_i]` (single bit selection) — only single-bit range used |
| `stream.read()` | `channel.read()` (same API) |
| `stream.write(v)` | `channel.write(v)` (same API) |

## Table 3: Behavioral/Performance Impact Analysis

| Change | Impact | Rationale |
|--------|--------|-----------|
| **Dropped `#pragma HLS array_partition`** | ⚠ Performance | Catapult controls partitioning via TCL (`memory map`). The source-level directives are removed; partitioning must be re-applied manually in `run.tcl` if needed. |
| **Dropped `#pragma HLS bind_storage`** | ⚠ Performance | Resource binding for RAM type is done in Catapult TCL. Default synthesis may infer different memory types. |
| **Dropped `#pragma HLS bind_op`** | ⚠ Performance | Resource binding for operators (e.g., `fadd`, `fsub`, `mul` with `impl=fabric`) is dropped. Catapult will auto-select implementation. |
| **Dropped `#pragma HLS stream type=pipo`** | ⚠ Performance | Streaming FIFO configuration must be set in Catapult TCL. Default may differ. |
| **Dropped `#pragma HLS dataflow`** | ⚠ Architecture | Replaced with `#pragma hls_design block` on each sub-function. Catapult's hierarchical block design creates similar dataflow pipelines automatically during `go architect`. However, the original Vitis dataflow region is per-function-region, while Catapult blocks are per-function-definition. |
| **Dropped `#pragma HLS performance`** | ⚪ Analysis only | No performance impact; used only for estimation. |
| **Rounding/Overflow modes** | ✅ No change | Default modes (TRN/WRAP) match between Vitis and Catapult. No explicit rounding modes were specified. |
| **Bit-widths / Signedness** | ✅ No change | `ap_int<W>` → `ac_int<W, true>`, `ap_uint<W>` → `ac_int<W, false>` preserves exact widths and signedness. |
| **Algorithm / Loop structure** | ✅ No change | Identical loops, identical computation, identical function call hierarchy. |
| **Function names / Arguments** | ✅ No change | All function names and argument order preserved; only types changed. |
| **Data type: `hls::vector` → `std::array`** | ✅ No change | Both provide contiguous storage and `operator[]`. |

## Testbench Result

**PASSED** — The translated testbench compiled and ran successfully. The maximum error between the Catapult-translated FFT and the golden model was **0.0000** (threshold: 1.0). All checks pass and the binary returns 0.

## Summary

- **Active `#pragma HLS` directives in original**: ~30+ (pipeline, unroll, inline, dataflow, array_partition, bind_storage, bind_op, stream, performance)
- **Translated to Catapult equivalents**: pipeline → `hls_pipeline_init_interval`, unroll → `hls_unroll`, inline → `hls_design inline`, dataflow → `hls_design block` on sub-functions
- **Dropped** (TCL or analysis-only): array_partition, bind_storage, bind_op, stream, performance, inline_off
- **Remaining commented Vitis pragmas**: ~10 lines (in comments, harmless documentation)
- **No active Vitis identifiers remain** in the source code.
- **Testbench**: Passes with max error 0.0.