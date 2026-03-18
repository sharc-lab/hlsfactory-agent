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
 * Adapted from
 * https://github.com/NVIDIA/FasterTransformer/blob/release/v5.3_tag/src/fastertransformer/kernels/decoder_masked_multihead_attention/decoder_masked_multihead_attention_template.hpp
 * https://github.com/vllm-project/vllm/blob/main/benchmarks/kernels/benchmark_paged_attention.py
 *
 *
 * Copyright (c) 2023, The vLLM team.
 * Copyright (c) 2020-2023, NVIDIA CORPORATION.  All rights reserved.
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
#include "attention_kernels.cuh"

#define LAUNCH_PAGED_ATTENTION_V1(HEAD_SIZE)                          \
  VLLM_DevFuncAttribute_SET_MaxDynamicSharedMemorySize(               \
      ((void*)paged_attention_v1_kernel<T, CACHE_T, HEAD_SIZE,        \
                                              BLOCK_SIZE, NUM_THREADS,\
                                              IS_BLOCK_SPARSE>),      \
      shared_mem_size);                                               \
  paged_attention_v1_kernel<T, CACHE_T, HEAD_SIZE, BLOCK_SIZE,        \
                                  NUM_THREADS, IS_BLOCK_SPARSE>       \

          out, query, key_cache, value_cache, num_kv_heads,           \
          scale, block_tables, seq_lens, max_num_blocks_per_seq,      \
          alibi_slopes, q_stride, kv_block_stride, kv_head_stride,    \
          tp_rank, blocksparse_local_blocks,                          \
          blocksparse_vert_stride, blocksparse_block_size,            \
          blocksparse_head_sliding_step);

// TODO(woosuk): Tune NUM_THREADS.
template <typename T, typename CACHE_T, int BLOCK_SIZE,
          bool IS_BLOCK_SPARSE, int NUM_THREADS = 128>
void paged_attention_v1_launcher(
    T *out,
    T *query,
    CACHE_T* key_cache,
    CACHE_T* value_cache,
    int num_kv_heads,
    float scale,
    int *block_tables,
    int *seq_lens,
    int max_seq_len,
    float *alibi_slopes,
    const int tp_rank,
    const int blocksparse_local_blocks, const int blocksparse_vert_stride,
    const int blocksparse_block_size, const int blocksparse_head_sliding_step,
    const int num_seqs,
    const int num_heads,
    const int head_size,
    const int max_num_blocks_per_seq,
    const int q_stride,
    const int kv_block_stride,
    const int kv_head_stride)
{
  const int NUM_WARPS = NUM_THREADS / WARP_SIZE;
  int padded_max_seq_len = (max_seq_len + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE;
  int logits_size = padded_max_seq_len * sizeof(float);
  int outputs_size = (NUM_WARPS / 2) * head_size * sizeof(float);
  int shared_mem_size = std::max(logits_size, outputs_size);

  dim3 grid(num_heads, num_seqs, 1);
}

template <typename T, int block_size = 16> 



// --- from attention_dtypes.h ---
#pragma once

#include "attention_generic.cuh"
#include "dtype_float32.cuh"
#include "dtype_bfloat16.cuh"


// --- from cuda_compat.h ---
#pragma once

#ifdef USE_ROCM

#include <hip/hip_runtime.h>
#define GPU_CHECK(x) do { \
    hipError_t err = x; \
    if (err != hipSuccess) { \
        printf("HIP error %s:%d: %s\n", \
               __FILE__, __LINE__, hipGetErrorString(err)); \
    } \
} while (0)

#else

#define GPU_CHECK(x) do { \
    cudaError_t err = x; \
    if (err != cudaSuccess) { \
        printf("CUDA error %s:%d: %s\n", \
               __FILE__, __LINE__, cudaGetErrorString(err)); \
    } \
} while (0)

#endif

