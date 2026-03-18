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
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <chrono>
#include <random>
#include <hip/hip_fp16.h>
#include <hip/hip_runtime.h>
#include "reference.h"

template<typename T>




// --- from reduce.h ---

template<typename T>
__inline__ T warpReduceSum(T val)
{
#pragma unroll
  #ifdef WAVE64
  for (int mask = 32; mask > 0; mask >>= 1)
    val += __shfl_xor(val, mask);
  #else
  for (int mask = 16; mask > 0; mask >>= 1)
    val += __shfl_xor(val, mask, 32);
  #endif
  return val;
}

/* Calculate the sum of all elements in a block */
template<typename T>
__inline__ T blockReduceSum(T val)
{
  #ifdef WAVE64
  static T shared[64];
  int lane = _tid_x & 0x3f;
  int wid = _tid_x >> 6;
  #else
  static T shared[32];
  int lane = _tid_x & 0x1f;
  int wid = _tid_x >> 5;
  #endif

  val = warpReduceSum<T>(val);

  if (lane == 0)
    shared[wid] = val;

  // Modify from BLOCK_DIM_X << 5 to BLOCK_DIM_X / 32. to prevent
  // BLOCK_DIM_X is not divided by 32
  #ifdef WAVE64
  val = (_tid_x < (BLOCK_DIM_X / 64.f)) ? shared[lane] : (T)(0.0f);
  #else
  val = (_tid_x < (BLOCK_DIM_X / 32.f)) ? shared[lane] : (T)(0.0f);
  #endif
  val = warpReduceSum<T>(val);

  return val;
}

template<typename T>
__inline__ T warpReduceMax(T val)
{
  #pragma unroll
  #ifdef WAVE64
  for (int mask = 32; mask > 0; mask >>= 1)
    val = max(val, __shfl_xor(val, mask));
  #else
  for (int mask = 16; mask > 0; mask >>= 1)
    val = max(val, __shfl_xor(val, mask, 32));
  #endif
  return val;
}

/* Calculate the maximum of all elements in a block */
template<typename T>
__inline__ T blockReduceMax(T val)
{
  #ifdef WAVE64
  static T shared[64];
  int lane = _tid_x & 0x3f;  // in-warp idx
  int wid = _tid_x >> 6;     // warp idx
  #else
  static T shared[32];
  int lane = _tid_x & 0x1f;  // in-warp idx
  int wid = _tid_x >> 5;     // warp idx
  #endif

  val = warpReduceMax(val);  // get maxx in each warp

  if (lane == 0)  // record in-warp maxx by warp Idx
    shared[wid] = val;

  // Modify from BLOCK_DIM_X << 5 to BLOCK_DIM_X / 32. to prevent
  // BLOCK_DIM_X is not divided by 32
  #ifdef WAVE64
  val = (_tid_x < (BLOCK_DIM_X / 64.f)) ? shared[lane] : -1e20f;
  #else
  val = (_tid_x < (BLOCK_DIM_X / 32.f)) ? shared[lane] : -1e20f;
  #endif
  val = warpReduceMax(val);

  return val;
}
