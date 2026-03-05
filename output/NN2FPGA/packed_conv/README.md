# packed_conv - HLS Design

## Overview

This is an HLS design module from the NN2FPGA framework for quantized neural network accelerators on AMD/Xilinx FPGAs.

## File Structure

- `src/` - Source files including headers and HLS kernels
- `testbench/` - Testbench for verification
- `tcl/` - Synthesis scripts
- `compile_log.txt` - Compilation results

## HLS Pragmas

#pragma HLS expression_balance, #pragma HLS array_partition, #pragma HLS aggregate, #pragma HLS pipeline, #pragma HLS inline

## Functions/Kernels

double_packing_debug, conv_pipe, conv_comp, conv_comp_onchip_OW_OPS_OUT, conv_comp_onchip_ICH, conv_comp_onchip_OCH, conv_comp_onchip, conv_comp_wrap

## Dependencies

- ap_int.h - Arbitrary precision integer types
- ap_fixed.h - Arbitrary precision fixed-point types
- hls_stream.h - HLS streaming interfaces
- ap_axi_sdata.h - AXI stream data types

## Repository Information

- **Source**: https://github.com/robertoBosio/NN2FPGA.git
- **License**: MIT License
- **Framework**: NN2FPGA - Neural Network to FPGA

## Usage

This design uses C++ templates for flexibility. To use in your HLS project:

1. Include the header file
2. Instantiate templates with appropriate types
3. Call functions from your top-level HLS kernel

## Notes

This is a template-based HLS library component. Specific instantiation requires
defining template parameters based on your neural network architecture.
