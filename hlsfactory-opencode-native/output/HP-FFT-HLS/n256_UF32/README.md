# n256_UF32 Design

## Overview
256-point FFT implementation using High-Level Synthesis (HLS).

## Design Parameters
- **FFT Size**: 256 points (8 stages)
- **Variant**: UF32
- **Unroll Factor**: 32

## Files
- FFT.h - Header with parameters and top function declaration
- FFT.cpp - HLS implementation
- testbench.cpp - Verification testbench
- synth.tcl - Vivado HLS synthesis script
- ap_int.h, ap_fixed.h, hls_stream.h, hls_vector.h, hls_streamofblocks.h, hls_fft.h - HLS library stubs

## Top Function
```cpp
void FFT_TOP(hls::stream<hls::vector<complex<float>, UF*2>> & in, 
    hls::stream<hls::vector<complex<float>, UF*2>> & out
);
```

## Key Features
- Radix-2 Decimation-in-Time (DIT) FFT algorithm
- Spatial unrolling for parallelism
- Double-buffering between stages
- Optimized twiddle factor computation
- Bit-reversal permutation

## Compilation
```bash
clang++ -std=c++17 -c FFT.cpp -I.
clang++ -std=c++17 -c testbench.cpp -I.
```

## Synthesis
```bash
vivado_hls synth.tcl
```

## References
Part of the HP-FFT-HLS project from UCLA VAST group.
Publication: FCCM25 - "HP-FFT: A General High-Performance FFT Generator Using High-Level Synthesis"
