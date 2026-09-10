# Translation report: vitis_mac -> Catapult (hand-translated reference)

This folder is the expected result of translating `designs/vitis_mac`. It exists to validate the
check path without an LLM and to give the agent prompt a worked example if needed.

## Pragmas

| File | Original | Result |
|---|---|---|
| mac.cpp | `#pragma HLS INTERFACE axis port=in` | DROPPED: interfaces are TCL directives in Catapult |
| mac.cpp | `#pragma HLS INTERFACE axis port=out` | DROPPED: interfaces are TCL directives in Catapult |
| mac.cpp | `#pragma HLS ARRAY_PARTITION variable=coef complete dim=1` | DROPPED: no source-level equivalent; set in TCL if needed |
| mac.cpp | `#pragma HLS PIPELINE II=1` (inside loop) | `#pragma hls_pipeline_init_interval 1` on the line before the loop |
| mac.cpp | (none) | `#pragma hls_design top` added before the top function |

## Types and headers

| Original | Replacement |
|---|---|
| `ap_fixed.h` | `ac_fixed.h` |
| `hls_stream.h` | `ac_channel.h` |
| `ap_fixed<16, 8>` | `ac_fixed<16, 8, true>` |
| `ap_fixed<32, 16>` | `ac_fixed<32, 16, true>` |
| `hls::stream<T>` | `ac_channel<T>` |

## Behavioral notes

- Rounding and overflow modes were not named in the original, so both sides use their defaults, truncate and wrap.
- The dropped array partition may change resource usage and scheduling in Catapult; it does not change results.

Testbench: passes (PASS printed, exit 0).
