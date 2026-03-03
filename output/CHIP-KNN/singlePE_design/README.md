# CHIP-KNN Single PE Design

## Overview

This is the single Processing Element (PE) version of CHIP-KNN, a configurable and high-performance K-Nearest Neighbors accelerator for cloud FPGAs. The design is capable of saturating DDR bandwidth with double-buffering techniques.

## Architecture

The single PE design consists of the following components:

1. **Load Unit (`load_KNN`)**: Loads query and search space data from off-chip memory using 512-bit wide interfaces
2. **Compute Unit (`compute_KNN`)**: Computes Manhattan or Euclidean distance between query point and search space points
3. **Sort Unit (`para_partial_sort`)**: Parallel partial sort to find K nearest neighbors
4. **Hierarchical Merge Unit (`merge_dual_streams`)**: Merges results from multiple segments
5. **Write Unit (`write_KNN`)**: Writes final KNN results back to memory

## Configuration

| Parameter | Value | Description |
|-----------|-------|-------------|
| INPUT_DIM | 16 | Number of dimensions in each data point |
| TOP (K) | 10 | Number of nearest neighbors to return |
| NUM_SP_PTS | 4,194,304 | Total number of points in search space |
| DISTANCE_METRIC | 0 (Manhattan) | Distance metric (0=Manhattan, 1=Euclidean) |
| NUM_PE | 1 | Number of processing elements |
| IWIDTH | 512 | Interface width in bits |
| DATA_TYPE | float | Data type for computations |

## Files

- `knn.cpp` - Main kernel implementation
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

- **Memory Interface**: 512-bit HBM2 interface for high bandwidth
- **Parallel Sort**: Parallel partial sort with configurable II
- **Data Types**: Supports float and fixed-point data types
- **Segmented Processing**: Divides data into segments for efficient processing

## Performance Metrics

- Target Frequency: 225 MHz
- Interface Bandwidth: Saturates HBM bandwidth
- Throughput: Optimized for large-scale KNN search

## References

- Paper: "CHIP-KNN: A Configurable and High-Performance K-Nearest Neighbors Accelerator on Cloud FPGAs" (ICFPT 2020)
- Authors: Alec Lu, Zhenman Fang, Nazanin Farahpour, Lesley Shannon
