# Design Documentation: kernel_covariance

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
poly

## Function Signature
```c
void kernel_covariance(
    int m,
    int n,
    double float_n,
    double data[100][80],
    double cov[80][80],
    double mean[80]
);
```

## Parameters

- **m**: int scalar
- **n**: int scalar
- **float_n**: double scalar
- **data**: double array[100 x 80]
- **cov**: double array[80 x 80]
- **mean**: double array[80]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/poly/sources/covariance_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
