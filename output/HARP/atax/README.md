# Design Documentation: kernel_atax

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_atax(
    int m,
    int n,
    double A[116][124],
    double x[124],
    double y[124],
    double tmp[116]
);
```

## Parameters

- **m**: int scalar
- **n**: int scalar
- **A**: double array[116 x 124]
- **x**: double array[124]
- **y**: double array[124]
- **tmp**: double array[116]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/atax_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
