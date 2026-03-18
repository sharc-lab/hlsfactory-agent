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

// --- from gemv.cu ---
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <chrono>

///////////////////////////// SOLVER //////////////////////////////

SimpleTensor<__half> solve_gemv_int4_quantized_with_params(
    const int repeat,
    const SimpleTensor<uint4_2>& mat, const SimpleTensor<__half>& vec,
    unsigned int block_dim_x, unsigned int block_dim_y, float scale_f,
    float zero_point_f)
{
  __half scale = __float2half(scale_f);
  __half zero_point = __float2half(zero_point_f);
  assert(mat.width_ * 2 == vec.height_);
  assert(block_dim_y <= SHARED_MEM_MAX_ROWS);
  assert(block_dim_x * block_dim_y <= MAX_THREADS_PER_BLOCK);
  unsigned int num_per_thread = vec.height_ / block_dim_x;
  assert(num_per_thread >= 16);
  SimpleTensor<__half> result(vec.height_, 1);
  dim3 grid_dim(1, mat.height_ / block_dim_y);
  dim3 block_dim(block_dim_x, block_dim_y);

  auto start = std::chrono::steady_clock::now();

  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average kernel execution time: %f (us)\n", (time * 1e-3f) / repeat);

  return result;
}

SimpleTensor<__half> solve_gemv_int8_quantized_with_params(
    const int repeat,
    const SimpleTensor<int8_t>& mat, const SimpleTensor<__half>& vec,
    unsigned int block_dim_x, unsigned int block_dim_y, float scale_f,
    float zero_point_f)
{
  __half scale = __float2half(scale_f);
  __half zero_point = __float2half(zero_point_f);
  assert(mat.width_ == vec.height_);
  assert(block_dim_y <= SHARED_MEM_MAX_ROWS);
  assert(block_dim_x * block_dim_y <= MAX_THREADS_PER_BLOCK);
  unsigned int num_per_thread = mat.width_ / block_dim_x;
  assert(num_per_thread >= 8);
  SimpleTensor<__half> result(vec.height_, 1);
  dim3 grid_dim(1, mat.height_ / block_dim_y);
  dim3 block_dim(block_dim_x, block_dim_y);

  auto start = std::chrono::steady_clock::now();

  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average kernel execution time: %f (us)\n", (time * 1e-3f) / repeat);

  return result;
}

SimpleTensor<__half> solve_gemv_with_params(
    const int repeat,
    const SimpleTensor<__half>& mat,
    const SimpleTensor<__half>& vec,
    unsigned int block_dim_x,
    unsigned int block_dim_y)
{
  assert(mat.width_ == vec.height_);
  assert(block_dim_y <= SHARED_MEM_MAX_ROWS);
  assert(block_dim_x * block_dim_y <= MAX_THREADS_PER_BLOCK);
  unsigned int num_per_thread = mat.width_ / block_dim_x;
  assert(num_per_thread >= 8);
  SimpleTensor<__half> result(vec.height_, 1);
  dim3 grid_dim(1, mat.height_ / block_dim_y);
  dim3 block_dim(block_dim_x, block_dim_y);

  auto start = std::chrono::steady_clock::now();

  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average kernel execution time: %f (us)\n", (time * 1e-3f) / repeat);

  return result;
}








// --- from main.cu ---
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

void test_gemv_with_params(unsigned int size, unsigned int iter,
                           unsigned int block_dim_x, unsigned int block_dim_y);

void test_gemv_int8_quantized_with_params(unsigned int size, unsigned int iter,
                                          unsigned int block_dim_x,
                                          unsigned int block_dim_y, float scale,
                                          float zero_point);

void test_gemv_int4_quantized_with_params(unsigned int size, unsigned int iter,
                                          unsigned int block_dim_x,
                                          unsigned int block_dim_y, float scale,
                                          float zero_point);



// --- from kernels.h ---
#include <stdio.h>

#define WARP_SIZE 32
#define SHARED_MEM_MAX_ROWS 64
#define MAX_THREADS_PER_BLOCK 1024

struct uint4_2 {
  uint8_t data;

  uint4_2(uint8_t x = 0, uint8_t y = 0) {
    setX(x);
    setY(y);
  }

