# TDLCA_CBIR

## Description
#include "../../BenchmarkNode.h" #include "cnn_cfg.hpp" #include "MatMulLCacc.h" #include "MatMul400LCacc.h" #include "ReluLCacc.h" #include "PoolLCacc.h" #include "ManhattanDistLCacc.h" #include "Par

## Accelerator Design
This design implements a hardware accelerator using the PARADE LCAcc (Loop Control Accelerator) framework.

## Files
- `ManhattanDistLCacc.h`
- `MatMul400LCacc.h`
- `MatMulLCacc.h`
- `PartialSortLCacc.h`
- `PoolLCacc.h`
- `ReluLCacc.h`
- `TDLCA_CBIR.cpp`
- `cnn_cfg.hpp`

## Accelerator Modules
ManhattanDistLCacc.h MatMul400LCacc.h MatMulLCacc.h PartialSortLCacc.h PoolLCacc.h ReluLCacc.h 

## Interface
The accelerator uses the LCAcc framework with the following data types:
- Input/Output: Floating point arrays
- Control: Integer parameters for data sizing

## Top-Level Function
`CBIRLCacc`

## Synthesis Target
- Target Clock: 10ns (100MHz)
- Technology: Xilinx Virtex-7
- Interface: AXI Stream
