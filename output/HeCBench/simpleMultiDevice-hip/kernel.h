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
 * This application demonstrates how to use the HIP API to use multiple GPUs
 */

#include <math.h>
#include <stdio.h>
#include <chrono>
#include <hip/hip_runtime.h>

#ifndef MIN
#define MIN(a,b) (a < b ? a : b)
#endif


// Data configuration
#ifndef MAX_GPU_COUNT 
  #define MAX_GPU_COUNT 8
#endif

const int DATA_N = 1048576 * 32;

// Simple reduction kernel.
// Refer to the 'reduction' CUDA Sample describing
// reduction optimization strategies

// Program main


// --- from simpleMultiDevice.h ---
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
 * This application demonstrates how to use the CUDA API to use multiple GPUs.
 *
 * Note that in order to detect multiple GPUs in your system you have to disable
 * SLI in the nvidia control panel. Otherwise only one GPU is visible to the
 * application. On the other side, you can still extend your desktop to screens
 * attached to both GPUs.
 */

#ifndef SIMPLEMULTIGPU_H
#define SIMPLEMULTIGPU_H

typedef struct
{
    //Host-side input data
    int dataN;
    float *h_Data;

    //Partial sum for this GPU
    float *h_Sum;

    //Device buffers
    float *d_Data,*d_Sum;

    //Reduction copied back from GPU
    float *h_Sum_from_device;

    //Stream for asynchronous command execution
    hipStream_t stream;

} TGPUplan;

#endif
