# TDLCA_EKF_SLAM

## Description
#include "../../BenchmarkNode.h" #include "JacobiansLCacc.h" #include "SphericalCoordsLCacc.h" #include <stdint.h> #include <iostream> #include <cmath> 

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `JacobiansLCacc.h`
- `SphericalCoordsLCacc.h`
- `TDLCA_EKF_SLAM.cpp`

## Accelerator Modules
JacobiansLCacc.h SphericalCoordsLCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`EKF_SLAMLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
