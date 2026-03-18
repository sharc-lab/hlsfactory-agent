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
 * Copyright (c) 2019-2021, NVIDIA CORPORATION.  All rights reserved.
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

#include <algorithm>
#include <chrono>
#include <random>
#include <hip/hip_runtime.h>
#include <hip/hip_fp16.h>
#include <hip/hip_bf16.h>

template<typename T, int V, int WarpSize>

template<typename T, int V>



// --- from kernels.h ---
/*
 * Copyright (c) 2019-2021, NVIDIA CORPORATION.  All rights reserved.
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

template<typename T>
inline float typeToFloat(T a);

template<>
inline float typeToFloat(__hip_bfloat16 a) {
  return __bfloat162float(a);
}

template<>
inline float typeToFloat(half a) {
  return __half2float(a);
}

template<typename T>
inline T floatToType(float a);

template<>
inline half floatToType(float a) {
  return __float2half_rn(a);
}

template<>
inline __hip_bfloat16 floatToType(float a) {
  return __float2bfloat16(a);
}

template<typename T>
inline T floatToType2(float a);

// Converts input to half precision in round-to-nearest-even mode
// and populates both halves of half2 with converted value.
template<>
inline half2 floatToType2(float a) {
  return __float2half2_rn(a);
}

template<>
inline __hip_bfloat162 floatToType2(float a) {
  //return __float2bfloat162_rn(a);
  float2 t = float2(a, a);
  return __float22bfloat162_rn(t);
}

template<typename T>
inline T float2ToType2(float2 a) {
  return a;
}

// Converts both components of float2 to half precision in round-to-nearest mode
// and combines the results into one half2 number.
template<>
inline half2 float2ToType2(float2 a) {
  return __float22half2_rn(a);
}

template<>
inline __hip_bfloat162 float2ToType2(float2 a) {
  return __float22bfloat162_rn(a);
}

template<typename T>
inline float2 type2ToFloat2(T a) {
  return a;
}

template<>
inline float2 type2ToFloat2(half2 a) {
  return __half22float2(a);
}

template<>
inline float2 type2ToFloat2(__hip_bfloat162 a) {
  return __bfloat1622float2(a);
}

// Convert between a vector type and a scalar type
template<typename T>
struct TypeConverter {using Type = half2;}; // keep for generality

template<>
struct TypeConverter<__hip_bfloat16> {using Type = __hip_bfloat162;};

// general add
template<typename T>
inline T add(T a, T b) {
  return a + b;
}

template<>
inline half add(half a, half b) {
  return __hadd(a, b);
}

template<>
inline half2 add(half2 a, half2 b) {
  return __hadd2(a, b);
}

template<>
inline __hip_bfloat16 add(__hip_bfloat16 a, __hip_bfloat16 b) {
  return __hadd(a, b);
}

template<>
inline __hip_bfloat162 add(__hip_bfloat162 a, __hip_bfloat162 b) {
  return __hadd2(a, b);
}

template<typename T>
inline T add(T a, T b, T c) {
  return a + b + c;
}

template<>
inline __hip_bfloat162 add(__hip_bfloat162 a, __hip_bfloat162 b, __hip_bfloat162 c) {
  return add(add(a, b), c);
}

template<>
inline __hip_bfloat16 add(__hip_bfloat16 a, __hip_bfloat16 b, __hip_bfloat16 c) {
  return add(add(a, b), c);
}

// general sub
template<typename T>
inline T sub(T a, T b) {
  return a - b;
}

template<>
inline half2 sub(half2 a, half2 b) {
  return __hsub2(a, b);
}

template<>
inline __hip_bfloat162 sub(__hip_bfloat162 a, __hip_bfloat162 b) {
  return __hsub2(a, b);
}

// general fma
template<typename T>
inline T fma(T a, T b, T c, T d) {
    return a * b * c + d;
}

template<>
inline half2 fma(half2 a, half2 b, half2 c, half2 d) {
    return __hadd2(__hmul2(__hmul2(a, b), c), d);
}

template<>
inline __hip_bfloat162 fma(__hip_bfloat162 a, __hip_bfloat162 b, __hip_bfloat162 c, __hip_bfloat162 d) {
    return __hadd2(__hmul2(__hmul2(a, b), c), d);
}

// reduction 
template<typename T, int WarpSize>
__inline__ T warpReduceSum(T val)
{
#pragma unroll
    for (int mask = WarpSize/2; mask > 0; mask >>= 1)
        val = add(val, __shfl_xor(val, mask, WarpSize));
    return val;
}

/* Calculate the sum of all elements in a block */
template<typename T, int WarpSize>
__inline__ T blockReduceSum(T val)
{
    static T shared[1024/WarpSize];
    int lane = _tid_x % WarpSize;
    int wid = _tid_x / WarpSize;

    val = warpReduceSum<T, WarpSize>(val);

    if (lane == 0)
        shared[wid] = val;

    val = (_tid_x < (BLOCK_DIM_X / WarpSize)) ? shared[lane] : (T)(0.0f);
    val = warpReduceSum<T, WarpSize>(val);

    return val;
}

