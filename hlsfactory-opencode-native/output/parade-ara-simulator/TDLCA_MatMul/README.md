# TDLCA_MatMul

## Description
#include "../../BenchmarkNode.h" #include <stdint.h> #include "MatMulLCacc.h" 

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `MatMulLCacc.h`
- `TDLCA_MatMul.cpp`

## Accelerator Modules
MatMulLCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`MatMulLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
