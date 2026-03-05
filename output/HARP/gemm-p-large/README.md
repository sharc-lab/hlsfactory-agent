# Design Documentation: kernel_gemm

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_gemm(
    int ni,
    int nj,
    int nk,
    double alpha,
    double beta,
    double C[200][220],
    double A[200][240],
    double B[240][220]
);
```

## Parameters

- **ni**: int scalar
- **nj**: int scalar
- **nk**: int scalar
- **alpha**: double scalar
- **beta**: double scalar
- **C**: double array[200 x 220]
- **A**: double array[200 x 240]
- **B**: double array[240 x 220]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/gemm-p-large_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
