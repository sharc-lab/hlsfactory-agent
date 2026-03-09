# DGEMM Design - HPC Challenge Benchmark

## Description
DGEMM (Double-precision General Matrix Multiply) performs matrix-matrix multiplication. It computes C = alpha * A * B + beta * C where A, B, and C are matrices.

## Top-Level Function
- `HPCC_TestDGEMM()` - Main DGEMM test function
- `dmatgen()` - Random matrix generator
- `dnrm_inf()` - Infinity norm computation

## Source Files
- `tstdgemm.c` - DGEMM implementation

## Key Features
- Double-precision floating point operations
- Matrix generation with random values
- Residual checking for numerical accuracy
- Performance measurement in GFLOPS

## HLS Considerations
- Compute-bound kernel with O(N^3) operations
- Memory access patterns can be optimized with tiling
- Systolic array architecture suitable
- Loop unrolling and pipelining opportunities
- Data reuse in local buffers critical for performance
