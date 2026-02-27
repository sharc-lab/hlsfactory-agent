# TDLCA_Denoise

## Description
#include "../../BenchmarkNode.h" #include "Denoise1LCacc.h" #include "Denoise2LCacc.h" #include <stdint.h> #include <iostream> #define ITER_COUNT 2 

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `Denoise1LCacc.h`
- `Denoise2LCacc.h`
- `TDLCA_Denoise.cpp`

## Accelerator Modules
Denoise1LCacc.h Denoise2LCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`DenoiseLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