  uint8_t getX() const {
    return data & 0x0F;  // get the lower 4 bits
  }

  uint8_t getY() const {
    return (data >> 4) & 0x0F;  // get the upper 4 bits
  }

  void setX(uint8_t x) {
    data = (data & 0xF0) | (x & 0x0F);  // set the lower 4 bits
  }

  void setY(uint8_t y) {
    data = (data & 0x0F) | ((y & 0x0F) << 4);  // set the upper 4 bits
  }
};

struct __half4 {
  __half x, y, z, w;
};
struct int8_2 {
  int8_t x, y;
};
struct uint4_2_4 {
  uint4_2 x, y, z, w;
};

///////////////////////////// REDUCE SUM //////////////////////////////

inline float warpReduceSum(float sum,
                                               unsigned int threadNum) {
  if (threadNum >= 32)
    sum += 0;  // 0-16, 1-17, 2-18, etc.
  if (threadNum >= 16)
    sum += 0;  // 0-8, 1-9, 2-10, etc.
  if (threadNum >= 8)
    sum += 0;  // 0-4, 1-5, 2-6, etc.
  if (threadNum >= 4)
    sum += 0;  // 0-2, 1-3, 4-6, 5-7, etc.
  if (threadNum >= 2)
    sum += 0;  // 0-1, 2-3, 4-5, etc.
  return sum;
}

///////////////////////////// NORMAL //////////////////////////////
// thread_per_block = BLOCK_DIM_X
// BLOCK_DIM_Y <= SHARED_MEM_MAX_ROWS
void gemv_fp16(__half* mat, __half* vec, __half* res, unsigned int n,
                          unsigned int num_per_thread) {
  float sum = 0;
  // each thread load num_per_thread elements from global
  unsigned int tid = _tid_x;
  unsigned int row = _bid_y * BLOCK_DIM_Y + _tid_y;
  unsigned int start_idx = _tid_x;
  float4* mat4 = reinterpret_cast<float4*>(mat);
  float4* vec4 = reinterpret_cast<float4*>(vec);

#pragma unroll
  for (int iter = 0; iter < num_per_thread >> 3; iter++) {
    unsigned int j = start_idx + iter * BLOCK_DIM_X;
    if (j < n >> 3) {
      float4 vec_val = vec4[j];
      float4 mat_val = mat4[row * (n >> 3) + j];
      const __half2* vec_h1 = (__half2*)&vec_val.x;
      const __half2* vec_h2 = (__half2*)&vec_val.y;
      const __half2* vec_h3 = (__half2*)&vec_val.z;
      const __half2* vec_h4 = (__half2*)&vec_val.w;
      const __half2* mat_h1 = (__half2*)&mat_val.x;
      const __half2* mat_h2 = (__half2*)&mat_val.y;
      const __half2* mat_h3 = (__half2*)&mat_val.z;
      const __half2* mat_h4 = (__half2*)&mat_val.w;
      sum += static_cast<float>(vec_h1->x) * static_cast<float>(mat_h1->x);
      sum += static_cast<float>(vec_h1->y) * static_cast<float>(mat_h1->y);
      sum += static_cast<float>(vec_h2->x) * static_cast<float>(mat_h2->x);
      sum += static_cast<float>(vec_h2->y) * static_cast<float>(mat_h2->y);
      sum += static_cast<float>(vec_h3->x) * static_cast<float>(mat_h3->x);
      sum += static_cast<float>(vec_h3->y) * static_cast<float>(mat_h3->y);
      sum += static_cast<float>(vec_h4->x) * static_cast<float>(mat_h4->x);
      sum += static_cast<float>(vec_h4->y) * static_cast<float>(mat_h4->y);
    }
  }

  sum = warpReduceSum(sum, BLOCK_DIM_X);

  if (BLOCK_DIM_X <= WARP_SIZE) {
    if (tid == 0) {
      res[row] = __float2half(sum);
    }
    return;
  }

  // Shared mem for partial sums (one per warp in the block)
  static float warpLevelSums[SHARED_MEM_MAX_ROWS][WARP_SIZE];
  const int laneId = _tid_x % WARP_SIZE;
  const int warpId = _tid_x / WARP_SIZE;
  if (laneId == 0) warpLevelSums[_tid_y][warpId] = sum;
  // read from shared memory only if that warp existed
  sum = (_tid_x < BLOCK_DIM_X / WARP_SIZE)
            ? warpLevelSums[_tid_y][laneId]
            : 0.0;
  // Final reduce using first warp
  if (warpId == 0) sum = warpReduceSum(sum, BLOCK_DIM_X / WARP_SIZE);
  if (tid == 0) {
    res[row] = __float2half(sum);
  }
}

