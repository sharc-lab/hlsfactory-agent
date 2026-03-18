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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <hip/hip_runtime.h>
#include "reference.h"

#define threadsPerBlock 512

// Kernel for fast unfold+copy on volumes
template <typename T>
void vol2col_kernel(
    const int64_t range,
    const T* data_vol,
    const int depth,
    const int height,
    const int width,
    const int ksize_t,
    const int ksize_h,
    const int ksize_w,
    const int pad_t,
    const int pad_h,
    const int pad_w,
    const int stride_t,
    const int stride_h,
    const int stride_w,
    const int dilation_t,
    const int dilation_h,
    const int dilation_w,
    const int depth_col,
    const int height_col,
    const int width_col,
    T* data_col)
{
}

template <typename T, typename accT>
void col2vol_kernel(
    const int64_t n,
    const T* data_col,
    const int depth,
    const int height,
    const int width,
    const int kernel_t,
    const int kernel_h,
    const int kernel_w,
    const int pad_t,
    const int pad_h,
    const int pad_w,
    const int stride_t,
    const int stride_h,
    const int stride_w,
    const int dilation_t,
    const int dilation_h,
    const int dilation_w,
    const int depth_col,
    const int height_col,
    const int width_col,
    T* data_vol)
{
}


template <typename T>
void eval (
    const int repeat,
    const int channels,
    const int depth,
    const int height,
    const int width,
    const int depth_col,
    const int height_col,
    const int width_col,
    const int ksize_t,
    const int ksize_h,
    const int ksize_w,
    const int pad_t,
    const int pad_h,
    const int pad_w,
    const int stride_t,
    const int stride_h,
    const int stride_w,
    const int dilation_t,
    const int dilation_h,
    const int dilation_w)
{
  int64_t vol_size = (int64_t) channels * (2*pad_t+depth) * (2*pad_h+height) * (2*pad_w+width);
  int64_t col_size = ((int64_t) channels * ksize_t * ksize_h * ksize_w + 1) *
                    (depth_col+pad_t) * (height_col+pad_h) * (width_col+pad_w);

  int64_t vol_size_bytes = sizeof(T) * vol_size;
  int64_t col_size_bytes = sizeof(T) * col_size;

  T *h_data_vol = (T*) malloc (vol_size_bytes);
  T *h_data_col = (T*) malloc (col_size_bytes);

  T *h_data_vol_ref = (T*) malloc (vol_size_bytes);
  T *h_data_col_ref = (T*) malloc (col_size_bytes);


  T *d_data_vol;
  hipMalloc((void**)&d_data_vol, vol_size_bytes);
  hipMemcpy(d_data_vol, h_data_vol, vol_size_bytes, hipMemcpyHostToDevice);

  T *d_data_col;
  hipMalloc((void**)&d_data_col, col_size_bytes);
  hipMemset(d_data_col, 0, col_size_bytes);

  // each of "channels * depth_col * height_col * width_col"
  // blocks responsible for copying a single-channel grid.
  // We cast an operand to int64 so that the product will not overflow
  int64_t n = static_cast<int64_t>(channels) * depth_col * height_col * width_col;

  int blocksPerGrid = get_blocks(n);

  hipDeviceSynchronize();
  auto start = std::chrono::steady_clock::now();


  hipDeviceSynchronize();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average execution time of vol2col kernel: %f (us)\n", (time * 1e-3f) / repeat);

  hipMemcpy(h_data_col, d_data_col, col_size_bytes, hipMemcpyDeviceToHost);

  // verify
  vol2col_reference<T>(
      h_data_vol,
      channels, depth, height, width,
      ksize_t, ksize_h, ksize_w,
      pad_t, pad_h, pad_w,
      stride_t, stride_h, stride_w,
      dilation_t, dilation_h, dilation_w,
      depth_col, height_col, width_col,
      h_data_col_ref);

  int error = memcmp(h_data_col_ref, h_data_col, col_size_bytes);
  printf("%s\n", error ? "FAIL" : "PASS");

  hipDeviceSynchronize();
  start = std::chrono::steady_clock::now();


  hipDeviceSynchronize();
  end = std::chrono::steady_clock::now();
  time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average execution time of col2vol kernel: %f (us)\n", (time * 1e-3f) / repeat);

  hipMemcpy(h_data_vol, d_data_vol, vol_size_bytes, hipMemcpyDeviceToHost);

  // verify
  col2vol_reference<T, T>(
      h_data_col_ref,
      channels, depth, height, width,
      ksize_t, ksize_h, ksize_w,
      pad_t, pad_h, pad_w,
      stride_t, stride_h, stride_w,
      dilation_t, dilation_h, dilation_w,
      depth_col, height_col, width_col,
      h_data_vol_ref);

  printf("%s\n", error ? "FAIL" : "PASS");

  hipFree(d_data_vol);
  hipFree(d_data_col);
  free(h_data_vol);
  free(h_data_col);
  free(h_data_vol_ref);
  const int repeat = atoi(argv[1]);

  int channels = 4;
  int depth = 3;
  int height = 255;
  int width = 255;
  int pad_t = 1;
  int pad_h = 1;
  int pad_w = 1;
  int stride_t = 2;
  int stride_h = 2;
  int stride_w = 2;
  int dilation_t = 2;
  int dilation_h = 2;
  int dilation_w = 2;
  int depth_col = 3;
  int height_col = 255;
  int width_col = 255;


  return 0;
}
