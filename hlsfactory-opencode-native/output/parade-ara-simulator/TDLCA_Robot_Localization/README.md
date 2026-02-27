# TDLCA_Robot_Localization

## Description
#include "../../BenchmarkNode.h" #include "RobLocLCacc.h" #include <stdint.h> #include <cmath> #include <cstdlib> #include <iostream> #define N_VAL               3 

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `RobLocLCacc.h`
- `TDLCA_Robot_Localization.cpp`

## Accelerator Modules
RobLocLCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`Robot_LocalizationLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
