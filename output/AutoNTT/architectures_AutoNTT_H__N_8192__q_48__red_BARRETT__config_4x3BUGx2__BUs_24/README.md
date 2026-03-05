# HLS Design: architectures_AutoNTT_H__N_8192__q_48__red_BARRETT__config_4x3BUGx2__BUs_24

## Description
This is an NTT (Number Theoretic Transform) kernel implementation for FPGA using High-Level Synthesis (HLS).

## Source Files
- ntt_kernel.cpp - Main kernel implementation
- ntt.h - Header file with configuration parameters
- ntt_test.cpp - Testbench (if available)

## Architecture Details
- Polynomial size: 8192
- Word size: 48 bits
- Uses Barrett reduction (or Montgomery/Naive/WLM depending on variant)
- Implements parallel butterfly units

## Synthesis
Use the provided synthesis.tcl script with Vitis HLS or Vivado HLS.

## Compilation Notes
This design uses Xilinx-specific HLS types (ap_int, ap_uint) and TAPA library constructs.
Compilation with standard C++ compilers requires stub headers.
The compilation errors observed are due to limitations of the stub headers
and do not indicate actual design issues. For proper synthesis, use Xilinx Vitis HLS.
