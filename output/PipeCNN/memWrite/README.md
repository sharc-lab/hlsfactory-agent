# memWrite

## Description
The memWrite kernel writes output feature maps from the convolution pipeline back to global memory (DDR). It handles both regular outputs and pooling outputs.

## Function
- Top-level function: `memWrite`
- Receives data via HLS streams from previous kernels
- Performs address calculation for batched outputs
- Writes results back to DDR with proper vectorization
- Handles padding offsets for dimension alignment

## Parameters
- out_dim1, out_dim2, out_dim3: Output dimensions
- out_dim1xbatch, out_dim1x2xbatch: Batched addressing
- batch_indx_dim1, batch_indx_dim2: Batch indices
- padd_offset: Padding offset for alignment
- pool_on: Enable pooling mode addressing

## Interfaces
- AXI-MM: top (output)
- AXI-Stream: conv_in (or bn_in, bypass_in for ResNet)

## Source Files
- memWrite.cpp
- hw_param.h
