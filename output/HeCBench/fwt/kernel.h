#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// CUDA thread/block dimension constants (adjust for your workload)
#ifndef BLOCK_DIM_X
#define BLOCK_DIM_X 256
#endif
#ifndef GRID_DIM_X
#define GRID_DIM_X 1
#endif
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif

// --- from kernels.cu ---
/*
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *

// Single in-global memory radix-4 Fast Walsh Transform pass
// (for strides exceeding elementary vector size)

// Put everything together: batched Fast Walsh Transform CPU front-end

// Modulate two arrays

//Interface to modulateKernel()


// --- from main.cu ---
/*
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/*
 * Walsh transforms belong to a class of generalized Fourier transformations.
 * They have applications in various fields of electrical engineering
 * and numeric theory. In this sample we demonstrate efficient implementation
 * of naturally-ordered Walsh transform
 * (also known as Walsh-Hadamard or Hadamard transform) in CUDA and its
 * particular application to dyadic convolution computation.
 * Refer to excellent Jorg Arndt's "Algorithms for Programmers" textbook
 * http://www.jjj.de/fxt/fxtbook.pdf (Chapter 22)
 *
 * Victor Podlozhnyuk (vpodlozhnyuk@nvidia.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <chrono>

// Reference CPU FWT
extern"C" void fwtCPU(float *h_Output, float *h_Input, int log2N);
extern"C" void slowWTcpu(float *h_Output, float *h_Input, int log2N);
extern "C" void dyadicConvolutionCPU(
    float *h_Result,
    float *h_Data,
    float *h_Kernel,
    int log2dataN,
    int log2kernelN);

// GPU FWT

// Data configuration
const int log2Data = 23;
const int dataN = 1 << log2Data;
const int DATA_SIZE = dataN * sizeof(float);

const int log2Kernel = 7;
const int kernelN = 1 << log2Kernel;
const int KERNEL_SIZE = kernelN * sizeof(float);



// --- from reference.cu ---
/*
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *

// Straightforward Walsh Transform: used to test both CPU and GPU FWT
// Slow. Uses doubles because of straightforward accumulation
extern"C" void slowWTcpu(float *h_Output, float *h_Input, int log2N)
{
  const int N = 1 << log2N;

}

// Reference CPU dyadic convolution.
// Extremely slow because of non-linear memory access patterns (cache thrashing)
extern "C" void dyadicConvolutionCPU(
    float *h_Result,
    float *h_Data,
    float *h_Kernel,
    int log2dataN,
    int log2kernelN
    )
{
  const int   dataN = 1 << log2dataN;
  const int kernelN = 1 << log2kernelN;

}
