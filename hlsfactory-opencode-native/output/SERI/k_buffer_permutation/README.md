# k_buffer_permutation

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
void k_buffer_permutation( hls::stream<D_WIDTH( ucba_pt::bit_width / 2 )
```

### Arguments
- `ucba_stream_in_1`: Stream interface
- `n_stream_in`: Stream interface
- `ucba_stream_in_0`: Stream interface
- `n_stream_out`: Stream interface

## File Structure

```
k_buffer_permutation/
├── k_buffer_permutation.cpp          # Kernel source code
├── k_buffer_permutation_tb.cpp       # Testbench
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
clang++ -c -std=c++17 -I./stubs -I./src/common -I./src/device k_buffer_permutation.cpp
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
