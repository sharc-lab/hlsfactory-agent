# Translation Report: n256_UF2 Vitis HLS → Siemens Catapult HLS

## Source Design
- **Design name**: n256_UF2
- **Top function**: `FFT_TOP`
- **Original tool**: Vitis HLS
- **Target tool**: Siemens Catapult HLS

---

## Table 1: Pragma Translations

| File | Original Pragma | Translated To | Notes |
|------|----------------|---------------|-------|
| FFT.cpp | `#pragma HLS inline` (in RADIX2_BFLY_double_buffer_quarter_CY) | `#pragma hls_design inline` | Placed before function definition |
| FFT.cpp | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | **DROPPED** | No equivalent in Catapult source; resource binding is done via TCL |
| FFT.cpp | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | **DROPPED** | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | **DROPPED** | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | **DROPPED** | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | **DROPPED** | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | **DROPPED** | Same as above |
| FFT.cpp | `#pragma HLS bind_op variable=index op=mul impl=fabric` (3 occurrences) | **DROPPED** | Same as above |
| FFT.cpp | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` (multiple occurrences) | **DROPPED** | Analysis-only directive; no Catapult equivalent in source |
| FFT.cpp | `#pragma HLS pipeline` (or `#pragma HLS pipeline II=1`) (inside loop bodies) | `#pragma hls_pipeline_init_interval 1` | Moved to line BEFORE the loop statement |
| FFT.cpp | `#pragma HLS UNROLL` (in bit_reverse) | `#pragma hls_unroll yes` | Moved before the loop |
| FFT.cpp | `#pragma HLS UNROLL factor=UF>>(stage-1)` | `#pragma hls_unroll UF>>(stage-1)` | Moved before the loop |
| FFT.cpp | `#pragma HLS UNROLL factor=UF` | `#pragma hls_unroll UF` | Moved before the loop |
| FFT.cpp | `#pragma HLS DATAFLOW disable_start_propagation` (in reverse_input_stream_UF2) | **DROPPED** | Catapult cannot express intra-function dataflow regions; loops are sequential |
| FFT.cpp | `#pragma HLS DATAFLOW disable_start_propagation` (in FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma hls_design block` on each sub-function called within | Sub-functions marked as `hls_design block` |
| FFT.cpp | `#pragma HLS DATAFLOW disable_start_propagation` (in FFT_TOP) | **DROPPED** | Initialization loops kept sequential; FFT_DIT... marked `hls_design block` |
| FFT.cpp | `#pragma HLS array_partition variable=... type=complete dim=1` (multiple occurrences) | **DROPPED** | No source-level equivalent in Catapult; memory mapping is TCL |
| FFT.cpp | `#pragma HLS array_partition variable=... type=cyclic factor=UF*2 dim=1` (multiple) | **DROPPED** | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=... type=cyclic factor=UF dim=1` (multiple) | **DROPPED** | Same as above |
| FFT.cpp | `#pragma HLS array_partition variable=twiddles complete` | **DROPPED** | Same as above |
| FFT.cpp | `#pragma HLS bind_storage variable=data_0 type=RAM_2P impl=LUTRAM` | **DROPPED** | Resource binding is TCL in Catapult |
| FFT.cpp | `#pragma HLS stream type=pipo variable=data_in_cyclic` | **DROPPED** | Stream type directive; no source equivalent |
| FFT.cpp | `#pragma HLS stream type=pipo variable=data_rev_stream` | **DROPPED** | Same as above |
| FFT.cpp | `#pragma HLS dependence variable=... inter direction=WAW false` (multiple) | **DROPPED** | Dependence analysis directive; no source equivalent |
| FFT.cpp | `#pragma inline off` (Vitis pragma, not `#pragma HLS`) | **DROPPED** | Default inlining behavior in Catapult |

---

## Table 2: Type and Header Translations

| Original | Replacement | Notes |
|----------|-------------|-------|
| `ap_fixed.h` | *(removed)* | Header was included but no ap_fixed types were used in the design |
| `hls_fft.h` | *(removed)* | Header was included but no Vitis FFT functions were used |
| `hls_stream.h` | `ac_channel.h` | Direct replacement |
| `hls_vector.h` | *(replaced with custom `ac_vector` struct)* | No direct Catapult equivalent; a simple template struct `ac_vector<T,N>` with `operator[]` was defined |
| `hls_streamofblocks.h` | *(removed)* | Not used in the design |
| `hls::stream<T>` | `ac_channel<T>` | Stream → channel |
| `hls::vector<T, N>` | `ac_vector<T, N>` | Custom replacement struct wrapping `T data[N]` |
| `ap_uint<N>` | `ac_int<N, false>` | Unsigned integer |
| `stream.read()` | `channel.read()` | Same API |
| `stream.write(v)` | `channel.write(v)` | Same API |
| `x.range(bit_i, bit_i)` | `x[bit_i]` | Single-bit access |

---

## Table 3: Behavioral and Performance Differences

| Item | Impact | Explanation |
|------|--------|-------------|
| Intra-function DATAFLOW in `reverse_input_stream_UF2` | **Performance difference** | The three loops (READ_STREAM_INPUT, FROM_BLOCK_TO_CYCLIC, STREAM_OUT_REVERSE) were connected via Vitis DATAFLOW to run concurrently as a pipeline. In Catapult, these run sequentially since there is no intra-function dataflow construct. The algorithm is functionally identical. |
| DATAFLOW in `FFT_TOP` | **Performance difference** | The twiddle initialization loop, revidtab loop, and FFT_DIT... call were in a Vitis DATAFLOW region. In Catapult they run sequentially. The FFT_DIT... sub-function is marked `hls_design block`. |
| All `ARRAY_PARTITION` directives dropped | **Performance difference** | Vitis ARRAY_PARTITION completely partitions arrays into registers for parallel access. Catapult sets memory mapping via TCL (not source). Without equivalent TCL directives, the synthesized design may have different memory access characteristics. |
| All `DEPENDENCE` directives dropped | **Possible performance difference** | Vitis DEPENDENCE can relax false dependencies; dropping them may cause the scheduler to be more conservative. |
| All `BIND_OP` and `BIND_STORAGE` directives dropped | **Possible implementation difference** | Catapult resource binding is done via TCL directives; the default allocation may differ. |
| `#pragma inline off` dropped | **Possible implementation difference** | Functions may be inlined by Catapult's optimizer differently. |
| All `performance target_ti` dropped | **No functional impact** | Analysis-only guidance. |
| `stream type=pipo` dropped | **No functional impact** | FIFO implementation hint. |
| `hls_design top` on `FFT_TOP` | **Marking** | Required Catapult top-level design marker. |
| `hls_design block` on dataflow sub-functions | **Structural** | Sub-functions called in dataflow regions marked as hierarchical blocks. |

---

## Verification

**Testbench result: PASSED**
- Maximum error: 0.0000
- The translated design produces numerically identical results to the original (within floating-point precision).
- All testbench checks remain intact.