# Design Documentation: kernel_trmm

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_trmm(
    double alpha,
    double A[60][60],
    double B[60][80]
);
```

## Parameters

- **alpha**: double scalar
- **A**: double array[60 x 60]
- **B**: double array[60 x 80]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/trmm-opt_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
