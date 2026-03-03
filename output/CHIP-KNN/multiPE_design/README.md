# CHIP-KNN Multi PE Design

## Overview

This is the multi-Processing Element (PE) version of CHIP-KNNv2, a streaming-based and frequency-optimized K-Nearest Neighbors accelerator for HBM-based FPGAs. This design significantly improves performance through parallelism.

## Architecture

The multi PE design extends the single PE architecture with multiple parallel processing elements:

1. **Multiple Load Units**: Each PE has dedicated load units for parallel data access
2. **Multiple Compute Units**: Parallel distance computation across PEs
3. **Multiple Sort Units**: Parallel sorting within each PE
4. **Hierarchical Merge**: Multi-level merge for combining results from all PEs
5. **Unified Write Unit**: Aggregates results from all PEs

## Configuration

| Parameter | Value | Description |
|-----------|-------|-------------|
| INPUT_DIM | 16 | Number of dimensions in each data point |
| TOP (K) | 10 | Number of nearest neighbors to return |
| NUM_SP_PTS | 4,194,304 | Total number of points in search space |
| DISTANCE_METRIC | 0 (Manhattan) | Distance metric (0=Manhattan, 1=Euclidean) |
| NUM_PE | 4 | Number of parallel processing elements |
| IWIDTH | 512 | Interface width in bits |
| DATA_TYPE | float | Data type for computations |

## Files

- `knn.cpp` - Main kernel implementation with multiple PEs
- `knn.h` - Header file with configuration parameters
- `knn-host.cpp` - Host code for CPU-FPGA communication
- `tb_knn.cpp` - Testbench for verification
- `Makefile` - Build configuration
- `knn.ini` - Connectivity configuration

## Usage

### Compilation
```bash
make build TARGET=hw DEVICE=xilinx_u280_xdma_201920_3
```

### Running
```bash
./knn knn.xclbin
```

## Key Features

- **Parallel Processing**: Multiple PEs work on different data partitions simultaneously
- **HBM Utilization**: Optimized for HBM-based FPGAs (Xilinx U280)
- **Streaming Architecture**: Streaming-based data flow between units
- **Scalable**: Number of PEs configurable based on FPGA resources
- **Hierarchical Merge**: Efficient merging of results from multiple PEs

## Performance Metrics

- Target Frequency: 225 MHz
- Speedup: ~NUM_PE times faster than single PE design
- Throughput: Significantly improved for large-scale KNN search
- Resource Utilization: Higher due to multiple PEs

## Architecture Details

### Data Partitioning
- Search space is partitioned equally among PEs
- Each PE processes its partition independently
- Final merge combines top-K results from all PEs

### Memory Access
- Each PE accesses different HBM channels
- Parallel memory access maximizes bandwidth utilization
- Data partition alignment for efficient transfers

## References

- Paper: "CHIP-KNNv2: A Configurable and High-Performance K-Nearest Neighbors Accelerator on HBM-based FPGAs" (TRETS 2023)
- Authors: Kenneth Liu, Alec Lu, Kartik Samtani, Zhenman Fang, Licheng Guo
- DOI: 10.1145/3616873

## Comparison with Single PE Design

| Feature | Single PE | Multi PE (4x) |
|---------|-----------|---------------|
| Throughput | Baseline | ~4x higher |
| Resource Usage | Lower | Higher |
| HBM Channels | 1 | 4 |
| Use Case | Bandwidth-bound | Compute-bound |
