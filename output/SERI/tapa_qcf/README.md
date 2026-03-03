# tapa_qcf

## Overview

This is an HLS design extracted from the SERI (Streaming Accelerator for Electron Repulsion Integrals) repository.

**Source File:** `tapa_qcf.cpp`

## Description

This kernel is part of the quantum chemistry FPGA acceleration system for computing electron repulsion integrals (ERIs).

## Design Details

- **Repository:** SFU-HiAccel/SERI
- **Kernel Type:** Vitis HLS C++ Kernel
- **Target Platform:** Xilinx Alveo U280 FPGA

## Dependencies

- Vitis HLS 2023.2
- Xilinx Runtime (XRT)
- TAPA (Task-Parallel High-Level Synthesis) library (for TAPA designs)

## Usage

### Synthesis
```bash
vitis_hls -f script.tcl
```

### Compilation
```bash
clang++ -c -std=c++14 -I<path_to_hls_includes> tapa_qcf.cpp
```

## Kernel Interface

See the source code for detailed interface specifications.

## Notes

- This design requires specific angular momentum configuration (AM_ABCD)
- Part of a multi-kernel dataflow architecture
- Optimized for HBM-based FPGAs
