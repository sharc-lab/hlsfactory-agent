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

// --- from main.cu ---
/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include <chrono>
#include <memory>
#include <iostream>
#include <hip/hip_runtime.h>
#include "shrUtils.h"

extern void BoxFilterHost(unsigned int* uiInputImage, unsigned int* uiTempImage, unsigned int* uiOutputImage,
                          int uiWidth, int uiHeight, int iRadius, float fScale );

const unsigned int RADIUS = 10;                   // initial radius of 2D box filter mask
const float SCALE = 1.0f/(2.0f * RADIUS + 1.0f);  // precalculated GV rescaling value


// Helper function to convert float[4] rgba color to 32-bit unsigned integer

// Inline device function to convert floating point rgba color to 32-bit unsigned integer




