# HP-FFT-HLS: n256_UF8 (Siemens Catapult HLS Port)

## Description
This is an HLS implementation of an FFT (Fast Fourier Transform) design for FFT size N=256 (EXP2_FFT=8) with unrolling factor UF=8.

FFT with spatial unrolling factor UF=8 and stream-based vector interface. Uses ac_channel<hls_vector<complex<float>, UF*2>> for input/output, bit-reversal via reverse_input_stream_UF8, and template-based FFT_stage_spatial_unroll for each stage. Stage 1 is handled specially via FFT_Stage1_vectorstream_parameterize.

## Top-Level Function
`FFT_TOP`

## Interface
Stream-based (ac_channel<hls_vector<complex<float>, UF*2>>)

## Source Files
- FFT.h - Header with type definitions, constants, and function declarations
- FFT.cpp - Implementation of FFT computation
- testbench.cpp - Testbench with golden reference FFT and verification
- run.tcl - Catapult HLS run script

## Testbench
The testbench generates synthetic input signals (sinusoidal + exponential decay), runs the FFT_TOP function, and compares results against a software golden model (DFT reference). Returns 0 on success (max error < 1.0) and 1 on failure.

## Build
To synthesize with Catapult HLS:
```
catapult -f run.tcl
```

## Original Repository
From HP-FFT-HLS: A General High-Performance FFT Generator Using High-Level Synthesis (FCCM25)