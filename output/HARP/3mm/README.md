# Design Documentation: kernel_3mm

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_3mm(
    int ni,
    int nj,
    int nk,
    int nl,
    int nm,
    double E[40][50],
    double A[40][60],
    double B[60][50],
    double F[50][70],
    double C[50][80],
    double D[80][70],
    double G[40][70]
);
```

## Parameters

- **ni**: int scalar
- **nj**: int scalar
- **nk**: int scalar
- **nl**: int scalar
- **nm**: int scalar
- **E**: double array[40 x 50]
- **A**: double array[40 x 60]
- **B**: double array[60 x 50]
- **F**: double array[50 x 70]
- **C**: double array[50 x 80]
- **D**: double array[80 x 70]
- **G**: double array[40 x 70]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/3mm_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
