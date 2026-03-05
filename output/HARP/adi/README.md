# Design Documentation: kernel_adi

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_adi(
    int tsteps,
    int n,
    double u[60][60],
    double v[60][60],
    double p[60][60],
    double q[60][60]
);
```

## Parameters

- **tsteps**: int scalar
- **n**: int scalar
- **u**: double array[60 x 60]
- **v**: double array[60 x 60]
- **p**: double array[60 x 60]
- **q**: double array[60 x 60]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/adi_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
