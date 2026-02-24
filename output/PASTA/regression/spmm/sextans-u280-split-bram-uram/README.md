# spmm/sextans-u280-split-bram-uram HLS Design

## Overview
This is an HLS design extracted from the PASTA repository.

## Design Information
- **Name**: spmm/sextans-u280-split-bram-uram
- **Source**: PASTA Repository (https://github.com/SFU-HiAccel/PASTA)
- **Extraction Date**: 2026-02-24 03:31:07

## Source Files

### Kernel Files (HLS)
- `sextans.cpp`

### Host Files (Testbench)
- `sextans-host.cpp`

### Header Files
- `mmio.h`
- `modules.h`
- `sextans.h`
- `sparse_helper.h`

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