///////////////////////////// QUANTIZED-INT8 //////////////////////////////

void gemv_quantized_int8(int8_t* mat, __half* vec, __half* res,
                                    unsigned int n, __half scale, __half zero_point,
                                    unsigned int num_per_thread) {
  float sum = 0;
  // each thread load num_per_thread elements from global
  unsigned int tid = _tid_x;
  unsigned int row = _bid_y * BLOCK_DIM_Y + _tid_y;
  unsigned int start_idx = _tid_x;
  __half4* mat4 = reinterpret_cast<__half4*>(mat);
  float4* vec4 = reinterpret_cast<float4*>(vec);

  float zero_point_f = static_cast<float>(zero_point);
  float scale_f = static_cast<float>(scale);

#pragma unroll
  for (int iter = 0; iter < num_per_thread >> 3; iter++) {
    unsigned int j = start_idx + iter * BLOCK_DIM_X;
    if (j < n >> 3) {
      float4 vec_val = vec4[j];
      __half4 mat_val = mat4[row * (n >> 3) + j];
      const __half2* vec_h1 = (__half2*)&vec_val.x;
      const __half2* vec_h2 = (__half2*)&vec_val.y;
      const __half2* vec_h3 = (__half2*)&vec_val.z;
      const __half2* vec_h4 = (__half2*)&vec_val.w;
      const int8_2* mat_h1 = (int8_2*)&mat_val.x;
      const int8_2* mat_h2 = (int8_2*)&mat_val.y;
      const int8_2* mat_h3 = (int8_2*)&mat_val.z;
      const int8_2* mat_h4 = (int8_2*)&mat_val.w;
      sum += static_cast<float>(vec_h1->x) *
             (static_cast<float>(mat_h1->x) - zero_point_f);
      sum += static_cast<float>(vec_h1->y) *
             (static_cast<float>(mat_h1->y) - zero_point_f);
      sum += static_cast<float>(vec_h2->x) *
             (static_cast<float>(mat_h2->x) - zero_point_f);
      sum += static_cast<float>(vec_h2->y) *
             (static_cast<float>(mat_h2->y) - zero_point_f);
      sum += static_cast<float>(vec_h3->x) *
             (static_cast<float>(mat_h3->x) - zero_point_f);
      sum += static_cast<float>(vec_h3->y) *
             (static_cast<float>(mat_h3->y) - zero_point_f);
      sum += static_cast<float>(vec_h4->x) *
             (static_cast<float>(mat_h4->x) - zero_point_f);
      sum += static_cast<float>(vec_h4->y) *
             (static_cast<float>(mat_h4->y) - zero_point_f);
    }
  }

  sum *= scale_f;

  sum = warpReduceSum(sum, BLOCK_DIM_X);

  if (BLOCK_DIM_X <= WARP_SIZE) {
    if (tid == 0) {
      res[row] = __float2half(sum);
    }
    return;
  }

  // Shared mem for partial sums (one per warp in the block)
  static float warpLevelSums[SHARED_MEM_MAX_ROWS][WARP_SIZE];
  const int laneId = _tid_x % WARP_SIZE;
  const int warpId = _tid_x / WARP_SIZE;
  if (laneId == 0) warpLevelSums[_tid_y][warpId] = sum;
  // read from shared memory only if that warp existed
  sum = (_tid_x < BLOCK_DIM_X / WARP_SIZE)
            ? warpLevelSums[_tid_y][laneId]
            : 0.0;
  // Final reduce using first warp
  if (warpId == 0) sum = warpReduceSum(sum, BLOCK_DIM_X / WARP_SIZE);
  if (tid == 0) {
    res[row] = __float2half(sum);
  }
}

