# FFT Design - HPC Challenge Benchmark

## Description
This is a 1D Complex FFT implementation from the FFTE (Fast Fourier Transform Package) by Daisuke Takahashi. It supports radix-2, 3, 4, 5, and 8 FFT operations and can handle vector sizes that are products of powers of 2, 3, and 5.

## Top-Level Function
- `HPCC_zfft1d()` - Main 1D complex FFT function
- `HPCC_fft235()` - Radix-2, 3, 5, 4, and 8 FFT kernel

## Source Files
- `zfft1d.c` - Main FFT implementation with 1D transform logic
- `fft235.c` - Core FFT kernels (radix-2, 3, 4, 5, 8)
- `hpccfft.h` - Header file with macros and function declarations
- `wrapfftw.h` - FFTW wrapper interface
- `wrapfftw.c` - FFTW wrapper implementation
- `bcnrand.c` - Random number generation utilities

## Key Features
- Supports mixed-radix FFT (2, 3, 5 factors)
- Optimized for memory access patterns
- Cache blocking with configurable block size (FFTE_NBLK)
- Padding to avoid cache conflicts (FFTE_NP)

## HLS Considerations
- Uses double-precision complex numbers
- Has loop nests suitable for pipelining
- Memory bandwidth is the limiting factor
- OpenMP pragmas can be replaced with HLS pragmas