#ifdef USE_ROCM
struct Utils {
  static int get_warp_size() {
    static bool is_cached = false;
    static int result;

    if (!is_cached) {
      int warp_size;
      GPU_CHECK(hipDeviceGetAttribute(&warp_size, hipDeviceAttributeWarpSize, 0));
      result = warp_size;
      is_cached = true;
    }

    return result;
  }

  static constexpr int get_warp_size() {
  #ifdef __GFX9__
    return 64;
  #else
    return 32;
  #endif
  }
};

  #define WARP_SIZE Utils::get_warp_size()
#else
  #define WARP_SIZE 32
#endif

#ifndef USE_ROCM
  #define VLLM_LDG(arg) __ldg(arg)
#else
  #define VLLM_LDG(arg) *(arg)
#endif

#ifndef USE_ROCM
  #define VLLM_SHFL_XOR_SYNC(var, lane_mask) \
    0, var, lane_mask)
  #define VLLM_SHFL_XOR_SYNC_WIDTH(var, lane_mask, width) \
    0, var, lane_mask, width)
#else
  #define VLLM_SHFL_XOR_SYNC(var, lane_mask) __shfl_xor(var, lane_mask)
  #define VLLM_SHFL_XOR_SYNC_WIDTH(var, lane_mask, width) \
    __shfl_xor(var, lane_mask, width)
#endif

#ifndef USE_ROCM
  #define VLLM_SHFL_SYNC(var, src_lane) 0, var, src_lane)
#else
  #define VLLM_SHFL_SYNC(var, src_lane) __shfl(var, src_lane)
#endif

#ifndef USE_ROCM
  #define VLLM_SHFL_DOWN_SYNC(var, lane_delta) \
    0, var, lane_delta)
#else
  #define VLLM_SHFL_DOWN_SYNC(var, lane_delta) __shfl_down(var, lane_delta)
#endif

#ifndef USE_ROCM
  #define VLLM_DevFuncAttribute_SET_MaxDynamicSharedMemorySize(FUNC, VAL) \
    GPU_CHECK(cudaFuncSetAttribute(FUNC, cudaFuncAttributeMaxDynamicSharedMemorySize, VAL))
#else
  #define VLLM_DevFuncAttribute_SET_MaxDynamicSharedMemorySize(FUNC, VAL) \
    GPU_CHECK(hipFuncSetAttribute(FUNC, hipFuncAttributeMaxDynamicSharedMemorySize, VAL))
#endif


// --- from kvcache.h ---
/*
def create_kv_caches_with_random(
    num_blocks: int,
    block_size: int,
    num_layers: int,
    num_heads: int,
    head_size: int,
    cache_dtype: str | torch.dtype | None,
    model_dtype: str | torch.dtype | None = None,
    seed: int | None = None,
    device: str | None = "cuda",
) -> tuple[list[torch.Tensor], list[torch.Tensor]]:
    if cache_dtype == "fp8" and head_size % 16:
        raise ValueError(
            f"Does not support key cache of type fp8 with head_size {head_size}"
        )

    set_random_seed(seed)

    dtype = get_kv_cache_torch_dtype(cache_dtype, model_dtype)

    scale = head_size**-0.5
    x = 16 // torch.tensor([], dtype=dtype).element_size()
    key_cache_shape = (num_blocks, num_heads, head_size // x, block_size, x)
    key_caches: list[torch.Tensor] = []
    for _ in range(num_layers):
        key_cache = torch.empty(size=key_cache_shape, dtype=dtype, device=device)
        if cache_dtype in ["auto", "half", "bfloat16", "float"]:
            key_cache.uniform_(-scale, scale)
        elif cache_dtype == "fp8":
            _generate_random_fp8(key_cache, -scale, scale)
        else:
            raise ValueError(f"Does not support key cache of type {cache_dtype}")
        key_caches.append(key_cache)

    value_cache_shape = (num_blocks, num_heads, head_size, block_size)
    value_caches: list[torch.Tensor] = []
    for _ in range(num_layers):
        value_cache = torch.empty(size=value_cache_shape, dtype=dtype, device=device)
        if cache_dtype in ["auto", "half", "bfloat16", "float"]:
            value_cache.uniform_(-scale, scale)
        elif cache_dtype == "fp8":
            _generate_random_fp8(value_cache, -scale, scale)
        else:
            raise ValueError(f"Does not support value cache of type {cache_dtype}")
        value_caches.append(value_cache)
    return key_caches, value_caches
struct KVCaches {
    void** key_caches;    // array of device pointers
    void** value_caches;
};
*/

