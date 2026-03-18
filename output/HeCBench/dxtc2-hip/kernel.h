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
/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Utilities and system includes
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <float.h>  // for FLT_MAX
#include <chrono>

#include "shrUtils.h"

#define ERROR_THRESHOLD 0.02f

#define NUM_THREADS 64  // Number of threads per block.

// Use power method to find the first eigenvector.
// https://en.wikipedia.org/wiki/Power_iteration

template <class T>

////////////////////////////////////////////////////////////////////////////////
// Round color to RGB565 and expand
////////////////////////////////////////////////////////////////////////////////

float alphaTable4[4] = {9.0f, 0.0f, 6.0f, 3.0f};
float alphaTable3[4] = {4.0f, 0.0f, 2.0f, 2.0f};
const int prods4[4] = {0x090000, 0x000900, 0x040102, 0x010402};
const int prods3[4] = {0x040000, 0x000400, 0x040101, 0x010401};

#define USE_TABLES 1

////////////////////////////////////////////////////////////////////////////////
// Evaluate permutations
////////////////////////////////////////////////////////////////////////////////


void evalAllPermutations(const float3 *colors,
                                    const uint *permutations, ushort &bestStart,
                                    ushort &bestEnd, uint &bestPermutation,
                                    float *errors, float3 color_sum,

  const int idx = _tid_x;

  float bestError = FLT_MAX;

  uint s_permutations[160];




  errors[idx] = bestError;
}

////////////////////////////////////////////////////////////////////////////////
// Find index with minimum error
////////////////////////////////////////////////////////////////////////////////

  const int idx = _tid_x;
  int indices[NUM_THREADS];
  indices[idx] = idx;


  return indices[0];
}

////////////////////////////////////////////////////////////////////////////////
// Save DXT block
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Compress color block
////////////////////////////////////////////////////////////////////////////////

// Helper structs and functions to validate the output of the compressor.
// We cannot simply do a bitwise compare, because different compilers produce
// different
// results for different targets due to floating point arithmetic.

union Color32 {
  struct {
    unsigned char b, g, r, a;
  };
  unsigned int u;
};

union Color16 {
  struct {
    unsigned short b : 5;
    unsigned short g : 6;
    unsigned short r : 5;
  };
  unsigned short u;
};

struct BlockDXT1 {
  Color16 col0;
  Color16 col1;
  union {
    unsigned char row[4];
    unsigned int indices;
  };

  void decompress(Color32 colors[16]) const;
};

void BlockDXT1::decompress(Color32 *colors) const {
  Color32 palette[4];

  // Does bit expansion before interpolation.
  palette[0].b = (col0.b << 3) | (col0.b >> 2);
  palette[0].g = (col0.g << 2) | (col0.g >> 4);
  palette[0].r = (col0.r << 3) | (col0.r >> 2);
  palette[0].a = 0xFF;

  palette[1].r = (col1.r << 3) | (col1.r >> 2);
  palette[1].g = (col1.g << 2) | (col1.g >> 4);
  palette[1].b = (col1.b << 3) | (col1.b >> 2);
  palette[1].a = 0xFF;


}



////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////


// --- from dds.h ---
/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DDS_H
#define DDS_H

#if !defined(MAKEFOURCC)
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                \
  ((unsigned int)(ch0) | ((unsigned int)(ch1) << 8) | \
   ((unsigned int)(ch2) << 16) | ((unsigned int)(ch3) << 24))
#endif

typedef unsigned int uint;
typedef unsigned short ushort;

struct DDSPixelFormat {
  uint size;
  uint flags;
  uint fourcc;
  uint bitcount;
  uint rmask;
  uint gmask;
  uint bmask;
  uint amask;
};

struct DDSCaps {
  uint caps1;
  uint caps2;
  uint caps3;
  uint caps4;
};

/// DDS file header.
struct DDSHeader {
  uint fourcc;
  uint size;
  uint flags;
  uint height;
  uint width;
  uint pitch;
  uint depth;
  uint mipmapcount;
  uint reserved[11];
  DDSPixelFormat pf;
  DDSCaps caps;
  uint notused;
};

static const uint FOURCC_DDS = MAKEFOURCC('D', 'D', 'S', ' ');
static const uint FOURCC_DXT1 = MAKEFOURCC('D', 'X', 'T', '1');
static const uint DDSD_WIDTH = 0x00000004U;
static const uint DDSD_HEIGHT = 0x00000002U;
static const uint DDSD_CAPS = 0x00000001U;
static const uint DDSD_PIXELFORMAT = 0x00001000U;
static const uint DDSCAPS_TEXTURE = 0x00001000U;
static const uint DDPF_FOURCC = 0x00000004U;
static const uint DDSD_LINEARSIZE = 0x00080000U;

#endif  // DDS_H


// --- from permutations.h ---
/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PERMUTATIONS_H
#define PERMUTATIONS_H

static void computePermutations(uint permutations[1024]) {
  int indices[16];
  int num = 0;

  // 3 element permutations:

  // first cluster [0,i) is at the start
  for (int m = 0; m < 16; ++m) {
    indices[m] = 0;
  }

  const int imax = 15;

  for (int i = imax; i >= 0; --i) {
    // second cluster [i,j) is half along
    for (int m = i; m < 16; ++m) {
      indices[m] = 2;
    }

    const int jmax = (i == 0) ? 15 : 16;

    for (int j = jmax; j >= i; --j) {
      // last cluster [j,k) is at the end
      if (j < 16) {
        indices[j] = 1;
      }

      uint permutation = 0;

      for (int p = 0; p < 16; p++) {
        permutation |= indices[p] << (p * 2);
        // permutation |= indices[15-p] << (p * 2);
      }

      permutations[num] = permutation;

      num++;
    }
  }

  assert(num == 151);

  for (int i = 0; i < 9; i++) {
    permutations[num] = 0x000AA555;
    num++;
  }

  assert(num == 160);

  // Append 4 element permutations:

  // first cluster [0,i) is at the start
  for (int m = 0; m < 16; ++m) {
    indices[m] = 0;
  }

  for (int i = imax; i >= 0; --i) {
    // second cluster [i,j) is one third along
    for (int m = i; m < 16; ++m) {
      indices[m] = 2;
    }

    const int jmax = (i == 0) ? 15 : 16;

    for (int j = jmax; j >= i; --j) {
      // third cluster [j,k) is two thirds along
      for (int m = j; m < 16; ++m) {
        indices[m] = 3;
      }

      int kmax = (j == 0) ? 15 : 16;

      for (int k = kmax; k >= j; --k) {
        // last cluster [k,n) is at the end
        if (k < 16) {
          indices[k] = 1;
        }

        uint permutation = 0;

        bool hasThree = false;

        for (int p = 0; p < 16; p++) {
          permutation |= indices[p] << (p * 2);
          // permutation |= indices[15-p] << (p * 2);

          if (indices[p] == 3) hasThree = true;
        }

        if (hasThree) {
          permutations[num] = permutation;
          num++;
        }
      }
    }
  }

  assert(num == 975);

  // 1024 - 969 - 7 = 48 extra elements

  // It would be nice to set these extra elements with better values...
  for (int i = 0; i < 49; i++) {
    permutations[num] = 0x00AAFF55;
    num++;
  }

  assert(num == 1024);
}

#endif  // PERMUTATIONS_H
