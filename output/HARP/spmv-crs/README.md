# Design Documentation: spmv

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
machsuite

## Function Signature
```c
void spmv(
    double val[1666],
    int cols[1666],
    int rowDelimiters[495],
    double vec[494],
    double out[494]
);
```

## Parameters

- **val**: double array[1666]
- **cols**: int array[1666]
- **rowDelimiters**: int array[495]
- **vec**: double array[494]
- **out**: double array[494]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/machsuite/sources/spmv-crs_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
