# RandomAccess Design - HPC Challenge Benchmark

## Description
RandomAccess measures the rate of integer random updates of memory (GUPS - Giga Updates Per Second). It performs read-modify-write operations on a large table with randomly generated addresses.

## Top-Level Functions
- `HPCC_RandomAccess()` - Main RandomAccess test function
- `HPCC_SingleRandomAccess()` - Single CPU version with MPI support
- `HPCC_starts()` - Random number generator initialization
- `RandomAccessUpdate()` - Core update kernel

## Source Files
- `single_cpu.c` - Single CPU RandomAccess test
- `core_single_cpu.c` - Core computational kernel
- `RandomAccess.h` - Header file

## Key Features
- Measures memory subsystem performance
- Random memory access patterns
- Uses Linear Congruential Generator for random numbers
- Read-modify-write operations on 64-bit words

## HLS Considerations
- Random memory access pattern is challenging for HLS
- Memory coalescing not possible
- Consider caching frequently accessed regions
- Hash-based partitioning can improve parallelism
- Memory bandwidth limited by random access latency
