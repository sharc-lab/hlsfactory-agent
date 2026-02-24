# serpens-32ch HLS Design

## Overview
This is an HLS design extracted from the PASTA repository.

## Design Information
- **Name**: serpens-32ch
- **Source**: PASTA Repository (https://github.com/SFU-HiAccel/PASTA)
- **Extraction Date**: 2026-02-24 03:31:05

## Source Files

### Kernel Files (HLS)
- `serpens.cpp`
- `serpens_tapa.cpp`
- `serpens_tapa_mmap.cpp`

### Host Files (Testbench)
- `host.cpp`

### Header Files
- `mmio.h`
- `sparse_helper.h`
- `mmio.h`

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
