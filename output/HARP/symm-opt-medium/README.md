# Design Documentation: kernel_symm

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_symm(
    double alpha,
    double beta,
    double C[200][240],
    double A[200][200],
    double B[200][240]
);
```

## Parameters

- **alpha**: double scalar
- **beta**: double scalar
- **C**: double array[200 x 240]
- **A**: double array[200 x 200]
- **B**: double array[200 x 240]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/symm-opt-medium_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
