# Design Documentation: kernel_doitgen

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_doitgen(
    int nr,
    int nq,
    int np,
    double A[25][20],
    double C4[30][30],
    double sum[30]
);
```

## Parameters

- **nr**: int scalar
- **nq**: int scalar
- **np**: int scalar
- **A**: double array[25 x 20]
- **C4**: double array[30 x 30]
- **sum**: double array[30]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/doitgen_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
