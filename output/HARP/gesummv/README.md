# Design Documentation: kernel_gesummv

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_gesummv(
    int n,
    double alpha,
    double beta,
    double A[90][90],
    double B[90][90],
    double tmp[90],
    double x[90],
    double y[90]
);
```

## Parameters

- **n**: int scalar
- **alpha**: double scalar
- **beta**: double scalar
- **A**: double array[90 x 90]
- **B**: double array[90 x 90]
- **tmp**: double array[90]
- **x**: double array[90]
- **y**: double array[90]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/gesummv_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