template<typename T, int WarpSize>
void addBiasResidualPostLayerNormV2(
          T* out,
    const T*  input,
    const T*  bias,
    const T*  gamma,
    const T*  beta,
    const float layernorm_eps,
    const int n)
{
  using T2             = typename TypeConverter<T>::Type;
  const int        ite = 4;
  const int        tid = _tid_x;
  const int        bid = _bid_x;
  float s_mean;
  float s_variance;
  float            mean     = 0.0f;
  float            variance = 0.0f;
  T2               local_out_half2[ite];

  T2*       out_ptr   = (T2*)out;
  const T2* input_ptr = (const T2*)input;
  const T2* bias_ptr  = (const T2*)bias;
  const T2* gamma_ptr = (const T2*)gamma;
  const T2* beta_ptr  = (const T2*)beta;

  T2 sum = floatToType2<T2>(0.0f);

  // ite = 4 and BLOCK_DIM_X = n / 8
  // When n = 1024, BLOCK_DIM_X = 128
  // col_id range: [0-127], [128-255], [256-383], [384-511]
  // block stride = n / 2 = 512
#pragma unroll
  for (int i = 0; i < ite; i++) {
    int col_id         = i * BLOCK_DIM_X + tid;
    int id             = bid * n / 2 + col_id;
    local_out_half2[i] = add(out_ptr[id], input_ptr[id], bias_ptr[col_id]);
    sum                = add(sum, local_out_half2[i]);
  }

  mean = blockReduceSum<float, WarpSize>(typeToFloat(__hadd(sum.x , sum.y)));
  if (_tid_x == 0) {
    s_mean = mean / n;
  }

  float var      = 0.0f;
  T2    s_mean_2 = floatToType2<T2>(s_mean);

#pragma unroll
  for (int i = 0; i < ite; i++) {
    local_out_half2[i] = sub(local_out_half2[i], s_mean_2);
    float v1           = typeToFloat(local_out_half2[i].x);
    float v2           = typeToFloat(local_out_half2[i].y);
    var += v1 * v1 + v2 * v2;
  }

  variance = blockReduceSum<float, WarpSize>(var);
  if (tid == 0) {
    s_variance = rsqrtf(variance / n + layernorm_eps);
  }

  T2 s_var_2 = floatToType2<T2>(s_variance);
#pragma unroll
  for (int i = 0; i < ite; i++) {
    int col_id  = i * BLOCK_DIM_X + tid;
    int id      = bid * n / 2 + col_id;
    out_ptr[id] = fma(local_out_half2[i], s_var_2,
                      gamma_ptr[col_id], beta_ptr[col_id]);
  }
}

