# TDLCA_Registration_Modified

## Description
#include "../../BenchmarkNode.h" #include "Registration1LCacc.h" #include "Blur1LCacc.h" #include "Blur2LCacc.h" #include "Blur3LCacc.h" #include <stdint.h> #include <iostream> #include <cmath> #defin

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `Blur1LCacc.h`
- `Blur2LCacc.h`
- `Blur3LCacc.h`
- `Registration1LCacc.h`
- `TDLCA_Registration.cpp`

## Accelerator Modules
Blur1LCacc.h Blur2LCacc.h Blur3LCacc.h Registration1LCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`Registration_ModifiedLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
