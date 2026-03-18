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
/**********
  Copyright (c) 2017, Xilinx, Inc.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,


// --- from kernel.h ---
void affine (
  const unsigned short * src,
        unsigned short * dst) 
{
  int x = _bid_x*BLOCK_DIM_X+_tid_x;
  int y = _bid_y*BLOCK_DIM_Y+_tid_y;

  const float lx_rot   = 30.0f;
  const float ly_rot   = 0.0f; 
  const float lx_expan = 0.5f;
  const float ly_expan = 0.5f; 
  int   lx_move  = 0;
  int   ly_move  = 0;
  float affine[2][2];   // coefficients
  float i_affine[2][2];
  float beta[2];
  float i_beta[2];
  float det;
  float x_new, y_new;
  float x_frac, y_frac;
  float gray_new;
  int   m, n;
  unsigned short output_buffer;

  // forward affine transformation 
  affine[0][0] = lx_expan * cosf(lx_rot*PI/180.0f);
  affine[0][1] = ly_expan * sinf(ly_rot*PI/180.0f);
  affine[1][0] = lx_expan * sinf(lx_rot*PI/180.0f);
  affine[1][1] = ly_expan * cosf(ly_rot*PI/180.0f);
  beta[0]      = lx_move;
  beta[1]      = ly_move;

  // determination of inverse affine transformation
  det = (affine[0][0] * affine[1][1]) - (affine[0][1] * affine[1][0]);
  if (det == 0.0f)
  {
    i_affine[0][0] = 1.0f;
    i_affine[0][1] = 0.0f;
    i_affine[1][0] = 0.0f;
    i_affine[1][1] = 1.0f;
    i_beta[0]      = -beta[0];
    i_beta[1]      = -beta[1];
  } 
  else 
  {
    i_affine[0][0] =  affine[1][1]/det;
    i_affine[0][1] = -affine[0][1]/det;
    i_affine[1][0] = -affine[1][0]/det;
    i_affine[1][1] =  affine[0][0]/det;
    i_beta[0]      = -i_affine[0][0]*beta[0]-i_affine[0][1]*beta[1];
    i_beta[1]      = -i_affine[1][0]*beta[0]-i_affine[1][1]*beta[1];
  }

  // Output image generation by inverse affine transformation and bilinear transformation

  x_new  = i_beta[0] + i_affine[0][0]*(x-X_SIZE/2.0f) + i_affine[0][1]*(y-Y_SIZE/2.0f) + X_SIZE/2.0f;
  y_new  = i_beta[1] + i_affine[1][0]*(x-X_SIZE/2.0f) + i_affine[1][1]*(y-Y_SIZE/2.0f) + Y_SIZE/2.0f;

  m      = (int)floorf(x_new);
  n      = (int)floorf(y_new);

  x_frac = x_new - m;
  y_frac = y_new - n;

  if ((m >= 0) && (m + 1 < X_SIZE) && (n >= 0) && (n+1 < Y_SIZE))
  {
    gray_new = (1.0f - y_frac) * ((1.0f - x_frac) * (src[(n * X_SIZE) + m]) + x_frac * (src[(n * X_SIZE) + m + 1])) + 
      y_frac  * ((1.0f - x_frac) * (src[((n + 1) * X_SIZE) + m]) + x_frac * (src[((n + 1) * X_SIZE) + m + 1]));

    output_buffer = (unsigned short)gray_new;
  } 
  else if (((m + 1 == X_SIZE) && (n >= 0) && (n < Y_SIZE)) || ((n + 1 == Y_SIZE) && (m >= 0) && (m < X_SIZE))) 
  {
    output_buffer = src[(n * X_SIZE) + m];
  } 
  else 
  {
    output_buffer = WHITE;
  }

  dst[(y * X_SIZE)+x] = output_buffer;
}

