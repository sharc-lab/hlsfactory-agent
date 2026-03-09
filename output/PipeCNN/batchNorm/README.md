# batchNorm

## Description
The batchNorm kernel performs batch normalization operations on convolution outputs. It normalizes the input using pre-computed mean, variance, alpha, and beta parameters.

## Function
- Top-level function: `batchNorm`
- Implements: output = ((input - mean) / sqrt(variance)) * alpha + beta
- Converts fixed-point input to float for computation
- Converts result back to fixed-point with rounding
- Optional ReLU activation

## Parameters
- dim1xdim2: Feature map dimensions
- input_num: Total number of input elements
- contol: Control flags for ReLU
- frac2float, frac2char: Conversion factors

## Interfaces
- AXI-MM: mean, var, alpha, beta (normalization parameters)
- AXI-Stream: conv_in, bn_out

## Source Files
- batchNorm.cpp
- hw_param.h
