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
#include <iostream>
#include <cstring>

#ifdef DEBUG
#endif





// --- from kernel.h ---
// a is a multiple of WGS for simplicity
#define N 8192
#define WGS 256
#define SAMPLE_TEST_LEN 20000

void
lstm_inference(
  const float* d_x, 
  const float* d_inW, 
  const float* d_intW, 
  const float* d_intB, 
  const float* d_outW, 
  const float* d_outB, 
        float* d_y)
{
  int gid = BLOCK_DIM_X * _bid_x + _tid_x;
  if (gid >= N) return;

  int t,i,j;
  float h_state[5] = {0,0,0,0,0};
  float c_state[5] = {0,0,0,0,0};
  float i_state[5] = {0,0,0,0,0};
  float f_state[5] = {0,0,0,0,0};
  float o_state[5] = {0,0,0,0,0};
  float g_state[5] = {0,0,0,0,0};

  for (t = 0; t < SAMPLE_TEST_LEN; ++t) {

    float x = d_x[gid * SAMPLE_TEST_LEN + t];

    for (j = 0; j < 5; ++j) {
      i_state[j] = d_inW[j] * x;
      for (i = 0; i < 5; ++i)
        i_state[j] += h_state[i] * d_intW[j*5+i];
      i_state[j] += d_intB[j];
      i_state[j] = 1.f / (1.f + expf(-i_state[j]));
    }

    for (j = 0; j < 5; ++j) {
      f_state[j] = d_inW[5+j] * x;
      for (i = 0; i < 5; ++i)
        f_state[j] += h_state[i] * d_intW[25+j*5+i];
      f_state[j] += d_intB[5+j];
      f_state[j] = 1.f / (1.f + expf(-f_state[j]));
    }

    for (j = 0; j < 5; ++j) {
      o_state[j] = d_inW[10+j] * x;
      for (i = 0; i < 5; ++i)
        o_state[j] += h_state[i] * d_intW[50+j*5+i];
      o_state[j] += d_intB[10+j];
      o_state[j] = 1.f / (1.f + expf(-o_state[j]));
    }

    for (j = 0; j < 5; ++j) {
      g_state[j] = d_inW[15+j] * x;
      for (i = 0; i < 5; ++i)
        g_state[j] += h_state[i] * d_intW[75+j*5+i];
      g_state[j] += d_intB[15+j];
      g_state[j] = tanhf(g_state[j]);
    }

    for (j = 0; j < 5; ++j) {
      c_state[j] = c_state[j] * f_state[j] + g_state[j] * i_state[j];
      h_state[j] = tanhf(c_state[j]) * o_state[j];
    }

    float y = d_outB[0];
    for (j = 0; j < 5; ++j)
      y += h_state[j] * d_outW[j];
    d_y[gid * SAMPLE_TEST_LEN + t] = y;
  }
}
