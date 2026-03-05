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
    double C[60][70],
    double A[60][80],
    double B[80][70]
);
```

## Parameters

- **ni**: int scalar
- **nj**: int scalar
- **nk**: int scalar
- **alpha**: double scalar
- **beta**: double scalar
- **C**: double array[60 x 70]
- **A**: double array[60 x 80]
- **B**: double array[80 x 70]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/gemm-p_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
