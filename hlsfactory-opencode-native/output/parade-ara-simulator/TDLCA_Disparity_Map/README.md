# TDLCA_Disparity_Map

## Description
#include "../../BenchmarkNode.h" #include "DispMapCompSADLCacc.h" #include "DispMapIntegSum1LCacc.h" #include "DispMapIntegSum2LCacc.h" #include "DispMapFindDispLCacc.h" #include <stdint.h> #include <

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `DispMapCompSADLCacc.h`
- `DispMapFindDispLCacc.h`
- `DispMapIntegSum1LCacc.h`
- `DispMapIntegSum2LCacc.h`
- `TDLCA_Disparity_Map.cpp`

## Accelerator Modules
DispMapCompSADLCacc.h DispMapFindDispLCacc.h DispMapIntegSum1LCacc.h DispMapIntegSum2LCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`Disparity_MapLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
