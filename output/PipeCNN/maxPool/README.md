# maxPool

## Description
The maxPool kernel performs max pooling operations on feature maps. It supports different pool sizes (2x2 or 3x3) with configurable stride.

## Function
- Top-level function: `maxPool`
- Implements max pooling over a 2D window
- Supports pool sizes of 2 or 3
- Configurable stride for downsampling
- Uses shift registers for efficient window scanning
- Ping-pong buffering for parallel read/write operations

## Parameters
- conv_x, conv_xy: Input convolution dimensions
- pool_dim1, pool_dim3: Output pooling dimensions
- pool_size: Size of pooling window (2 or 3)
- pool_stride: Stride for pooling
- pool_group: Number of z-dimension vectorized packets

## Interfaces
- AXI-MM: bottom (input), top (output)

## Source Files
- maxPool.cpp
- hw_param.h
