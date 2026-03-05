# Design Documentation: md_kernel

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
machsuite

## Function Signature
```c
void md_kernel(
    double force_x[256],
    double force_y[256],
    double force_z[256],
    double position_x[256],
    double position_y[256],
    double position_z[256],
    int NL[4096]
);
```

## Parameters

- **force_x**: double array[256]
- **force_y**: double array[256]
- **force_z**: double array[256]
- **position_x**: double array[256]
- **position_y**: double array[256]
- **position_z**: double array[256]
- **NL**: int array[4096]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/machsuite/sources/md_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
