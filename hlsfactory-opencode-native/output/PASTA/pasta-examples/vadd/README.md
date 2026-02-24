# vadd HLS Design

## Overview
This is an HLS design extracted from the PASTA repository.

## Design Information
- **Name**: vadd
- **Source**: PASTA Repository (https://github.com/SFU-HiAccel/PASTA)
- **Extraction Date**: 2026-02-24 03:30:47

## Source Files

### Kernel Files (HLS)
- `add.cpp`

### Host Files (Testbench)
- `add-host.cpp`

### Header Files
- `add.h`

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
