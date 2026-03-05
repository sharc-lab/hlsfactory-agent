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
    double A[124][116],
    double s[116],
    double q[124],
    double p[116],
    double r[124]
);
```

## Parameters

- **m**: int scalar
- **n**: int scalar
- **A**: double array[124 x 116]
- **s**: double array[116]
- **q**: double array[124]
- **p**: double array[116]
- **r**: double array[124]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/bicg_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
