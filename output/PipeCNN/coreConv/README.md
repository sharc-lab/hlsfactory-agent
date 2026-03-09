# coreConv

## Description
The coreConv kernel performs the main convolution operation for CNN layers. It implements a vectorized MAC (Multiply-Accumulate) computation with multiple lanes.

## Function
- Top-level function: `coreConv`
- Performs dot product between input data and weights
- Supports multiple parallel lanes (LANE_NUM) for throughput
- Implements pipelined accumulation with configurable PIPE_DEPTH
- Performs quantization and rounding to fixed-point output

## Key Features
- Vectorized MAC units with configurable VEC_SIZE
- Deep pipelined accumulation registers
- Fixed-point arithmetic with configurable precision
- Optional ReLU activation (controlled by 'contol' parameter)

## Parameters
- output_num: Number of output pixels to generate
- conv_loop_cnt: Convolution inner loop count
- contol: Control flags for ReLU and bypass
- frac_w, frac_din, frac_dout: Fixed-point fraction bits

## Interfaces
- AXI-Stream: bias_in, weight_in, data_in, conv_out

## Source Files
- coreConv.cpp
- hw_param.h
