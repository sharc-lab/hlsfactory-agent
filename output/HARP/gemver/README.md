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
    double A[120][120],
    double u1[120],
    double v1[120],
    double u2[120],
    double v2[120],
    double w[120],
    double x[120],
    double y[120],
    double z[120]
);
```

## Parameters

- **n**: int scalar
- **alpha**: double scalar
- **beta**: double scalar
- **A**: double array[120 x 120]
- **u1**: double array[120]
- **v1**: double array[120]
- **u2**: double array[120]
- **v2**: double array[120]
- **w**: double array[120]
- **x**: double array[120]
- **y**: double array[120]
- **z**: double array[120]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/gemver_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
