# Translation Report: Vitis HLS → Siemens Catapult HLS

**Design:** n1024_UF2 (FFT N=1024, UF=2)
**Top function:** `FFT_TOP`
**Original files:** FFT.h, FFT.cpp, testbench.cpp

---

## Table 1: #pragma HLS translations

| File | Original Vitis pragma | Catapult replacement | Notes |
|------|----------------------|---------------------|-------|
| FFT.cpp:4 | `#pragma HLS inline` | `#pragma hls_design inline` | Placement: before function definition |
| FFT.cpp:5 | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | **DROPPED** | No source-level equivalent; resource binding is TCL in Catapult |
| FFT.cpp:6 | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | **DROPPED** | Same reason |
| FFT.cpp:7 | `#pragma HLS bind_op variable=d2_real op=fadd impl=fabric` | **DROPPED** | Same reason |
| FFT.cpp:8 | `#pragma HLS bind_op variable=d2_imag op=fadd impl=fabric` | **DROPPED** | Same reason |
| FFT.cpp:9 | `#pragma HLS bind_op variable=d3_real op=fsub impl=fabric` | **DROPPED** | Same reason |
| FFT.cpp:10 | `#pragma HLS bind_op variable=d3_imag op=fsub impl=fabric` | **DROPPED** | Same reason |
| FFT.cpp (output_result_array_to_stream) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only pragma; no Catapult equivalent |
| FFT.cpp (output_result_array_to_stream) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` | Placement: before `for` statement |
| FFT.cpp (bit_reverse) | `#pragma HLS inline` | `#pragma hls_design inline` | Before function |
| FFT.cpp (bit_reverse) | `#pragma HLS UNROLL` | `#pragma hls_unroll yes` | Before `for` statement |
| FFT.cpp (reverse_read_stream_input) | `#pragma inline off` | **DROPPED** | Vitis-specific "disable inlining"; no Catapult equivalent |
| FFT.cpp (reverse_read_stream_input) | `#pragma HLS pipeline II=1` | `#pragma hls_pipeline_init_interval 1` | Before `for` |
| FFT.cpp (reverse_read_stream_input) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| FFT.cpp (reverse_read_stream_input) | `#pragma HLS array_partition variable=original type=complete dim=1` | **DROPPED** | No source-level equivalent; memory mapping via TCL in Catapult |
| FFT.cpp (reverse_read_stream_input) | `#pragma HLS array_partition variable=reversed type=complete dim=1` | **DROPPED** | Same reason |
| FFT.cpp (reverse_read_stream_input) | `#pragma HLS dependence variable=data_rev_stream_0 inter direction=WAW false` | **DROPPED** | No equivalent; possible performance difference |
| FFT.cpp (reverse_read_stream_input) | `#pragma HLS dependence variable=data_rev_stream_1 inter direction=WAW false` | **DROPPED** | Same reason |
| FFT.cpp (reverse_read_stream_input) | `#pragma HLS dependence variable=data_rev_stream_2 inter direction=WAW false` | **DROPPED** | Same reason |
| FFT.cpp (reverse_read_stream_input) | `#pragma HLS dependence variable=data_rev_stream_3 inter direction=WAW false` | **DROPPED** | Same reason |
| FFT.cpp (reverse_from_block_to_cyclic) | `#pragma inline off` | **DROPPED** | Vitis-specific |
| FFT.cpp (reverse_from_block_to_cyclic) | `#pragma HLS pipeline II=1` | `#pragma hls_pipeline_init_interval 1` | Before `for` |
| FFT.cpp (reverse_from_block_to_cyclic) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| FFT.cpp (reverse_from_block_to_cyclic) | 5x `#pragma HLS array_partition variable=... type=complete dim=1` | **DROPPED** | No source-level equivalent |
| FFT.cpp (reverse_from_block_to_cyclic) | `#pragma HLS dependence variable=data_in_cyclic inter direction=WAW false` | **DROPPED** | No equivalent; possible perf impact |
| FFT.cpp (reverse_write_stream_output) | `#pragma inline off` | **DROPPED** | Vitis-specific |
| FFT.cpp (reverse_write_stream_output) | `#pragma HLS pipeline II=1` | `#pragma hls_pipeline_init_interval 1` | Before `for` |
| FFT.cpp (reverse_write_stream_output) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| FFT.cpp (reverse_input_stream_UF2) | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=1` | **DROPPED** | No source-level equivalent |
| FFT.cpp (reverse_input_stream_UF2) | `#pragma HLS array_partition variable=data_in_cyclic type=cyclic factor=UF dim=2` | **DROPPED** | Same reason |
| FFT.cpp (reverse_input_stream_UF2) | `#pragma HLS stream type=pipo variable=data_in_cyclic` | **DROPPED** | No equivalent in Catapult source |
| FFT.cpp (reverse_input_stream_UF2) | `#pragma HLS stream type=pipo variable=data_rev_stream` | **DROPPED** | Same reason |
| FFT.cpp (reverse_input_stream_UF2) | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design block` on `FFT_DIT_spatial_unroll_CY_stream_vector` | Catapult hierarchical blocks are per-function, not per-region. Lossy: inner dataflow in reverse_input_stream_UF2 is flattened into the function block. |
| FFT.cpp (reverse_input_stream_UF2) | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | **DROPPED** | Analysis-only |
| FFT.cpp (reverse_input_stream_UF2) | 3x `#pragma HLS pipeline II=1` (in 3 inner loops) | `#pragma hls_pipeline_init_interval 1` | Before each `for` |
| FFT.cpp (FFT_stage_spatial_unroll) | `#pragma HLS performance target_ti=...` (3x) | **DROPPED** | Analysis-only |
| FFT.cpp (FFT_stage_spatial_unroll) | `#pragma HLS unroll factor=UF>>(stage-1)` | `#pragma hls_unroll yes` | Unroll factor not directly expressible; full unroll used |
| FFT.cpp (FFT_stage_spatial_unroll) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` | Before `for` |
| FFT.cpp (FFT_stage_spatial_unroll) | `#pragma HLS bind_op variable=index op=mul impl=fabric` | **DROPPED** | TCL in Catapult |
| FFT.cpp (FFT_stage_spatial_unroll) | `#pragma HLS unroll factor=UF` | `#pragma hls_unroll yes` | Factor dropped; full unroll |
| FFT.cpp (FFT_stage_spatial_unroll) | `#pragma HLS array_partition variable=twiddles complete` | **DROPPED** | TCL in Catapult |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_onlycompute) | `#pragma HLS inline` | `#pragma hls_design inline` | Before function |
| FFT.cpp (RADIX2_BFLY_double_buffer_quarter_onlycompute) | 4x `#pragma HLS bind_op ...` | **DROPPED** | TCL in Catapult |
| FFT.cpp (FFT_Stage1_vectorstream_parameterize) | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design block` | Before function |
| FFT.cpp (FFT_Stage1_vectorstream_parameterize) | `#pragma HLS performance ...` | **DROPPED** | Analysis-only |
| FFT.cpp (FFT_Stage1_vectorstream_parameterize) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` | Before inner `for` |
| FFT.cpp (FFT_Stage2_vectorstreamIn_arrayOut_parametize) | `#pragma HLS performance ...` (2x) | **DROPPED** | Analysis-only |
| FFT.cpp (FFT_Stage2_vectorstreamIn_arrayOut_parametize) | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` | Before `for` |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design block` | Catapult hierarchical block on this function |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS performance ...` | **DROPPED** | Analysis-only |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | 9x `#pragma HLS array_partition variable=data_0..data_8 type=cyclic factor=UF*2 dim=1` or `factor=UF dim=1` | **DROPPED** | No source-level equivalent |
| FFT.cpp (FFT_DIT_spatial_unroll_CY_stream_vector) | `#pragma HLS bind_storage variable=data_0 type=RAM_2P impl=LUTRAM` | **DROPPED** | TCL in Catapult |
| FFT.cpp (FFT_TOP) | `#pragma HLS dataflow disable_start_propagation` | **DROPPED** | Top-level dataflow; sequential execution in Catapult (the sub-functions are called sequentially anyway) |
| FFT.cpp (FFT_TOP) | Top-level `#pragma hls_design top` | **ADDED** | Marks the top function |

