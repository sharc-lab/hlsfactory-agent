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
/*  Copyright (c) 2021-2022 Intel Corporation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <chrono>



template <typename scalar_t, typename gscalar_t>

template <>
void loss_bwd<__half, __half> (
    const __half*  log_softmax,
    const __half*  grad_output,
    const __half*  grad_output_neg,
    const int64_t*  target,
    const __half*  weight,
    const int64_t*  mask,
          __half*  grad_predict)
{
  int local_id_x = _tid_x;
  int group_id_bs = _bid_y;
  int group_id_x = _bid_x;

  int linear_x_id = group_id_x * threadX + local_id_x;

  if (linear_x_id >= H) return;

  int offset2d = group_id_bs * H + linear_x_id;
  int idx = target[offset2d];
  int sum_offset = group_id_bs * W * H + idx * H + linear_x_id;

  __half tmp_grad;
  if (mask[offset2d])
    tmp_grad = __hneg(__hadd(grad_output[offset2d] , grad_output_neg[offset2d]));
  else
    tmp_grad = __hneg(grad_output[offset2d]);

  tmp_grad = __hmul(tmp_grad , weight[offset2d]);

  float sum_value = h2f(__hmul(tmp_grad , log_softmax[sum_offset]));

  #pragma unroll
}

template <typename scalar_t, typename gscalar_t>

// compute cross entropy in the backward phase
template <typename scalar_t, typename gscalar_t>

