# Design Documentation: kernel_gesummv

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_gesummv(
    double alpha,
    double beta,
    double A[250][250],
    double B[250][250],
    double tmp[250],
    double x[250],
    double y[250]
);
```

## Parameters

- **alpha**: double scalar
- **beta**: double scalar
- **A**: double array[250 x 250]
- **B**: double array[250 x 250]
- **tmp**: double array[250]
- **x**: double array[250]
- **y**: double array[250]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/gesummv-medium_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
