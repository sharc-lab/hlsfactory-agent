# TDLCA_BlackScholes

## Description
#include "../../BenchmarkNode.h" #include "BlackScholesLCacc.h" #include <stdint.h> #include <iostream> 

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `BlackScholesLCacc.h`
- `TDLCA_BlackScholes.cpp`

## Accelerator Modules
BlackScholesLCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`BlackScholesLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
