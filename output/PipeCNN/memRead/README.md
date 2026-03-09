# memRead

## Description
The memRead kernel is responsible for reading input feature maps, weights, and biases from global memory and feeding them to the coreConv kernel through HLS streams.

## Function
- Top-level function: `memRead`
- Reads input data from DDR memory
- Performs data padding when necessary
- Implements ping-pong buffering for efficient data reuse
- Streams data to the convolution kernel via FIFOs

## Parameters
- data_dim1, data_dim2: Input feature map dimensions
- weight_dim1, weight_dim2, weight_dim3: Weight dimensions
- conv_x, stride, padding: Convolution parameters
- split, group_num_x, group_num_y: Group processing parameters

## Interfaces
- AXI-MM: bottom (input feature maps), weights, bias
- AXI-Stream: bias_out, weight_out, data_out

## Source Files
- memRead.cpp
- hw_param.h
