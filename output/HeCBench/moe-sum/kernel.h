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
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>

#define GPU_CHECK(ans)                                                                   \
    {                                                                                    \
}

template <typename scalar_t>




// --- from kernels.h ---

template <typename scalar_t, int TOPK>
void moe_sum_kernel(
    scalar_t*  out,          // [..., d]
    const scalar_t*  input,  // [..., topk, d]
    const int d)
{
  const int64_t output_base = _bid_x * d; 
  const int64_t input_base = output_base * TOPK;
  for (int64_t idx = _tid_x; idx < d; idx += BLOCK_DIM_X) {
    scalar_t x = 0.0;
    #pragma unroll
    for (int k = 0; k < TOPK; ++k) {
      x += __ldg(&input[input_base + k * d + idx]);
    }
    out[output_base + idx] = x;
  }
}

template <int TOPK>
void moe_sum_kernel_vec4(
    float*  out,          // [..., d]
    const float*  input,  // [..., topk, d]
    int d)
{
  int d4 = d >> 2; // divisible by 4
  int output_base4 = (_bid_x * d) >> 2;
  int input_base4  = (_bid_x * d * TOPK) >> 2;

  const float4* input4 = reinterpret_cast<const float4*>(input);
  float4* out4 = reinterpret_cast<float4*>(out);

  for (int idx = _tid_x; idx < d4; idx += BLOCK_DIM_X) {
    float4 acc = float4(0.f, 0.f, 0.f, 0.f);

    #pragma unroll
    for (int k = 0; k < TOPK; ++k) {
      float4 v = input4[input_base4 + k * d4 + idx];
      acc.x += v.x;
      acc.y += v.y;
      acc.z += v.z;
      acc.w += v.w;
    }
    out4[output_base4 + idx] = acc;
  }
}
