# Design Documentation: stencil

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
machsuite

## Function Signature
```c
void stencil(
    int orig[8192],
    int sol[8192],
    int filter[9]
);
```

## Parameters

- **orig**: int array[8192]
- **sol**: int array[8192]
- **filter**: int array[9]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/machsuite/sources/stencil_stencil2d_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
