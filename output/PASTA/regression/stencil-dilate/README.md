# stencil-dilate HLS Design

## Overview
This is an HLS design extracted from the PASTA repository.

## Design Information
- **Name**: stencil-dilate
- **Source**: PASTA Repository (https://github.com/SFU-HiAccel/PASTA)
- **Extraction Date**: 2026-02-24 03:31:11

## Source Files

### Kernel Files (HLS)
- `main.cpp`
- `unikernel.cpp`

### Header Files
- `DILATE.h`

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
