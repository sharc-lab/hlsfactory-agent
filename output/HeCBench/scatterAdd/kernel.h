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

// --- from main.cu ---
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>

/*
Reference
vip-token-centric-compression/src/t5/models/small_embedding/kernel.py

batch size = 4
vector dim = 5
>>> indexes = torch.randint(0, 3, size = (4, ))
tensor([1, 1, 2, 0])
>>> indexes = indexes[:, None].repeat(1, 5)
>>> indexes
tensor([[1, 1, 1, 1, 1],
        [1, 1, 1, 1, 1],
        [2, 2, 2, 2, 2],
        [0, 0, 0, 0, 0]])
outputs.scatter_add_(0, indexes, source)
*/

#define CHECK_CUDA(func)                                                       \
{                                                                              \
    cudaError_t status = func;                                                 \
}




// --- from kernels.h ---
#define MAX_THREADS_PER_BLOCK 512
#define WORK_SIZE 8192

void scatterAdd_kernel(
  const int   *indexes,// [batch_size]
  const float *source, // [batch_size, vector_dim]
  float *outputs,      // [output_size, vector_dim]
  const int batch_size,
  const int output_size,
  const int vector_dim
) 
{
  int thread_idx = _tid_y * warpSize + _tid_x;
  int batch_idx_start = _bid_x * WORK_SIZE;
  // assert BLOCK_DIM_X == warpSize
  // assert BLOCK_DIM_Y == MAX_THREADS_PER_BLOCK / warpSize

  float buffer[4096];
  float *output_buffer = buffer;
  int *index_buffer = (int*)&buffer[output_size * vector_dim];

  for (int idx_start = 0; idx_start < output_size * vector_dim; idx_start = idx_start + MAX_THREADS_PER_BLOCK) {
    int idx = idx_start + thread_idx;
    if (idx < output_size * vector_dim) {
      output_buffer[idx] = 0;
    }
  }

  for (int idx_start = 0; idx_start < WORK_SIZE; idx_start = idx_start + MAX_THREADS_PER_BLOCK) {
    int batch_idx = batch_idx_start + idx_start + thread_idx;
    if (batch_idx < batch_size) {
      index_buffer[thread_idx] = indexes[batch_idx];
    }
    for (int buffer_idx_start = 0; buffer_idx_start < MAX_THREADS_PER_BLOCK;
             buffer_idx_start = buffer_idx_start + MAX_THREADS_PER_BLOCK / warpSize) {
      int buffer_idx = buffer_idx_start + _tid_y;
      int batch_idx = batch_idx_start + idx_start + buffer_idx;
      if (batch_idx < batch_size) {
        int index = index_buffer[buffer_idx];
        for (int j_start = 0; j_start < vector_dim; j_start = j_start + warpSize) {
          int j = j_start + _tid_x;
          if (j < vector_dim) {
            (output_buffer[index * vector_dim + j] += source[(size_t)batch_idx * (size_t)vector_dim + (size_t)j]);
          }
        }
      }
    }
  }

  for (int idx_start = 0; idx_start < output_size * vector_dim; idx_start = idx_start + MAX_THREADS_PER_BLOCK) {
    int idx = idx_start + thread_idx;
    if (idx < output_size * vector_dim) {
      (outputs[idx] += output_buffer[idx]);
    }
  }
}

//
// simplify address generations in the kernel
//
void scatterAdd2_kernel(
  const int   *indexes,// [batch_size]
  const float *source, // [batch_size, vector_dim]
  float *outputs,      // [output_size, vector_dim]
  const int batch_size,
  const int output_size,
  const int vector_dim
) 
{
  int thread_idx = _tid_y * warpSize + _tid_x;
  int batch_idx_start = _bid_x * WORK_SIZE;

  float buffer[4096];
  float *output_buffer = buffer;
  int *index_buffer = (int*)&buffer[output_size * vector_dim];

  for (int idx = thread_idx; idx < output_size * vector_dim; idx += MAX_THREADS_PER_BLOCK) {
    output_buffer[idx] = 0;
  }

  for (int idx_start = 0; idx_start < WORK_SIZE; idx_start = idx_start + MAX_THREADS_PER_BLOCK) {
    int batch_idx = batch_idx_start + idx_start + thread_idx;
    if (batch_idx < batch_size) {
      index_buffer[thread_idx] = indexes[batch_idx];
    }
    for (int buffer_idx_start = 0; buffer_idx_start < MAX_THREADS_PER_BLOCK;
             buffer_idx_start = buffer_idx_start + MAX_THREADS_PER_BLOCK / warpSize) {
      int buffer_idx = buffer_idx_start + _tid_y;
      int batch_idx = batch_idx_start + idx_start + buffer_idx;
      if (batch_idx < batch_size) {
        int index = index_buffer[buffer_idx];
        for (int j = _tid_x; j < vector_dim; j += warpSize) {
          (output_buffer[index * vector_dim + j] += source[(size_t)batch_idx * (size_t)vector_dim + (size_t)j]);
        }
      }
    }
  }

  for (int idx = thread_idx; idx < output_size * vector_dim; idx += MAX_THREADS_PER_BLOCK) {
    (outputs[idx] += output_buffer[idx]);
  }
}

