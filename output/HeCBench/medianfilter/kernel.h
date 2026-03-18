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

// --- from MedianFilter.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *


// --- from main.cu ---
/* * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.  * * Please refer to the NVIDIA end user license agreement (EULA) associated * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include <chrono>
#include "shrUtils.h"

#ifndef min
#define min(a,b) (a < b ? a : b)
#endif

// Import host computation function 
extern "C" void MedianFilterHost(unsigned int* uiInputImage, unsigned int* uiOutputImage, 
                                 unsigned int uiWidth, unsigned int uiHeight);

double MedianFilterGPU(
    unsigned int* uiInputImage, 
    unsigned int* uiOutputImage, 
    uchar4* cmDevBufIn,
    unsigned int* cmDevBufOut,
    const int uiImageWidth,
    const int uiImageHeight);


// Copies input data from host buf to the device, runs kernel, 
// copies output data back to output host buf
