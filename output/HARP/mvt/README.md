# Design Documentation: kernel_mvt

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_mvt(
    double x1[120],
    double x2[120],
    double y_1[120],
    double y_2[120],
    double A[120][120]
);
```

## Parameters

- **x1**: double array[120]
- **x2**: double array[120]
- **y_1**: double array[120]
- **y_2**: double array[120]
- **A**: double array[120 x 120]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/mvt_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
