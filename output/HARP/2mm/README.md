# Design Documentation: kernel_2mm

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_2mm(
    int ni,
    int nj,
    int nk,
    int nl,
    double alpha,
    double beta,
    double tmp[40][50],
    double A[40][70],
    double B[70][50],
    double C[50][80],
    double D[40][80]
);
```

## Parameters

- **ni**: int scalar
- **nj**: int scalar
- **nk**: int scalar
- **nl**: int scalar
- **alpha**: double scalar
- **beta**: double scalar
- **tmp**: double array[40 x 50]
- **A**: double array[40 x 70]
- **B**: double array[70 x 50]
- **C**: double array[50 x 80]
- **D**: double array[40 x 80]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/2mm_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
