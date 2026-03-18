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

// --- from kernels.cu ---








































void (*coding_func_array[])(int k, int index,
    char *dataPtr, char *codeDevPtr,
    const unsigned int *bitMatrixPtr, 
    int threadDimX,int blockDimX,
    int workSizePerGridInLong) = {
  m_1_w_4_coding,m_1_w_5_coding,m_1_w_6_coding,m_1_w_7_coding,m_1_w_8_coding,
  m_2_w_4_coding,m_2_w_5_coding,m_2_w_6_coding,m_2_w_7_coding,m_2_w_8_coding,
  m_3_w_4_coding,m_3_w_5_coding,m_3_w_6_coding,m_3_w_7_coding,m_3_w_8_coding,
  m_4_w_4_coding,m_4_w_5_coding,m_4_w_6_coding,m_4_w_7_coding,m_4_w_8_coding
};



// --- from main.cu ---
//  Created by Liu Chengjian on 17/10/9.
//  Copyright (c) 2017 csliu. All rights reserved.
//

#include "GCRSMatrix.h"


// --- from utils.cu ---





// --- from utils.h ---
#ifndef __UTILS
#define __UTILS

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <hip/hip_runtime.h>

size_t align_value(size_t valueToAlign, size_t alignMask);

void generateRandomValue(char *data, size_t size);

double elapsed_time_in_ms(struct timeval startTime, struct timeval endTime);
 
typedef void (*coding_func)(int k, int index,
    char *dataPtr, char *codeDevPtr,
    const unsigned int *bitMatrixPtr,
    int threadDimX,int blockDimX,
    int workSizePerGridInLong);

#define talloc(type, num) (type *)malloc(sizeof(type) * (num))

#define MIN_K 1
#define MAX_K 4

#define MIN_M 1
#define MAX_M 4

#define MIN_W 4
#define MAX_W 8

#define MAX_K_MULIPLY_M (MAX_K * MAX_M)

#define WARM_UP_SIZE 4

#define MAX_THREAD_NUM 128
#define THREADS_PER_BLOCK 128
#define WARP_SIZE 32

#endif

