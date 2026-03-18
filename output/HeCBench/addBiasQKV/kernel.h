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
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif

// --- from main.cu ---
/*
 * Copyright (c) 2019-2023, NVIDIA CORPORATION.  All rights reserved.
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

#include <chrono>
#include <cstdio>

// scale = AMAX(tensor) / FP8_MAX
// A (original) = A_scaled (fp8) * "scale of A"

template<typename T1, typename T2>
struct FP8TrtAddQKVBiasParam {
    T1*          qkv_tgt;
    const T1*    qkv_src;
    const T2*    qkv_bias;
    const float* input_scale;
    const float* output_scale;
    const int    valid_word_num;
    const int    head_num;
    const int    size_per_head;
    const int    hidden_unit;
};





template<typename T1, typename T2>

