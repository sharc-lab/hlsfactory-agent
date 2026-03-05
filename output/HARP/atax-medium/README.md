# Design Documentation: kernel_atax

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_atax(
    double A[390][410],
    double x[410],
    double y[410],
    double tmp[390]
);
```

## Parameters

- **A**: double array[390 x 410]
- **x**: double array[410]
- **y**: double array[410]
- **tmp**: double array[390]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/atax-medium_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