template<typename T, int N, int WarpSize>
void addBiasResidualPostLayerNorm(
          T* out, 
    const T*  input,
    const T*  bias,
    const T*  gamma,
    const T*  beta,
    const float layernorm_eps,
    const int n)
{
  float s_mean;
  float s_variance;
  float            mean     = 0.0f;
  float            variance = 0.0f;
  float            local_out_cache[N];

#pragma unroll N
  for (int idx = _tid_x, i = 0; idx < n && i < N; ++i) {
    float local_out = typeToFloat(add(out[_bid_x * n + idx], input[_bid_x * n + idx], bias[idx]));
    mean += local_out;
    // save local_out to local_out_cache to save some recompute
    local_out_cache[i] = local_out;
    idx += BLOCK_DIM_X;
  }

  mean = blockReduceSum<float, WarpSize>(mean);
  if (_tid_x == 0) {
    s_mean = mean / n;
  }

#pragma unroll N
  for (int idx = _tid_x, i = 0; idx < n && i < N; ++i) {
    float local_out = local_out_cache[i];
    variance += (local_out - s_mean) * (local_out - s_mean);
    idx += BLOCK_DIM_X;
  }
  variance = blockReduceSum<float, WarpSize>(variance);
  if (_tid_x == 0) {
    s_variance = variance / n + layernorm_eps;
  }

#pragma unroll N
  for (int idx = _tid_x, i = 0; idx < n && i < N; ++i) {
    float local_out = local_out_cache[i];
    out[_bid_x * n + idx] =
      floatToType<T>(((local_out - s_mean) * rsqrtf(s_variance)) * typeToFloat(gamma[idx]) + typeToFloat(beta[idx]));
    idx += BLOCK_DIM_X;
  }
}

template<typename T, int WarpSize>
void generalAddBiasResidualPostLayerNorm(
          T* out, 
    const T*  input,
    const T*  bias,
    const T*  gamma,
    const T*  beta,
    const float layernorm_eps,
    const int n)
{
  using T2 = typename TypeConverter<T>::Type;
  float s_mean;
  float s_variance;
  float            mean     = 0.0f;
  float            variance = 0.0f;

  T2*       out_ptr   = (T2*)out;
  const T2* input_ptr = (const T2*)input;
  const T2* bias_ptr  = (const T2*)bias;
  const T2* gamma_ptr = (const T2*)gamma;
  const T2* beta_ptr  = (const T2*)beta;

  float local_out = 0.0f;
  for (int idx = _tid_x; idx < n / 2; idx += BLOCK_DIM_X) {
    int    id            = _bid_x * n / 2 + idx;
    T2     tmp           = add(add(out_ptr[id], input_ptr[id]), bias_ptr[idx]);
    float2 local_out_fp2 = type2ToFloat2(tmp);
    local_out += local_out_fp2.x;
    local_out += local_out_fp2.y;
    // save tmp to out_ptr to save some recomputation
    out_ptr[id] = tmp;
  }

  mean = blockReduceSum<float, WarpSize>(local_out);
  if (_tid_x == 0) {
    s_mean = mean / n;
  }

  for (int idx = _tid_x; idx < n / 2; idx += BLOCK_DIM_X) {
    int    id            = _bid_x * n / 2 + idx;
    float2 local_out_fp2 = type2ToFloat2(out_ptr[id]);
    variance += (local_out_fp2.x - s_mean) * (local_out_fp2.x - s_mean);
    variance += (local_out_fp2.y - s_mean) * (local_out_fp2.y - s_mean);
  }

  variance = blockReduceSum<float, WarpSize>(variance);
  if (_tid_x == 0) {
    s_variance = rsqrtf(variance / n + layernorm_eps);
  }

  for (int idx = _tid_x; idx < n / 2; idx += BLOCK_DIM_X) {
    int    id            = _bid_x * n / 2 + idx;
    float2 local_out_fp2 = type2ToFloat2(out_ptr[id]);
    float2 gamma_val     = type2ToFloat2(gamma_ptr[idx]);
    float2 beta_val      = type2ToFloat2(beta_ptr[idx]);
    local_out_fp2.x      = (local_out_fp2.x - s_mean) * s_variance * gamma_val.x + beta_val.x;
    local_out_fp2.y      = (local_out_fp2.y - s_mean) * s_variance * gamma_val.y + beta_val.y;
    out_ptr[id]          = float2ToType2<T2>(local_out_fp2);
  }
}