inline uint32_t xorshift32(uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

inline float uint32_to_uniform(uint32_t x) {
    // Convert to (0,1)
    return (x >> 8) * 0x1.0p-24f;
}

inline float normal_from_uniform( uint32_t& rng) {
    float u1 = uint32_to_uniform(xorshift32(rng));
    float u2 = uint32_to_uniform(xorshift32(rng));

    // Box–Muller
    float r = sqrtf(-2.0f * logf(u1));
    float theta = 6.28318530718f * u2;
    return r * cosf(theta);           // N(0,1)
}

template <typename T>
void uniform_fill_kernel(
    T* data,
    int64_t n,
    float low,
    float high,
    uint32_t seed
) {
    int64_t idx = _bid_x * BLOCK_DIM_X + _tid_x;
    if (idx >= n) return;

    uint32_t rng = seed ^ idx;

    uint32_t r = xorshift32(rng);
    float u = uint32_to_uniform(r);
    float v = low + (high - low) * u;
    data[idx] = (T)v;
}

template <typename T>
void norm_fill_kernel(
    T* data,
    int n,
    uint32_t seed
) {
    int idx = _bid_x * BLOCK_DIM_X + _tid_x;
    if (idx >= n) return;

    uint32_t rng = seed ^ idx;
    data[idx] = (T)normal_from_uniform(rng);
}

template<typename T>
struct KVCaches {
    T** key_caches;
    T** value_caches;
};

template <typename T>
KVCaches<T> create_kv_caches_with_random(
    int num_blocks,
    int block_size,
    int num_layers,
    int num_heads,
    int head_size,
    unsigned long seed,
    int &kv_block_stride,
    int &kv_head_stride
) {
    float scale = std::pow((float)head_size, -0.5f);

    int element_size = sizeof(T);
    int x = 16 / element_size;

    //printf("Key cache shape:\n");
    //printf("[%d %d %d %d %d]\n", num_blocks, num_heads, head_size/x, block_size, x);
    int64_t key_elems = (int64_t) num_blocks * num_heads * (head_size / x) * block_size * x;
    kv_block_stride = key_elems / num_blocks;
    kv_head_stride = kv_block_stride / num_heads;

    //printf("Value cache shape:\n");
    //printf("[%d %d %d %d]\n", num_blocks, num_heads, head_size, block_size);
    int64_t value_elems = (int64_t)num_blocks * num_heads * head_size * block_size;

    T** key_caches_h = (T**)malloc(num_layers * sizeof(T*));
    T** value_caches_h = (T**)malloc(num_layers * sizeof(T*));

    int threads = 256;

    for (int l = 0; l < num_layers; ++l) {
        T* key_cache_d;
        T* value_cache_d;

        int64_t key_blocks = (key_elems + threads - 1) / threads;
        int64_t val_blocks = (value_elems + threads - 1) / threads;

        uniform_fill_kernel<T>

                (T*)key_cache_d,
                key_elems,
                -scale,
                scale,
                seed + l);

        uniform_fill_kernel<T>

                (T*)value_cache_d,
                value_elems,
                -scale,
                scale,
                seed + l);

        key_caches_h[l] = key_cache_d;
        value_caches_h[l] = value_cache_d;
    }

    KVCaches<T> out;
    out.key_caches = key_caches_h;
    out.value_caches = value_caches_h;
    return out;
}
