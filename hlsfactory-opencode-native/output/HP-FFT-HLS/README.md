# HP-FFT-HLS Design Collection

This directory contains HLS designs extracted from the HP-FFT-HLS repository.

## Repository Information
- **Source**: https://github.com/UCLA-VAST/HP-FFT-HLS
- **Designs**: 16 FFT implementations
- **FFT Sizes**: 256-point, 1024-point
- **Variants**: Different unrolling factors and optimizations

## Design Variants

### n256 (256-point FFT)
- n256_UF1 - Unroll Factor 1
- n256_UF2 - Unroll Factor 2
- n256_UF4 - Unroll Factor 4
- n256_UF8 - Unroll Factor 8
- n256_UF16 - Unroll Factor 16
- n256_UF32 - Unroll Factor 32
- n256_no_StagePipeline - No stage pipeline optimization
- n256_original_C_style - Original C-style implementation

### n1024 (1024-point FFT)
- n1024_UF1 - Unroll Factor 1
- n1024_UF2 - Unroll Factor 2
- n1024_UF4 - Unroll Factor 4
- n1024_UF8 - Unroll Factor 8
- n1024_UF16 - Unroll Factor 16
- n1024_UF32 - Unroll Factor 32
- n1024_no_StagePipeline - No stage pipeline optimization
- n1024_original_C_style - Original C-style implementation

## Files per Design
- FFT.h - Header file with design parameters
- FFT.cpp - HLS source code
- testbench.cpp - Testbench for verification
- synth.tcl - Vivado HLS synthesis script
- README.md - Design-specific documentation

## Compilation
All designs have been compiled successfully with clang++-17.

## Synthesis
Run Vivado HLS with: `vivado_hls synth.tcl`
