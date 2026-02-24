# k_find_bmax

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
void k_find_bmax( #if GQ_KERNEL_SPLIT == 2 hls::stream<D_WIDTH( ba_cart_eris_a_pt::bit_width / 2 )
```

### Arguments
- `partial_eris_a_stream_in_0`: Stream interface
- `n_stream_in`: Stream interface
- `partial_eris_a_stream_in_1`: Stream interface
- `cart_eris_stream_out_0`: Stream interface
- `partial_eris_b_stream_in_1`: Stream interface
- `partial_eris_b_stream_in_0`: Stream interface
- `b_max_stream_out`: Stream interface
- `cart_eris_stream_out_1`: Stream interface

## File Structure

```
k_find_bmax/
├── k_find_bmax.cpp          # Kernel source code
├── k_find_bmax_tb.cpp       # Testbench
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
clang++ -c -std=c++17 -I./stubs -I./src/common -I./src/device k_find_bmax.cpp
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
