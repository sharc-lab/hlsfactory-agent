# Design Documentation: kernel_bicg

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_bicg(
    int m,
    int n,
    double A[410][390],
    double s[390],
    double q[410],
    double p[390],
    double r[410]
);
```

## Parameters

- **m**: int scalar
- **n**: int scalar
- **A**: double array[410 x 390]
- **s**: double array[390]
- **q**: double array[410]
- **p**: double array[390]
- **r**: double array[410]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/bicg-large_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
