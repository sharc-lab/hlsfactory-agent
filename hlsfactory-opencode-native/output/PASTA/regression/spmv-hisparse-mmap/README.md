# spmv-hisparse-mmap HLS Design

## Overview
This is an HLS design extracted from the PASTA repository.

## Design Information
- **Name**: spmv-hisparse-mmap
- **Source**: PASTA Repository (https://github.com/SFU-HiAccel/PASTA)
- **Extraction Date**: 2026-02-24 03:31:09

## Source Files

### Kernel Files (HLS)
- `spmv.cpp`
- `spmv.cpp`

### Host Files (Testbench)
- `host.cpp`

### Header Files
- `data_formatter.h`
- `data_loader.h`
- `common.h`
- `pe.h`
- `shuffle.h`
- `spmv_cluster.h`
- `stream_utils.h`
- `vecbuf_access_unit.h`

## Build Instructions

### Compilation
```bash
# Compile with clang++ (using HLS stubs)
clang++ -c -I/path/to/stubs -I. kernel.cpp -o kernel.o
```

### Synthesis
Use the generated TCL script for Vivado HLS/Vitis HLS synthesis.

## Notes
- This design uses the TAPA (Task-Parallel High-Level Synthesis) framework
- Original repository: https://github.com/SFU-HiAccel/PASTA
