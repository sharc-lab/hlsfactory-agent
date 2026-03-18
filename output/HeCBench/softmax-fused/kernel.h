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
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif
#ifndef GRID_DIM_Z
#define GRID_DIM_Z 1
#endif

// --- from main.cu ---
#include <cassert>
#include <cfloat>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

#define C10_WARP_SIZE 32
typedef __nv_bfloat16 BFloat16;
typedef __half Half;

template <typename Datatype, int ELEMENTS_PER_LDG>

template <>
__inline__ void copy_vector<BFloat16, 4>(BFloat16 *dst,
                                                    const BFloat16 *src) {
  *((float2 *)dst) = *((float2 *)src);
}

template <>
__inline__ void copy_vector<Half, 1>(Half *dst, const Half *src) {
  *dst = *src;
}

template <>
__inline__ void copy_vector<Half, 4>(Half *dst, const Half *src) {
  *((float2 *)dst) = *((float2 *)src);
}

template <>
__inline__ void copy_vector<uint8_t, 1>(uint8_t *dst,
                                                   const uint8_t *src) {
  *dst = *src;
}

template <>
__inline__ void copy_vector<uint8_t, 4>(uint8_t *dst,
                                                   const uint8_t *src) {
  *((uchar4 *)dst) = *((uchar4 *)src);
}


template <typename T> struct Add {
};

template <typename T> struct Max {
};

template <typename T>
inline T
WARP_SHFL_XOR_NATIVE(T value, int laneMask, int width = warpSize,
                     unsigned int mask = 0xffffffff) {
  return 0;
}

template <typename acc_t, int WARP_BATCH, int WARP_SIZE,
          template <typename> class ReduceOp>

/*

#define LAUNCH(n)                                                              \
  scaled_masked_softmax_warp_forward<input_t, output_t, acc_t, n>              \

                            pad_batches);

template <typename input_t, typename output_t, typename acc_t>

template <typename scalar_t>

