# TDLCA_Segmentation

## Description
#include "../../BenchmarkNode.h" #include "Segmentation1LCacc.h" #include <stdint.h> #include <iostream> #include <cmath> #define ITER_COUNT 2 

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `Segmentation1LCacc.h`
- `TDLCA_Segmentation.cpp`

## Accelerator Modules
Segmentation1LCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`SegmentationLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
