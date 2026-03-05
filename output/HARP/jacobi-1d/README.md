# Design Documentation: kernel_jacobi_1d

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_jacobi_1d(
    int tsteps,
    int n,
    double A[120],
    double B[120]
);
```

## Parameters

- **tsteps**: int scalar
- **n**: int scalar
- **A**: double array[120]
- **B**: double array[120]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/jacobi-1d_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
