# Design Documentation: kernel_syr2k

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_syr2k(
    double alpha,
    double beta,
    double C[80][80],
    double A[80][60],
    double B[80][60]
);
```

## Parameters

- **alpha**: double scalar
- **beta**: double scalar
- **C**: double array[80 x 80]
- **A**: double array[80 x 60]
- **B**: double array[80 x 60]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/syr2k_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
