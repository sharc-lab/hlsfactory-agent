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
// Copyright (c) 2019-2020, NVIDIA CORPORATION.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <chrono>

template <typename T>

template <typename T>



// --- from kernels.h ---
inline
void clip_plus( const bool &clip, const int &n, int &plus ) {
  if ( plus >= n ) plus = clip ? n - 1 : plus - n;
}

inline
void clip_minus( const bool &clip, const int &n, int &minus ) {
  if ( minus < 0 ) minus = clip ? 0 : minus + n;
}

///////////////////////////////////////////////////////////////////////////////
//                          BOOLRELEXTREMA 1D                                //
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void relextrema_1D(
  const int  n,
  const int  order,
  const bool clip,
  const T * inp,
  bool * results)
{
  const int tx = _bid_x * BLOCK_DIM_X + _tid_x;
  const int stride = BLOCK_DIM_X * GRID_DIM_X;

  for ( int tid = tx; tid < n; tid += stride ) {

    const T data = inp[tid];
    bool    temp = true;

    for ( int o = 1; o <= order; o++ ) {
      int plus = tid + o;
      int minus = tid - o;

      clip_plus( clip, n, plus );
      clip_minus( clip, n, minus );

      temp &= data > inp[plus];
      temp &= data >= inp[minus];
    }
    results[tid] = temp;
  }
}

template<typename T>
void relextrema_2D(
  const int  in_x,
  const int  in_y,
  const int  order,
  const bool clip,
  const int  axis,
  const T * inp,
  bool * results) 
{
  const int ty = _bid_x * BLOCK_DIM_X + _tid_x;
  const int tx = _bid_y * BLOCK_DIM_Y + _tid_y;

  if ( ( tx < in_y ) && ( ty < in_x ) ) {
    int tid = tx * in_x + ty ;

    const T data = inp[tid] ;
    bool    temp = true ;

    for ( int o = 1; o <= order; o++ ) {

      int plus;
      int minus;

      if ( axis == 0 ) {
        plus  = tx + o;
        minus = tx - o;

        clip_plus( clip, in_y, plus );
        clip_minus( clip, in_y, minus );

        plus  = plus * in_x + ty;
        minus = minus * in_x + ty;
      } else {
        plus  = ty + o;
        minus = ty - o;

        clip_plus( clip, in_x, plus );
        clip_minus( clip, in_x, minus );

        plus  = tx * in_x + plus;
        minus = tx * in_x + minus;
      }

      temp &= data > inp[plus] ;
      temp &= data >= inp[minus] ;
    }
    results[tid] = temp;
  }
}
