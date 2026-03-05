# n256_UF8 - HP-FFT HLS Design

## Overview
This is an HLS implementation of a High-Performance Fast Fourier Transform (FFT) from the UCLA-VAST HP-FFT-HLS repository.

## Design Parameters
- **FFT Size**: 256 points
- **Design Variant**: UF8
- **Unroll Factor (UF)**: 8

## Files
- `FFT.cpp` - Main HLS design implementation
- `FFT.h` - Header file with design parameters and function declarations
- `testbench.cpp` - Testbench for verification
- `compile_log.txt` - Compilation log showing successful compilation

## Top Function
`FFT_TOP` - The main FFT function that processes input data through hls::stream interfaces.

## Interfaces
- Input: `hls::stream<hls::vector<complex<float>, UF*2>>>& in`
- Output: `hls::stream<hls::vector<complex<float>, UF*2>>>& out`

## Algorithm
This design implements a radix-2 Decimation-In-Time (DIT) FFT algorithm with:
- Bit-reversal input reordering
- Spatial unrolling based on the UF parameter
- Double-buffering for data flow optimization
- Template-based stage implementation for compile-time optimization

## Compilation
Successfully compiled with clang++ -std=c++17

## Source Repository
https://github.com/UCLA-VAST/HP-FFT-HLS.git
