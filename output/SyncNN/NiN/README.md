# NiN (Network in Network) HLS Design

## Overview
This is a Spiking Convolutional Neural Network (SCNN) implementation of the Network in Network (NiN) architecture for CIFAR-10 image classification.

## Architecture Details
- **Network Type**: Network in Network (NiN)
- **Dataset**: CIFAR-10
- **Input Size**: 32x32 RGB images
- **Architecture**:
  - 9 Convolutional layers with MLP convolution (micro neural networks)
  - Global Average Pooling
  - Output Layer (10 classes)

## Top-Level Function
```c
void network(i_dt image_in[Sources], int intensity, int *dom,
             w_dt k_1[MapsMax*MapsMax*KSideMax*KSideMax],
             w_dt k_2[MapsMax*MapsMax*KSideMax*KSideMax], 
             w_dt k_3[MapsMax*MapsMax*KSideMax*KSideMax],
             w_dt k_4[MapsMax*MapsMax*KSideMax*KSideMax], 
             w_dt k_5[MapsMax*MapsMax*KSideMax*KSideMax],
             w_dt k_6[MapsMax*MapsMax*KSideMax*KSideMax], 
             w_dt k_7[MapsMax*MapsMax*KSideMax*KSideMax],
             w_dt k_8[MapsMax*MapsMax*KSideMax*KSideMax], 
             w_dt k_9[MapsMax*MapsMax*KSideMax*KSideMax],
             bool flag, sc_dt spike_cnt_max[NumLayers], int vm_max[NumLayers],
             double spike_average[NumLayers], int spike_max[NumLayers])
```

## Files
- `src.cpp` - Main HLS source code with network implementation
- `tb.cpp` - Testbench (requires weight files for compilation)
- `includes/header.h` - Main header with function declarations
- `includes/cifar10_1.h` - Network configuration parameters
- `includes/neuron_labels.h` - Output neuron labels
- `includes/sds_utils.h` - SDSoC utility functions

## HLS Pragmas Used
- `#pragma HLS PIPELINE` - Pipeline loops for throughput
- `#pragma HLS UNROLL` - Unroll loops for parallelism
- `#pragma HLS ARRAY_PARTITION` - Partition arrays for parallel access
- `#pragma HLS RESOURCE` - Specify memory resources (URAM)
- `#pragma HLS INLINE` - Function inlining control

## Key Parameters (from cifar10_1.h)
- Sources: Input image size (32x32x3 = 3072)
- MapsMax: Maximum feature maps
- KSideMax: Maximum kernel size
- NumLayers: Number of network layers
- Output: Number of output classes (10)

## Compilation Status
- Source (src.cpp): SUCCESS (with minor warning about double-to-int conversion)
- Testbench (tb.cpp): FAILED (requires trained weight files)

## Notes
This design implements the Network in Network architecture with MLP convolution layers.
Uses Poisson spike encoding for input stimulus generation.
