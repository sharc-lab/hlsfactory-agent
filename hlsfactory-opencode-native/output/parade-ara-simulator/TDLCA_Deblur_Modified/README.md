# TDLCA_Deblur_Modified

## Description
#include "../../BenchmarkNode.h" #include "Deblur1LCacc.h" #include "Deblur2LCacc.h" #include "Denoise1LCacc.h" #include "Blur1LCacc.h" #include "Blur2LCacc.h" #include "Blur3LCacc.h" #include <stdint

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `Blur1LCacc.h`
- `Blur2LCacc.h`
- `Blur3LCacc.h`
- `Deblur1LCacc.h`
- `Deblur2LCacc.h`
- `Denoise1LCacc.h`
- `TDLCA_Deblur.cpp`

## Accelerator Modules
Blur1LCacc.h Blur2LCacc.h Blur3LCacc.h Deblur1LCacc.h Deblur2LCacc.h Denoise1LCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`Deblur_ModifiedLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
