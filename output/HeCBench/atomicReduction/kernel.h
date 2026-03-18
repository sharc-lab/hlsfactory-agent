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

// --- from reduction.cu ---
/*
   Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <chrono>



// --- from kernels.h ---
void atomic_reduction(int *in, int* out, int arrayLength) {
  int sum=0;
  int idx = _bid_x*BLOCK_DIM_X+_tid_x;
  for(int i= idx;i<arrayLength;i+=BLOCK_DIM_X*GRID_DIM_X) {
    sum+=in[i];
  }
  (*out += sum);
}

void atomic_reduction_v2(int *in, int* out, int arrayLength) {
  int sum=0;
  int idx = _bid_x*BLOCK_DIM_X+_tid_x;
  for(int i= idx*2;i<arrayLength;i+=BLOCK_DIM_X*GRID_DIM_X*2) {
    sum+=in[i] + in[i+1];
  }
  (*out += sum);
}

void atomic_reduction_v4(int *in, int* out, int arrayLength) {
  int sum=0;
  int idx = _bid_x*BLOCK_DIM_X+_tid_x;
  for(int i= idx*4;i<arrayLength;i+=BLOCK_DIM_X*GRID_DIM_X*4) {
    sum+=in[i] + in[i+1] + in[i+2] + in[i+3];
  }
  (*out += sum);
}

void atomic_reduction_v8(int *in, int* out, int arrayLength) {
  int sum=0;
  int idx = _bid_x*BLOCK_DIM_X+_tid_x;
  for(int i= idx*8;i<arrayLength;i+=BLOCK_DIM_X*GRID_DIM_X*8) {
    sum+=in[i] + in[i+1] + in[i+2] + in[i+3] +in[i+4] +in[i+5] +in[i+6] +in[i+7];
  }
  (*out += sum);
}

void atomic_reduction_v16(int *in, int* out, int arrayLength) {
  int sum=0;
  int idx = _bid_x*BLOCK_DIM_X+_tid_x;
  for(int i= idx*16;i<arrayLength;i+=BLOCK_DIM_X*GRID_DIM_X*16) {
    sum+=in[i] + in[i+1] + in[i+2] + in[i+3] +in[i+4] +in[i+5] +in[i+6] +in[i+7]
      +in[i+8] +in[i+9] +in[i+10] +in[i+11] +in[i+12] +in[i+13] +in[i+14] +in[i+15] ;
  }
  (*out += sum);
}