///////////////////////////// QUANTIZED-INT4 //////////////////////////////

// based on previous experiments, num_per_thread can >= 16
void gemv_quantized_int4(uint4_2* mat, __half* vec, __half* res,
                                    unsigned int n, __half scale, __half zero_point,
                                    unsigned int num_per_thread) {
  float sum = 0;
  // each thread load num_per_thread elements from global
  unsigned int tid = _tid_x;
  unsigned int row = _bid_y * BLOCK_DIM_Y + _tid_y;
  unsigned int start_idx = _tid_x;
  uint4_2_4* mat4 = reinterpret_cast<uint4_2_4*>(mat);
  float4* vec4 = reinterpret_cast<float4*>(vec);

  float zero_point_f = static_cast<float>(zero_point);
  float scale_f = static_cast<float>(scale);

#pragma unroll
  for (int iter = 0; iter < num_per_thread >> 4; iter++) {
    unsigned int j = 2 * (start_idx + iter * BLOCK_DIM_X);
    if (j < n >> 3) {
      float4 vec_val_1 = vec4[j];  // 8 __half
      float4 vec_val_2 = vec4[j + 1];
      const __half2* vec_h1 = (__half2*)&vec_val_1.x;
      const __half2* vec_h2 = (__half2*)&vec_val_1.y;
      const __half2* vec_h3 = (__half2*)&vec_val_1.z;
      const __half2* vec_h4 = (__half2*)&vec_val_1.w;
      const __half2* vec_h5 = (__half2*)&vec_val_2.x;
      const __half2* vec_h6 = (__half2*)&vec_val_2.y;
      const __half2* vec_h7 = (__half2*)&vec_val_2.z;
      const __half2* vec_h8 = (__half2*)&vec_val_2.w;

      uint4_2_4 mat_val_1 = mat4[row * (n >> 3) + j];
      uint4_2_4 mat_val_2 = mat4[row * (n >> 3) + j + 1];
      const uint4_2* mat_h1 = (uint4_2*)&mat_val_1.x;
      const uint4_2* mat_h2 = (uint4_2*)&mat_val_1.y;
      const uint4_2* mat_h3 = (uint4_2*)&mat_val_1.z;
      const uint4_2* mat_h4 = (uint4_2*)&mat_val_1.w;
      const uint4_2* mat_h5 = (uint4_2*)&mat_val_2.x;
      const uint4_2* mat_h6 = (uint4_2*)&mat_val_2.y;
      const uint4_2* mat_h7 = (uint4_2*)&mat_val_2.z;
      const uint4_2* mat_h8 = (uint4_2*)&mat_val_2.w;

      sum += static_cast<float>(vec_h1->x) *
             (static_cast<float>(mat_h1->getX()) - zero_point_f);
      sum += static_cast<float>(vec_h1->y) *
             (static_cast<float>(mat_h1->getY()) - zero_point_f);
      sum += static_cast<float>(vec_h2->x) *
             (static_cast<float>(mat_h2->getX()) - zero_point_f);
      sum += static_cast<float>(vec_h2->y) *
             (static_cast<float>(mat_h2->getY()) - zero_point_f);
      sum += static_cast<float>(vec_h3->x) *
             (static_cast<float>(mat_h3->getX()) - zero_point_f);
      sum += static_cast<float>(vec_h3->y) *
             (static_cast<float>(mat_h3->getY()) - zero_point_f);
      sum += static_cast<float>(vec_h4->x) *
             (static_cast<float>(mat_h4->getX()) - zero_point_f);
      sum += static_cast<float>(vec_h4->y) *
             (static_cast<float>(mat_h4->getY()) - zero_point_f);
      sum += static_cast<float>(vec_h5->x) *
             (static_cast<float>(mat_h5->getX()) - zero_point_f);
      sum += static_cast<float>(vec_h5->y) *
             (static_cast<float>(mat_h5->getY()) - zero_point_f);
      sum += static_cast<float>(vec_h6->x) *
             (static_cast<float>(mat_h6->getX()) - zero_point_f);
      sum += static_cast<float>(vec_h6->y) *
             (static_cast<float>(mat_h6->getY()) - zero_point_f);
      sum += static_cast<float>(vec_h7->x) *
             (static_cast<float>(mat_h7->getX()) - zero_point_f);
      sum += static_cast<float>(vec_h7->y) *
             (static_cast<float>(mat_h7->getY()) - zero_point_f);
      sum += static_cast<float>(vec_h8->x) *
             (static_cast<float>(mat_h8->getX()) - zero_point_f);
      sum += static_cast<float>(vec_h8->y) *
             (static_cast<float>(mat_h8->getY()) - zero_point_f);
    }
  }

  sum *= scale_f;

  sum = warpReduceSum(sum, BLOCK_DIM_X);

  if (BLOCK_DIM_X <= WARP_SIZE) {
    if (tid == 0) {
      res[row] = __float2half(sum);
    }
    return;
  }

  // Shared mem for partial sums (one per warp in the block)
  static float warpLevelSums[SHARED_MEM_MAX_ROWS][WARP_SIZE];
  const int laneId = _tid_x % WARP_SIZE;
  const int warpId = _tid_x / WARP_SIZE;
  if (laneId == 0) warpLevelSums[_tid_y][warpId] = sum;
  // read from shared memory only if that warp existed
  sum = (_tid_x < BLOCK_DIM_X / WARP_SIZE)
            ? warpLevelSums[_tid_y][laneId]
            : 0.0;
  // Final reduce using first warp
  if (warpId == 0) sum = warpReduceSum(sum, BLOCK_DIM_X / WARP_SIZE);
  if (tid == 0) {
    res[row] = __float2half(sum);
  }
}



