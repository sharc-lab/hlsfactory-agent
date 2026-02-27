# TDLCA_StreamCluster

## Description
#include "../../BenchmarkNode.h" #include "StreamCluster1LCacc.h" #include "StreamCluster3LCacc.h" #include "StreamCluster4LCacc.h" #include "StreamCluster5LCacc.h" #include "StreamCluster6LCacc.h" #i

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `StreamCluster1LCacc.h`
- `StreamCluster3LCacc.h`
- `StreamCluster4LCacc.h`
- `StreamCluster5LCacc.h`
- `StreamCluster6LCacc.h`
- `TDLCA_StreamCluster.cpp`

## Accelerator Modules
StreamCluster1LCacc.h StreamCluster3LCacc.h StreamCluster4LCacc.h StreamCluster5LCacc.h StreamCluster6LCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`StreamClusterLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
