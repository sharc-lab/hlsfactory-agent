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

#define CLAMP_TO_EDGE 
#define MAC

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <iostream>
#include <cassert>
#include <chrono>
#include <hip/hip_runtime.h>
#include "main.h"
#include "shrUtils.h"

// Inline device function to convert 32-bit unsigned integer to floating point rgba color 
//*****************************************************************

// Inline device function to convert floating point rgba color to 32-bit unsigned integer
//*****************************************************************

// Transpose kernel (see transpose SDK sample for details)
//*****************************************************************

// 	simple 1st order recursive filter kernel 
//*****************************************************************
//    - processes one image column per thread
//      parameters:	
//      uiDataIn - pointer to input data (RGBA image packed into 32-bit integers)
//      uiDataOut - pointer to output data 
//      iWidth  - image width
//      iHeight  - image height
//      a  - blur parameter
//*****************************************************************

// Recursive Gaussian filter 
//*****************************************************************
//  parameters:	
//      uiDataIn - pointer to input data (RGBA image packed into 32-bit integers)
//      uiDataOut - pointer to output data 
//      iWidth  - image width
//      iHeight  - image height
//      a0-a3, b1, b2, coefp, coefn - filter parameters
//
//      If used, CLAMP_TO_EDGE is passed in via OpenCL clBuildProgram call options string at app runtime
//*****************************************************************


