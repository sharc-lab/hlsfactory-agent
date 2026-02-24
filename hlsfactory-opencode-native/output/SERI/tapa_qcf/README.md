# tapa_qcf

## Overview

This is an HLS kernel from the SERI (Streaming Accelerator for Electron Repulsion Integrals) project.

**Source:** SFU-HiAccel/SERI Repository  
**Type:** HLS C++ Kernel  
**Target Platform:** Xilinx Alveo U280

## Description

HLS kernel for quantum chemistry ERI computation.

## Interface

### Function Signature
```cpp
void t_preparation( tapa::mmap<const qp_t> abcd_inp, tapa::mmap<const D_WIDTH( qcf::util::next_pow2(sizeof(rys_t)
```

### Arguments
See source code for detailed argument list.

## File Structure

```
tapa_qcf/
├── tapa_qcf.cpp          # Kernel source code
├── tapa_qcf_tb.cpp       # Testbench
├── src/                        # Header files
│   ├── common/                # Common utilities
│   └── device/                # Device-specific headers
└── stubs/                      # HLS stub headers
    ├── ap_int.h
    ├── ap_fixed.h
    └── hls_stream.h
```

## Build Instructions

### Compilation (Software Emulation)
```bash
clang++ -c -std=c++17 -I./stubs -I./src/common -I./src/device tapa_qcf.cpp
```

### HLS Synthesis
```bash
vitis_hls -f run_hls.tcl
```

## Dependencies

- Vitis HLS / Vivado HLS
- Xilinx Runtime (XRT)
- C++17 compatible compiler

## Notes

This design is part of a quantum chemistry ERI computation accelerator. It requires specific angular momentum (AM_ABCD) configuration to be defined at compile time.

See the main SERI repository for complete documentation and build instructions.
