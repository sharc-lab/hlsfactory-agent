# Design Documentation: kernel_mvt

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_mvt(
    double x1[400],
    double x2[400],
    double y_1[400],
    double y_2[400],
    double A[400][400]
);
```

## Parameters

- **x1**: double array[400]
- **x2**: double array[400]
- **y_1**: double array[400]
- **y_2**: double array[400]
- **A**: double array[400 x 400]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/mvt-medium_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
