# Design Documentation: aes256_encrypt_ecb

## Overview
This is an HLS design from the HARP benchmark suite.

## Category
machsuite

## Function Signature
```c
void aes256_encrypt_ecb(
    char k[32],
    char buf[16]
);
```

## Parameters

- **k**: char array[32]
- **buf**: char array[16]

## HLS Pragmas
This design uses the following HLS pragmas:
- `#pragma ACCEL kernel` - Marks the function as an HLS kernel
- `#pragma ACCEL PIPELINE` - Pipeline directives
- `#pragma ACCEL TILE` - Tile optimization
- `#pragma ACCEL PARALLEL` - Parallelization directives

## Source File
`/workspace/harp/dse_database/machsuite/sources/aes_kernel.c`

## Notes
This design is part of the UCLA VAST HARP (HLS Modeling) project.
