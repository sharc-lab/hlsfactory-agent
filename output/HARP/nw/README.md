# Design Documentation: needwun

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
machsuite

## Function Signature
```c
void needwun(
    char SEQA[128],
    char SEQB[128],
    char alignedA[256],
    char alignedB[256],
    int M[16641],
    char ptr[16641]
);
```

## Parameters

- **SEQA**: char array[128]
- **SEQB**: char array[128]
- **alignedA**: char array[256]
- **alignedB**: char array[256]
- **M**: int array[16641]
- **ptr**: char array[16641]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/machsuite/sources/nw_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
