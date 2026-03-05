# Design Documentation: kernel_fdtd_2d

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_fdtd_2d(
    int tmax,
    int nx,
    int ny,
    double ex[200][240],
    double ey[200][240],
    double hz[200][240],
    double _fict_[100]
);
```

## Parameters

- **tmax**: int scalar
- **nx**: int scalar
- **ny**: int scalar
- **ex**: double array[200 x 240]
- **ey**: double array[200 x 240]
- **hz**: double array[200 x 240]
- **_fict_**: double array[100]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/fdtd-2d-large_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
