# Translation Report: n256_UF4 (Vitis HLS → Siemens Catapult HLS)

## Top Function
`FFT_TOP` (from `set_top` line in `synth.tcl`)

## Table 1: #pragma HLS Translation

| File | Original Vitis Pragma | Catapult Equivalent | Notes |
|------|----------------------|---------------------|-------|
| FFT.cpp:15 | `#pragma HLS inline` | `#pragma hls_design inline` (before function def) | Moved from inside function body to line before definition |
| FFT.cpp:33 | `#pragma HLS bind_op variable=d1_real op=fsub impl=fabric` | DROPPED | No Catapult source equivalent; resource binding is TCL-only |
| FFT.cpp:34 | `#pragma HLS bind_op variable=d1_imag op=fadd impl=fabric` | DROPPED | Same |
| FFT.cpp:45-48 | Four `#pragma HLS bind_op` (d2_real=fadd, d2_imag=fadd, d3_real=fsub, d3_imag=fsub) | DROPPED | Same |
| FFT.cpp:59 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only directive; no Catapult source equivalent |
| FFT.cpp:62 | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Moved from inside loop body to line before `for` |
| FFT.cpp:71 | `#pragma HLS inline` | `#pragma hls_design inline` | Moved before function definition |
| FFT.cpp:74 | `#pragma HLS UNROLL` | `#pragma hls_unroll yes` (before loop) | Moved from inside loop body to line before `for` |
| FFT.cpp:87 | `#pragma HLS array_partition variable=data_rev_stream type=complete dim=1` | DROPPED | No source-level equivalent in Catapult; memory mapping via TCL |
| FFT.cpp:88 | `#pragma HLS array_partition variable=data_in_cyclic type=complete dim=1` | DROPPED | Same |
| FFT.cpp:89 | `#pragma HLS array_partition variable=data_in_cyclic type=cyclic factor=UF dim=2` | DROPPED | Same |
| FFT.cpp:92 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:96 | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Moved |
| FFT.cpp:97 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:101 | `#pragma HLS array_partition variable=original type=complete dim=1` | DROPPED | No source equivalent |
| FFT.cpp:105 | `#pragma HLS array_partition variable=reversed type=complete dim=1` | DROPPED | Same |
| FFT.cpp:121 | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Moved |
| FFT.cpp:122 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:125 | `#pragma HLS array_partition variable=offset type=complete dim=1` | DROPPED | No source equivalent |
| FFT.cpp:128 | `#pragma HLS array_partition variable=cyclic_offset type=complete dim=1` | DROPPED | Same |
| FFT.cpp:130 | `#pragma HLS array_partition variable=block_data type=complete dim=1` | DROPPED | Same |
| FFT.cpp:133 | `#pragma HLS array_partition variable=cyclic_data type=complete dim=1` | DROPPED | Same |
| FFT.cpp:286 | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Moved |
| FFT.cpp:287 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:304 | `#pragma HLS unroll factor=UF>>(stage-1)` | `#pragma hls_unroll yes factor=UF>>(stage-1)` | Moved before loop |
| FFT.cpp:305 | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before outer loop) | Moved |
| FFT.cpp:305 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:310 | `#pragma HLS bind_op variable=index op=mul impl=fabric` | DROPPED | Resource binding is TCL |
| FFT.cpp:321 | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Moved |
| FFT.cpp:322 | `#pragma HLS unroll factor=UF` | `#pragma hls_unroll yes factor=UF` (before loop) | Moved |
| FFT.cpp:323 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:326 | `#pragma HLS bind_op variable=index op=mul impl=fabric` | DROPPED | Resource binding |
| FFT.cpp:337 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:341 | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before inner loop) | Moved |
| FFT.cpp:342 | `#pragma HLS unroll factor=UF` | `#pragma hls_unroll yes factor=UF` (before loop) | Moved |
| FFT.cpp:346 | `#pragma HLS bind_op variable=index op=mul impl=fabric` | DROPPED | Resource binding |
| FFT.cpp:347 | `#pragma HLS array_partition variable=twiddles complete` | DROPPED | No source equivalent |
| FFT.cpp:356 | `#pragma HLS inline` | `#pragma hls_design inline` | Moved before function definition |
| FFT.cpp:370-373 | Four `#pragma HLS bind_op` | DROPPED | Resource binding |
| FFT.cpp:387 | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design block` (before function def) | DATAFLOW region → hierarchical block on function |
| FFT.cpp:388 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:391 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:392 | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Moved |
| FFT.cpp:413 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:416 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:417 | `#pragma HLS pipeline` | `#pragma hls_pipeline_init_interval 1` (before loop) | Moved |
| FFT.cpp:443 | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design block` (before function def) | DATAFLOW → hierarchical block |
| FFT.cpp:444 | `#pragma HLS performance target_ti=FFT_NUM/(2*UF) unit=cycle` | DROPPED | Analysis-only |
| FFT.cpp:447-453 | Seven `#pragma HLS array_partition variable=data_X type=cyclic factor=... dim=1` | DROPPED | No source equivalent |
| FFT.cpp:454 | `#pragma HLS bind_storage variable=data_0 type=RAM_2P impl=LUTRAM` | DROPPED | No source equivalent |
| FFT.cpp:456 | `#pragma HLS bind_storage variable=data_1 type=RAM_2P impl=LUTRAM` | DROPPED | Same |
| FFT.cpp:470 | `#pragma HLS dataflow disable_start_propagation` | `#pragma hls_design top` (on FFT_TOP) + `#pragma hls_design block` on sub-functions | DATAFLOW at top level → top directive + block hierarchy |
| FFT.cpp:471 | Loop twiddle generation (no pragmas removed) | Unchanged | - |
| FFT.cpp:478 | Loop revIdxTab generation (no pragmas removed) | Unchanged | - |

