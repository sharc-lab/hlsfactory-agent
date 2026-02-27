# TDLCA_LPCIP_Desc

## Description
#include "../../BenchmarkNode.h" #include "LPCIPLCacc.h" #include <stdint.h> #include <iostream> #define IMGW                    640 #define IMGH                    480 #define PATCH_H                

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `LPCIPLCacc.h`
- `TDLCA_LPCIP_Desc.cpp`

## Accelerator Modules
LPCIPLCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`LPCIP_DescLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
