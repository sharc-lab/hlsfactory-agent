# LeNet HLS Design

## Overview
This is a Spiking Convolutional Neural Network (SCNN) implementation of the LeNet architecture for handwritten digit recognition on MNIST dataset.

## Architecture Details
- **Network Type**: LeNet-L (Large) with 4-bit weights
- **Dataset**: MNIST
- **Input Size**: 28x28 grayscale images
- **Architecture**:
  - Convolutional Layer 1 with 6 feature maps
  - Convolutional Layer 2 with 16 feature maps  
  - Fully Connected Layer 1
  - Output Layer (10 classes for digits 0-9)

## Top-Level Function
```c
void network(i_dt image_in[Sources], int intensity, int *dom, 
             w_dt k_1[MapsMax*MapsMax*KSide*KSide],
             w_dt k_2[MapsMax*MapsMax*KSide*KSide], 
             w_dt w_1[MaxRows*MaxCols],
             w_dt w_2[MaxRows*MaxCols], bool flag, 
             sc_dt spike_cnt_max[NumLayers], int vm_max[NumLayers],
             double spike_average[NumLayers], int spike_max[NumLayers])
```

## Files
- `src.cpp` - Main HLS source code with network implementation
- `tb.cpp` - Testbench (requires weight files for compilation)
- `includes/header.h` - Main header with function declarations
- `includes/mnist_2.h` - Network configuration parameters
- `includes/neuron_labels.h` - Output neuron labels
- `includes/sds_utils.h` - SDSoC utility functions

## HLS Pragmas Used
- `#pragma HLS PIPELINE` - Pipeline loops for throughput
- `#pragma HLS UNROLL` - Unroll loops for parallelism
- `#pragma HLS ARRAY_PARTITION` - Partition arrays for parallel access
- `#pragma HLS RESOURCE` - Specify memory resources (URAM)
- `#pragma HLS INLINE` - Function inlining control
- `#pragma HLS DEPENDENCE` - Specify data dependencies

## Key Parameters (from mnist_2.h)
- Sources: Input image size
- MapsMax: Maximum feature maps
- KSide: Kernel size (5x5)
- Side1/Side2: Feature map dimensions
- Output: Number of output classes (10)

## Compilation Status
- Source (src.cpp): SUCCESS
- Testbench (tb.cpp): FAILED (requires trained weight files)

## Notes
This design uses a Poisson spike generation mechanism for converting input images to spike trains.
The design supports both software simulation and hardware synthesis via SDSOC/Vitis HLS.
