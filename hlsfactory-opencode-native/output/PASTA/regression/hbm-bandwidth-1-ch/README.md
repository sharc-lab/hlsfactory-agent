# hbm-bandwidth-1-ch HLS Design

## Overview
This is an HLS design extracted from the PASTA repository.

## Design Information
- **Name**: hbm-bandwidth-1-ch
- **Source**: PASTA Repository (https://github.com/SFU-HiAccel/PASTA)
- **Extraction Date**: 2026-02-24 03:30:56

## Source Files

### Kernel Files (HLS)
- `bandwidth.cpp`

### Host Files (Testbench)
- `bandwidth-host.cpp`

### Header Files
- `bandwidth.h`

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
