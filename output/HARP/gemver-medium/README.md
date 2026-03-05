# Design Documentation: kernel_gemver

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_gemver(
    int n,
    double alpha,
    double beta,
    double A[400][400],
    double u1[400],
    double v1[400],
    double u2[400],
    double v2[400],
    double w[400],
    double x[400],
    double y[400],
    double z[400]
);
```

## Parameters

- **n**: int scalar
- **alpha**: double scalar
- **beta**: double scalar
- **A**: double array[400 x 400]
- **u1**: double array[400]
- **v1**: double array[400]
- **u2**: double array[400]
- **v2**: double array[400]
- **w**: double array[400]
- **x**: double array[400]
- **y**: double array[400]
- **z**: double array[400]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/gemver-medium_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
