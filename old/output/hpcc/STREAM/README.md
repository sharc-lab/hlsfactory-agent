# STREAM Design - HPC Challenge Benchmark

## Description
STREAM is a synthetic benchmark program that measures sustainable memory bandwidth (in MB/s) and the corresponding computation rate for simple vector kernels.

## Top-Level Functions
- `STREAM_Copy()` - a[i] = c[i]
- `STREAM_Scale()` - b[i] = scalar * c[i]
- `STREAM_Add()` - c[i] = a[i] + b[i]
- `STREAM_Triad()` - a[i] = b[i] + scalar * c[i]

## Source Files
- `stream.c` - Main STREAM benchmark implementation

## Key Features
- Four vector kernels: Copy, Scale, Add, Triad
- Memory-bound operations
- Measures sustainable memory bandwidth
- Configurable array size and iteration count

## HLS Considerations
- Memory bandwidth is the key bottleneck
- All kernels are simple loops with predictable access patterns
- Suitable for memory interface optimization
- Can exploit burst transfers
- Dataflow can be used between kernels
