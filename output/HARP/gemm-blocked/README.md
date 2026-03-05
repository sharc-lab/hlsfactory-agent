# Design Documentation: bbgemm

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
machsuite

## Function Signature
```c
void bbgemm(
    double m1[4096],
    double m2[4096],
    double prod[4096]
);
```

## Parameters

- **m1**: double array[4096]
- **m2**: double array[4096]
- **prod**: double array[4096]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/machsuite/sources/gemm-blocked_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
