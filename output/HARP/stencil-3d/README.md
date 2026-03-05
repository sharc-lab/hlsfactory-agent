# Design Documentation: stencil3d

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
machsuite

## Function Signature
```c
void stencil3d(
    long orig[39304],
    long sol[32768]
);
```

## Parameters

- **orig**: long array[39304]
- **sol**: long array[32768]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/machsuite/sources/stencil-3d_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
