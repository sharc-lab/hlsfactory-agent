# Design Documentation: kernel_heat_3d

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_heat_3d(
    int tsteps,
    int n,
    double A[20][20],
    double B[20][20]
);
```

## Parameters

- **tsteps**: int scalar
- **n**: int scalar
- **A**: double array[20 x 20]
- **B**: double array[20 x 20]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/heat-3d_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
