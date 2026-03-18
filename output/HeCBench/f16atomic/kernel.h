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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>

#define CHECK_ERROR( err ) (CheckError( err, __FILE__, __LINE__ ))

#define BLOCK_SIZE 256

#define ZERO_FP16 __ushort_as_half((unsigned short)0x0000U)
#define ONE_FP16  __ushort_as_half((unsigned short)0x3c00U)
#define ZERO_BF16 __ushort_as_bfloat16((unsigned short)0x0000U)
#define ONE_BF16  __ushort_as_bfloat16((unsigned short)0x3f80U)



template <typename T>