---

## Table 2: Type and header translations

| Original | Replacement | Notes |
|----------|-------------|-------|
| `hls_stream.h` | `ac_channel.h` | Standard AC type header |
| `hls_vector.h` | Custom `vec<T,N>` struct | No direct AC equivalent for hls::vector; defined in FFT.h |
| `hls_fft.h` | **Removed** | Not used in the design |
| `hls_streamofblocks.h` | **Removed** | Not used in the design |
| `ap_fixed.h` | **Removed** | Not used in the design |
| `hls::stream<T>` | `ac_channel<T>` | Standard AC channel |
| `hls::vector<T,N>` | `vec<T,N>` | Custom aggregate type (defined in FFT.h) |
| `ap_uint<N>` | `ac_int<N, false>` | Unsigned integer |
| `x.range(hi, lo)` (single bit assignment) | `x[bit]` or `x[bit]` | Bit-select via `operator[]` on ac_int |
| `stream.read()` | `channel.read()` | Same semantics |
| `stream.write(v)` | `channel.write(v)` | Same semantics |

---

## Table 3: Potential behavioral or performance differences

| Difference | Impact | Explanation |
|------------|--------|-------------|
| Dropped `#pragma HLS DEPENDENCE` | **Performance** | Vitis `DEPENDENCE` pragmas on `data_rev_stream_0/1/2/3` and `data_in_cyclic` told the scheduler to ignore false WAW dependencies. Without them, Catapult may schedule more conservatively, potentially increasing latency. The testbench is functional (correctness unaffected). |
| Dropped `#pragma HLS ARRAY_PARTITION` | **Performance** | Vitis `ARRAY_PARTITION` (complete, cyclic) partitioned arrays for parallel access. Catapult requires TCL-based memory partitioning in `go libraries`/`directive set -ARRAY_MAP`. Without TCL partitioning, memory may become a bottleneck. |
| Dropped `#pragma HLS BIND_STORAGE` | **Performance** | Vitis specified `RAM_2P` + `LUTRAM` for `data_0`. Catapult infers memory type automatically from TCL. |
| Dropped `#pragma HLS BIND_OP` | **Performance** | Vitis bound operations to `fabric` vs `dsp`. Catapult binding is via `directive set -CLOCKS` and operator libraries. |
| Dropped `#pragma HLS STREAM type=pipo` | **Performance** | Vitis specified pipelined streaming behavior for arrays `data_in_cyclic` and `data_rev_stream`. Catapult requires TCL equivalent. |
| Dropped `#pragma HLS DATAFLOW disable_start_propagation` on `reverse_input_stream_UF2` | **Structure** | Vitis dataflow pipelined the three loops inside this function. Catapult's `#pragma hls_design block` is per-function, so the three sequential loops inside `reverse_input_stream_UF2` will not be dataflow-pipelined. The function as a whole is a block. |
| Dropped `#pragma HLS DATAFLOW disable_start_propagation` on `FFT_TOP` | **Structure** | The top-level dataflow calling `FFT_DIT_spatial_unroll_CY_stream_vector` plus the twiddle/reverse table initialization loops is flattened into sequential execution. In Catapult, the sub-function `FFT_DIT_spatial_unroll_CY_stream_vector` is marked `hls_design block`, preserving its internal structure. |
| `#pragma HLS UNROLL factor=n` → `#pragma hls_unroll yes` (full unroll) | **Area** | Vitis unroll factor controls partial unrolling; Catapult's `hls_unroll yes` performs full unroll of the loop body. This may increase area usage. |
| `#pragma HLS PIPELINE` (unspecified II) → `#pragma hls_pipeline_init_interval 1` | **Performance** | Vitis default II is 1 when unspecified. Catapult default is 1 as well, so behavior is equivalent. |

---

## Testbench Result

**Status: PASSED**

The testbench compiled and ran successfully with exit code 0. Maximum error between the hardware design and the golden software model was 0.0000 (well below the 1.0 threshold).

All Vitis identifiers have been removed from source files. No `ap_int`, `ap_uint`, `ap_fixed`, `hls::stream`, or `#pragma HLS` remain in `.cpp` or `.h` files.