## Table 2: Type and Header Changes

| Original Vitis | Replacement | Notes |
|---------------|-------------|-------|
| `#include <ap_fixed.h>` | Removed | Not used in the code; no ap_fixed types were declared |
| `#include <hls_fft.h>` | Removed | Not used in the code |
| `#include <hls_stream.h>` | `#include <ac_channel.h>` | Core streaming type |
| `#include <hls_vector.h>` | `vec_wrapper<T,N>` (local struct) | Custom replacement for Vitis vector type |
| `#include <hls_streamofblocks.h>` | Removed | Not used in the code |
| `hls::stream<T>` | `ac_channel<T>` | Stream class replacement |
| `hls::vector<T, N>` | `vec_wrapper<T, N>` | Vector container replacement |
| `ap_uint<N>` | `ac_int<N, false>` | Unsigned integer type |
| `stream.read()` | `channel.read()` | Same method name |
| `stream.write(v)` | `channel.write(v)` | Same method name |
| `x.range(hi, lo)` | `x[bit_i]` (single bit access) | Single-bit range → array-style bit access |
| `x.range(N-1-bit, N-1-bit)` | `x[N-1-bit_i]` | Single-bit range → array-style bit access |

## Table 3: Behavioral / Performance Impact Assessment

| Change | Impact | Explanation |
|--------|--------|-------------|
| Dropped `#pragma HLS array_partition` | **Performance difference likely** | All array partitions (complete, cyclic, factor) were removed. In Catapult, array partitioning must be set via TCL directives. Without it, arrays will be implemented as single-block RAMs instead of multiple banks/registers, reducing parallel access and throughput. |
| Dropped `#pragma HLS bind_storage` | **Possible performance/resource difference** | Storage binding (RAM_2P, LUTRAM) was removed. Catapult uses its own memory allocation heuristics. |
| Dropped `#pragma HLS bind_op` | **Possible resource difference** | Operation binding (fabric vs. dsp) was removed. Catapult will use its own operator implementation selection. |
| Dropped `#pragma HLS performance` | **No functional impact** | These are analysis-only directives for reporting performance estimates. |
| DATAFLOW → `#pragma hls_design block` | **Architectural difference** | Vitis DATAFLOW creates dataflow regions within a function; Catapult hierarchical blocks apply to entire function definitions. This is lossy: sub-functions that were part of a dataflow region within a larger function become separate blocks. |
| Dropped `#pragma HLS DEPENDENCE` | **None (not present in this design)** | Not used in original code. |
| `#pragma HLS pipeline` → `#pragma hls_pipeline_init_interval 1` | **Equivalent** | All Vitis pipeline directives had default II=1, matching Catapult's explicit II=1. |
| `#pragma HLS UNROLL` → `#pragma hls_unroll yes` | **Equivalent** | Full unroll. |
| `#pragma HLS UNROLL factor=N` → `#pragma hls_unroll yes factor=N` | **Equivalent** | Partial unroll with same factor. |
| `#pragma HLS inline` → `#pragma hls_design inline` | **Equivalent** | Both force function inlining. |
| `ap_uint<N>` → `ac_int<N, false>` | **Functional equivalent** | Same width, unsigned semantics. Bit access via `operator[]` works identically. |
| `hls::vector<T,N>` → `vec_wrapper<T,N>` | **Functional equivalent** | Wrapper struct with `data[N]` array and `operator[]`. Same memory layout. |

## Testbench Verdict

**Testbench PASSED** — The translated design produces bit-identical results (max error: 0.0000, within floating-point noise tolerance).

- Compilation: All source files pass `-fsyntax-only` with clang++ -std=c++17
- Execution: `timeout 30s ./testbench.out` returns 0
- The testbench checks were preserved unchanged (max error > 1.0 → failure)