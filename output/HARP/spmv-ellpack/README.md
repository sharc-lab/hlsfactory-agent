# Design Documentation: ellpack

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
machsuite

## Function Signature
```c
void ellpack(
    double nzval[4940],
    int cols[4940],
    double vec[494],
    double out[494]
);
```

## Parameters

- **nzval**: double array[4940]
- **cols**: int array[4940]
- **vec**: double array[494]
- **out**: double array[494]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/machsuite/sources/spmv-ellpack_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