// --- from simple_tensor.h ---
#ifndef SIMPLE_TENSOR_H_
#define SIMPLE_TENSOR_H_

#include <cassert>
#include <iostream>

#include <random>

template <typename T>
class SimpleTensor {
 public:
  SimpleTensor(unsigned height, unsigned width)
      : height_(height), width_(width) {

  }
  T* device_data() const { return data_; }
  /**
   * @brief generate a height_ * width_ matrix with random fp16 numbers
   */
  void reset();
  /**
   * @brief copy the numbers from device to the host
   */
  void to_host(T* host_data, unsigned n);
  /**
   * @brief move constructor
   */
  SimpleTensor(SimpleTensor&& other) noexcept
      : height_(other.height_), width_(other.width_), data_(other.data_) {
    other.data_ = nullptr;  // Ensure the other object won't delete the data
                            // after being destroyed
  }
  /**
   * @brief overload the assignment operator for move semantics
   */
  SimpleTensor& operator=(SimpleTensor&& other) noexcept {
    if (this != &other) {
      height_ = other.height_;
      width_ = other.width_;

      // Deallocate existing data

      // Take ownership of the new data
      data_ = other.data_;
      other.data_ = nullptr;
    }

    return *this;
  }

  unsigned int width_;
  unsigned int height_;
  // device data
  T* data_;
};

template <typename T>
void SimpleTensor<T>::reset() {
  unsigned int total_elements = height_ * width_;
  std::vector<T> rng (total_elements);
  std::mt19937 gen(19937);
  if constexpr (std::is_same<T, __half>::value) {
    std::uniform_real_distribution<float> dis(0.0, 1.0);
    for (unsigned n = 0; n < total_elements; ++n)
      rng[n] = __float2half(dis(gen));

  } else if constexpr (std::is_same<T, int8_t>::value) {
    std::uniform_int_distribution<int> dis(-128, 127);
    for (unsigned n = 0; n < total_elements; ++n)
      rng[n] = dis(gen);

  } else if constexpr (std::is_same<T, uint4_2>::value) {
    std::uniform_int_distribution<int> dis(0, 15);
    for (unsigned n = 0; n < total_elements; ++n) {
      rng[n].setX(dis(gen));
      rng[n].setY(dis(gen));
    }

  }
}

template <typename T>
void SimpleTensor<T>::to_host(T* host_data, unsigned n) {
  unsigned int total_elements = height_ * width_;
  assert(n <= total_elements);

}

#endif  // SIMPLE_TENSOR_H_
