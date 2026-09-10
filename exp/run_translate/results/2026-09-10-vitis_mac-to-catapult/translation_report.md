# Translation Report: Vitis HLS to Siemens Catapult HLS

Design name: `vitis_mac`
Top-level function: `mac`

## Table 1: #pragma HLS Translations

| File | Original Pragma | Replacement | Notes |
|------|----------------|-------------|-------|
| mac.cpp | `#pragma HLS INTERFACE axis port=in` | **DROPPED** | No source-level equivalent in Catapult; interfaces are specified in TCL directives |
| mac.cpp | `#pragma HLS INTERFACE axis port=out` | **DROPPED** | No source-level equivalent in Catapult; interfaces are specified in TCL directives |
| mac.cpp | `#pragma HLS ARRAY_PARTITION variable=coef complete dim=1` | **DROPPED** | No source-level equivalent in Catapult; memory mapping is set in TCL |
| mac.cpp | `#pragma HLS PIPELINE II=1` | `#pragma hls_pipeline_init_interval 1` (moved before the `for` loop) | Placement changed from inside loop body to line before loop |

Additional directives added:
- `#pragma hls_design top` placed before the `mac()` function definition to mark it as the top-level design block.

## Table 2: Type and Header Changes

| Original | Replacement | Notes |
|----------|-------------|-------|
| `#include "ap_fixed.h"` | `#include "ac_fixed.h"` | Vitis fixed-point header -> Catapult AC fixed-point header |
| `#include "hls_stream.h"` | `#include "ac_channel.h"` | Vitis stream header -> Catapult AC channel header |
| `ap_fixed<16, 8>` (data_t) | `ac_fixed<16, 8, true>` | Signed fixed-point: same W and I; explicit signedness parameter `true` |
| `ap_fixed<32, 16>` (acc_t) | `ac_fixed<32, 16, true>` | Signed fixed-point: same W and I; explicit signedness parameter `true` |
| `hls::stream<data_t>` | `ac_channel<data_t>` | Vitis stream -> Catapult channel; same `read()` and `write()` API |
| `hls::stream<acc_t>` | `ac_channel<acc_t>` | Vitis stream -> Catapult channel |

No explicit rounding or overflow modes were specified in the original code, so neither Vitis defaults nor Catapult defaults needed cross-reference. Both tools default to `AP_TRN`/`AC_TRN` (truncation) and `AP_WRAP`/`AC_WRAP` (wrap-around).

## Table 3: Behavioral / Performance Implications

| Item | Status | Impact |
|------|--------|--------|
| **ARRAY_PARTITION (coef complete dim=1)** | DROPPED | In Vitis this ensures the coefficient array is fully partitioned into registers, enabling full parallel access. In Catapult, memory mapping is controlled via TCL; the absence of this directive may lead to a BRAM-based implementation instead of registers, potentially impacting throughput. The user should set array mapping in `run.tcl` via `directive set -REGISTER_NAME` or similar if register-level access is required. |
| **INTERFACE axis** | DROPPED | In Vitis this creates AXI4-Stream interfaces. In Catapult, I/O ports are inferred from function arguments; the default wire-interface protocol may differ. The user should add TCL directives for specific interface protocols if needed. |
| **PIPELINE II=1** | PRESERVED | Translated to `hls_pipeline_init_interval 1`. Catapult may schedule differently than Vitis, but the same II constraint is applied. |
| **Rounding / Overflow modes** | No explicit modes specified | Both default to truncation (AP_TRN / AC_TRN) and wrap-around (AP_WRAP / AC_WRAP), so behavior is identical. |
| **Signedness** | Preserved exactly | `ap_fixed` signed = `ac_fixed` with `true` signedness parameter. |
| **Bit widths** | Preserved exactly | W=16,I=8 for data_t; W=32,I=16 for acc_t. |

## Testbench Result

The testbench was compiled and run successfully:

```
expected=11.679688 got=11.679688 err=0.000000
PASS
```

**Conclusion: The testbench PASSED.** The output matches the expected value within the tolerance of 1e-3, confirming that the translation preserved the algorithmic behavior exactly.