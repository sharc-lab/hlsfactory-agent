# eltwise

## Description
The eltwise kernel performs element-wise addition of two input feature maps with optional average pooling. Used for ResNet-style skip connections.

## Function
- Top-level function: `eltwise`
- Element-wise addition: top = bottom_1 + bottom_2
- Supports ReLU activation
- Optional average pooling for final layer
- Converts between fixed-point and floating-point

## Parameters
- input_num: Number of input elements
- pool_on: Pooling mode (0=none, 2=stride pool, 3=avgpool)
- conv_x, conv_xy: Convolution dimensions
- stride: Stride for stride pooling
- divisor: 1/pool_size^2 for avgpool
- in1_frac, in2_frac: Input conversion factors

## Interfaces
- AXI-MM: bottom_1, bottom_2 (inputs), top (output)

## Source Files
- eltwise.cpp
- hw_param.h
