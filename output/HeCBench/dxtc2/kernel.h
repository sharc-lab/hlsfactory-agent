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


// --- from helper_math.h ---
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

/*
 *  This file implements common mathematical operations on vector types
 *  (float3, float4 etc.) since these are not provided as standard by CUDA.
 *
 *  The syntax is modeled on the Cg standard library.
 *
 *  This is part of the Helper library includes
 *
 *    Thanks to Linh Hah for additions and fixes.
 */

#ifndef HELPER_MATH_H
#define HELPER_MATH_H

typedef unsigned int uint;
typedef unsigned short ushort;

#ifndef EXIT_WAIVED
#define EXIT_WAIVED 2
#endif

#ifndef __CUDACC__
#include <math.h>

////////////////////////////////////////////////////////////////////////////////
// host implementations of CUDA functions
////////////////////////////////////////////////////////////////////////////////

inline float fminf(float a, float b)
{
    return a < b ? a : b;
}

inline float fmaxf(float a, float b)
{
    return a > b ? a : b;
}

inline int max(int a, int b)
{
    return a > b ? a : b;
}

inline int min(int a, int b)
{
    return a < b ? a : b;
}

inline float rsqrtf(float x)
{
    return 1.0f / sqrtf(x);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// constructors
////////////////////////////////////////////////////////////////////////////////

inline float2 float2(float s)
{
    return float2(s, s);
}
inline float2 float2(float3 a)
{
    return float2(a.x, a.y);
}
inline float2 float2(int2 a)
{
    return float2(float(a.x), float(a.y));
}
inline float2 float2(uint2 a)
{
    return float2(float(a.x), float(a.y));
}

inline int2 int2(int s)
{
    return int2(s, s);
}
inline int2 int2(int3 a)
{
    return int2(a.x, a.y);
}
inline int2 int2(uint2 a)
{
    return int2(int(a.x), int(a.y));
}
inline int2 int2(float2 a)
{
    return int2(int(a.x), int(a.y));
}

inline uint2 make_uint2(uint s)
{
    return make_uint2(s, s);
}
inline uint2 make_uint2(uint3 a)
{
    return make_uint2(a.x, a.y);
}
inline uint2 make_uint2(int2 a)
{
    return make_uint2(uint(a.x), uint(a.y));
}

inline float3 float3(float s)
{
    return float3(s, s, s);
}
inline float3 float3(float2 a)
{
    return float3(a.x, a.y, 0.0f);
}
inline float3 float3(float2 a, float s)
{
    return float3(a.x, a.y, s);
}
inline float3 float3(float4 a)
{
    return float3(a.x, a.y, a.z);
}
inline float3 float3(int3 a)
{
    return float3(float(a.x), float(a.y), float(a.z));
}
inline float3 float3(uint3 a)
{
    return float3(float(a.x), float(a.y), float(a.z));
}

inline int3 int3(int s)
{
    return int3(s, s, s);
}
inline int3 int3(int2 a)
{
    return int3(a.x, a.y, 0);
}
inline int3 int3(int2 a, int s)
{
    return int3(a.x, a.y, s);
}
inline int3 int3(uint3 a)
{
    return int3(int(a.x), int(a.y), int(a.z));
}
inline int3 int3(float3 a)
{
    return int3(int(a.x), int(a.y), int(a.z));
}

inline uint3 make_uint3(uint s)
{
    return make_uint3(s, s, s);
}
inline uint3 make_uint3(uint2 a)
{
    return make_uint3(a.x, a.y, 0);
}
inline uint3 make_uint3(uint2 a, uint s)
{
    return make_uint3(a.x, a.y, s);
}
inline uint3 make_uint3(uint4 a)
{
    return make_uint3(a.x, a.y, a.z);
}
inline uint3 make_uint3(int3 a)
{
    return make_uint3(uint(a.x), uint(a.y), uint(a.z));
}

inline float4 float4(float s)
{
    return float4(s, s, s, s);
}
inline float4 float4(float3 a)
{
    return float4(a.x, a.y, a.z, 0.0f);
}
inline float4 float4(float3 a, float w)
{
    return float4(a.x, a.y, a.z, w);
}
inline float4 float4(int4 a)
{
    return float4(float(a.x), float(a.y), float(a.z), float(a.w));
}
inline float4 float4(uint4 a)
{
    return float4(float(a.x), float(a.y), float(a.z), float(a.w));
}

inline int4 make_int4(int s)
{
    return make_int4(s, s, s, s);
}
inline int4 make_int4(int3 a)
{
    return make_int4(a.x, a.y, a.z, 0);
}
inline int4 make_int4(int3 a, int w)
{
    return make_int4(a.x, a.y, a.z, w);
}
inline int4 make_int4(uint4 a)
{
    return make_int4(int(a.x), int(a.y), int(a.z), int(a.w));
}
inline int4 make_int4(float4 a)
{
    return make_int4(int(a.x), int(a.y), int(a.z), int(a.w));
}

inline uint4 make_uint4(uint s)
{
    return make_uint4(s, s, s, s);
}
inline uint4 make_uint4(uint3 a)
{
    return make_uint4(a.x, a.y, a.z, 0);
}
inline uint4 make_uint4(uint3 a, uint w)
{
    return make_uint4(a.x, a.y, a.z, w);
}
inline uint4 make_uint4(int4 a)
{
    return make_uint4(uint(a.x), uint(a.y), uint(a.z), uint(a.w));
}

////////////////////////////////////////////////////////////////////////////////
// negate
////////////////////////////////////////////////////////////////////////////////

inline float2 operator-(float2 &a)
{
    return float2(-a.x, -a.y);
}
inline int2 operator-(int2 &a)
{
    return int2(-a.x, -a.y);
}
inline float3 operator-(float3 &a)
{
    return float3(-a.x, -a.y, -a.z);
}
inline int3 operator-(int3 &a)
{
    return int3(-a.x, -a.y, -a.z);
}
inline float4 operator-(float4 &a)
{
    return float4(-a.x, -a.y, -a.z, -a.w);
}
inline int4 operator-(int4 &a)
{
    return make_int4(-a.x, -a.y, -a.z, -a.w);
}

////////////////////////////////////////////////////////////////////////////////
// addition
////////////////////////////////////////////////////////////////////////////////

inline float2 operator+(float2 a, float2 b)
{
    return float2(a.x + b.x, a.y + b.y);
}
inline void operator+=(float2 &a, float2 b)
{
    a.x += b.x;
    a.y += b.y;
}
inline float2 operator+(float2 a, float b)
{
    return float2(a.x + b, a.y + b);
}
inline float2 operator+(float b, float2 a)
{
    return float2(a.x + b, a.y + b);
}
inline void operator+=(float2 &a, float b)
{
    a.x += b;
    a.y += b;
}

inline int2 operator+(int2 a, int2 b)
{
    return int2(a.x + b.x, a.y + b.y);
}
inline void operator+=(int2 &a, int2 b)
{
    a.x += b.x;
    a.y += b.y;
}
inline int2 operator+(int2 a, int b)
{
    return int2(a.x + b, a.y + b);
}
inline int2 operator+(int b, int2 a)
{
    return int2(a.x + b, a.y + b);
}
inline void operator+=(int2 &a, int b)
{
    a.x += b;
    a.y += b;
}

inline uint2 operator+(uint2 a, uint2 b)
{
    return make_uint2(a.x + b.x, a.y + b.y);
}
inline void operator+=(uint2 &a, uint2 b)
{
    a.x += b.x;
    a.y += b.y;
}
inline uint2 operator+(uint2 a, uint b)
{
    return make_uint2(a.x + b, a.y + b);
}
inline uint2 operator+(uint b, uint2 a)
{
    return make_uint2(a.x + b, a.y + b);
}
inline void operator+=(uint2 &a, uint b)
{
    a.x += b;
    a.y += b;
}

inline float3 operator+(float3 a, float3 b)
{
    return float3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline void operator+=(float3 &a, float3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
}
inline float3 operator+(float3 a, float b)
{
    return float3(a.x + b, a.y + b, a.z + b);
}
inline void operator+=(float3 &a, float b)
{
    a.x += b;
    a.y += b;
    a.z += b;
}

inline int3 operator+(int3 a, int3 b)
{
    return int3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline void operator+=(int3 &a, int3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
}
inline int3 operator+(int3 a, int b)
{
    return int3(a.x + b, a.y + b, a.z + b);
}
inline void operator+=(int3 &a, int b)
{
    a.x += b;
    a.y += b;
    a.z += b;
}

inline uint3 operator+(uint3 a, uint3 b)
{
    return make_uint3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline void operator+=(uint3 &a, uint3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
}
inline uint3 operator+(uint3 a, uint b)
{
    return make_uint3(a.x + b, a.y + b, a.z + b);
}
inline void operator+=(uint3 &a, uint b)
{
    a.x += b;
    a.y += b;
    a.z += b;
}

inline int3 operator+(int b, int3 a)
{
    return int3(a.x + b, a.y + b, a.z + b);
}
inline uint3 operator+(uint b, uint3 a)
{
    return make_uint3(a.x + b, a.y + b, a.z + b);
}
inline float3 operator+(float b, float3 a)
{
    return float3(a.x + b, a.y + b, a.z + b);
}

inline float4 operator+(float4 a, float4 b)
{
    return float4(a.x + b.x, a.y + b.y, a.z + b.z,  a.w + b.w);
}
inline void operator+=(float4 &a, float4 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
}
inline float4 operator+(float4 a, float b)
{
    return float4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline float4 operator+(float b, float4 a)
{
    return float4(a.x + b, a.y + b, a.z + b, a.w + b);
}
inline void operator+=(float4 &a, float b)
{
    a.x += b;
    a.y += b;
    a.z += b;
    a.w += b;
}

inline int4 operator+(int4 a, int4 b)
{
    return make_int4(a.x + b.x, a.y + b.y, a.z + b.z,  a.w + b.w);
}
inline void operator+=(int4 &a, int4 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
}
inline int4 operator+(int4 a, int b)
{
    return make_int4(a.x + b, a.y + b, a.z + b,  a.w + b);
}
inline int4 operator+(int b, int4 a)
{
    return make_int4(a.x + b, a.y + b, a.z + b,  a.w + b);
}
inline void operator+=(int4 &a, int b)
{
    a.x += b;
    a.y += b;
    a.z += b;
    a.w += b;
}

inline uint4 operator+(uint4 a, uint4 b)
{
    return make_uint4(a.x + b.x, a.y + b.y, a.z + b.z,  a.w + b.w);
}
inline void operator+=(uint4 &a, uint4 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
}
inline uint4 operator+(uint4 a, uint b)
{
    return make_uint4(a.x + b, a.y + b, a.z + b,  a.w + b);
}
inline uint4 operator+(uint b, uint4 a)
{
    return make_uint4(a.x + b, a.y + b, a.z + b,  a.w + b);
}
inline void operator+=(uint4 &a, uint b)
{
    a.x += b;
    a.y += b;
    a.z += b;
    a.w += b;
}

////////////////////////////////////////////////////////////////////////////////
// subtract
////////////////////////////////////////////////////////////////////////////////

inline float2 operator-(float2 a, float2 b)
{
    return float2(a.x - b.x, a.y - b.y);
}
inline void operator-=(float2 &a, float2 b)
{
    a.x -= b.x;
    a.y -= b.y;
}
inline float2 operator-(float2 a, float b)
{
    return float2(a.x - b, a.y - b);
}
inline float2 operator-(float b, float2 a)
{
    return float2(b - a.x, b - a.y);
}
inline void operator-=(float2 &a, float b)
{
    a.x -= b;
    a.y -= b;
}

inline int2 operator-(int2 a, int2 b)
{
    return int2(a.x - b.x, a.y - b.y);
}
inline void operator-=(int2 &a, int2 b)
{
    a.x -= b.x;
    a.y -= b.y;
}
inline int2 operator-(int2 a, int b)
{
    return int2(a.x - b, a.y - b);
}
inline int2 operator-(int b, int2 a)
{
    return int2(b - a.x, b - a.y);
}
inline void operator-=(int2 &a, int b)
{
    a.x -= b;
    a.y -= b;
}

inline uint2 operator-(uint2 a, uint2 b)
{
    return make_uint2(a.x - b.x, a.y - b.y);
}
inline void operator-=(uint2 &a, uint2 b)
{
    a.x -= b.x;
    a.y -= b.y;
}
inline uint2 operator-(uint2 a, uint b)
{
    return make_uint2(a.x - b, a.y - b);
}
inline uint2 operator-(uint b, uint2 a)
{
    return make_uint2(b - a.x, b - a.y);
}
inline void operator-=(uint2 &a, uint b)
{
    a.x -= b;
    a.y -= b;
}

inline float3 operator-(float3 a, float3 b)
{
    return float3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline void operator-=(float3 &a, float3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
}
inline float3 operator-(float3 a, float b)
{
    return float3(a.x - b, a.y - b, a.z - b);
}
inline float3 operator-(float b, float3 a)
{
    return float3(b - a.x, b - a.y, b - a.z);
}
inline void operator-=(float3 &a, float b)
{
    a.x -= b;
    a.y -= b;
    a.z -= b;
}

inline int3 operator-(int3 a, int3 b)
{
    return int3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline void operator-=(int3 &a, int3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
}
inline int3 operator-(int3 a, int b)
{
    return int3(a.x - b, a.y - b, a.z - b);
}
inline int3 operator-(int b, int3 a)
{
    return int3(b - a.x, b - a.y, b - a.z);
}
inline void operator-=(int3 &a, int b)
{
    a.x -= b;
    a.y -= b;
    a.z -= b;
}

inline uint3 operator-(uint3 a, uint3 b)
{
    return make_uint3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline void operator-=(uint3 &a, uint3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
}
inline uint3 operator-(uint3 a, uint b)
{
    return make_uint3(a.x - b, a.y - b, a.z - b);
}
inline uint3 operator-(uint b, uint3 a)
{
    return make_uint3(b - a.x, b - a.y, b - a.z);
}
inline void operator-=(uint3 &a, uint b)
{
    a.x -= b;
    a.y -= b;
    a.z -= b;
}

inline float4 operator-(float4 a, float4 b)
{
    return float4(a.x - b.x, a.y - b.y, a.z - b.z,  a.w - b.w);
}
inline void operator-=(float4 &a, float4 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
}
inline float4 operator-(float4 a, float b)
{
    return float4(a.x - b, a.y - b, a.z - b,  a.w - b);
}
inline void operator-=(float4 &a, float b)
{
    a.x -= b;
    a.y -= b;
    a.z -= b;
    a.w -= b;
}

inline int4 operator-(int4 a, int4 b)
{
    return make_int4(a.x - b.x, a.y - b.y, a.z - b.z,  a.w - b.w);
}
inline void operator-=(int4 &a, int4 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
}
inline int4 operator-(int4 a, int b)
{
    return make_int4(a.x - b, a.y - b, a.z - b,  a.w - b);
}
inline int4 operator-(int b, int4 a)
{
    return make_int4(b - a.x, b - a.y, b - a.z, b - a.w);
}
inline void operator-=(int4 &a, int b)
{
    a.x -= b;
    a.y -= b;
    a.z -= b;
    a.w -= b;
}

inline uint4 operator-(uint4 a, uint4 b)
{
    return make_uint4(a.x - b.x, a.y - b.y, a.z - b.z,  a.w - b.w);
}
inline void operator-=(uint4 &a, uint4 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
}
inline uint4 operator-(uint4 a, uint b)
{
    return make_uint4(a.x - b, a.y - b, a.z - b,  a.w - b);
}
inline uint4 operator-(uint b, uint4 a)
{
    return make_uint4(b - a.x, b - a.y, b - a.z, b - a.w);
}
inline void operator-=(uint4 &a, uint b)
{
    a.x -= b;
    a.y -= b;
    a.z -= b;
    a.w -= b;
}

////////////////////////////////////////////////////////////////////////////////
// multiply
////////////////////////////////////////////////////////////////////////////////

inline float2 operator*(float2 a, float2 b)
{
    return float2(a.x * b.x, a.y * b.y);
}
inline void operator*=(float2 &a, float2 b)
{
    a.x *= b.x;
    a.y *= b.y;
}
inline float2 operator*(float2 a, float b)
{
    return float2(a.x * b, a.y * b);
}
inline float2 operator*(float b, float2 a)
{
    return float2(b * a.x, b * a.y);
}
inline void operator*=(float2 &a, float b)
{
    a.x *= b;
    a.y *= b;
}

inline int2 operator*(int2 a, int2 b)
{
    return int2(a.x * b.x, a.y * b.y);
}
inline void operator*=(int2 &a, int2 b)
{
    a.x *= b.x;
    a.y *= b.y;
}
inline int2 operator*(int2 a, int b)
{
    return int2(a.x * b, a.y * b);
}
inline int2 operator*(int b, int2 a)
{
    return int2(b * a.x, b * a.y);
}
inline void operator*=(int2 &a, int b)
{
    a.x *= b;
    a.y *= b;
}

inline uint2 operator*(uint2 a, uint2 b)
{
    return make_uint2(a.x * b.x, a.y * b.y);
}
inline void operator*=(uint2 &a, uint2 b)
{
    a.x *= b.x;
    a.y *= b.y;
}
inline uint2 operator*(uint2 a, uint b)
{
    return make_uint2(a.x * b, a.y * b);
}
inline uint2 operator*(uint b, uint2 a)
{
    return make_uint2(b * a.x, b * a.y);
}
inline void operator*=(uint2 &a, uint b)
{
    a.x *= b;
    a.y *= b;
}

inline float3 operator*(float3 a, float3 b)
{
    return float3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline void operator*=(float3 &a, float3 b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
}
inline float3 operator*(float3 a, float b)
{
    return float3(a.x * b, a.y * b, a.z * b);
}
inline float3 operator*(float b, float3 a)
{
    return float3(b * a.x, b * a.y, b * a.z);
}
inline void operator*=(float3 &a, float b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
}

inline int3 operator*(int3 a, int3 b)
{
    return int3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline void operator*=(int3 &a, int3 b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
}
inline int3 operator*(int3 a, int b)
{
    return int3(a.x * b, a.y * b, a.z * b);
}
inline int3 operator*(int b, int3 a)
{
    return int3(b * a.x, b * a.y, b * a.z);
}
inline void operator*=(int3 &a, int b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
}

inline uint3 operator*(uint3 a, uint3 b)
{
    return make_uint3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline void operator*=(uint3 &a, uint3 b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
}
inline uint3 operator*(uint3 a, uint b)
{
    return make_uint3(a.x * b, a.y * b, a.z * b);
}
inline uint3 operator*(uint b, uint3 a)
{
    return make_uint3(b * a.x, b * a.y, b * a.z);
}
inline void operator*=(uint3 &a, uint b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
}

inline float4 operator*(float4 a, float4 b)
{
    return float4(a.x * b.x, a.y * b.y, a.z * b.z,  a.w * b.w);
}
inline void operator*=(float4 &a, float4 b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    a.w *= b.w;
}
inline float4 operator*(float4 a, float b)
{
    return float4(a.x * b, a.y * b, a.z * b,  a.w * b);
}
inline float4 operator*(float b, float4 a)
{
    return float4(b * a.x, b * a.y, b * a.z, b * a.w);
}
inline void operator*=(float4 &a, float b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
}

inline int4 operator*(int4 a, int4 b)
{
    return make_int4(a.x * b.x, a.y * b.y, a.z * b.z,  a.w * b.w);
}
inline void operator*=(int4 &a, int4 b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    a.w *= b.w;
}
inline int4 operator*(int4 a, int b)
{
    return make_int4(a.x * b, a.y * b, a.z * b,  a.w * b);
}
inline int4 operator*(int b, int4 a)
{
    return make_int4(b * a.x, b * a.y, b * a.z, b * a.w);
}
inline void operator*=(int4 &a, int b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
}

inline uint4 operator*(uint4 a, uint4 b)
{
    return make_uint4(a.x * b.x, a.y * b.y, a.z * b.z,  a.w * b.w);
}
inline void operator*=(uint4 &a, uint4 b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    a.w *= b.w;
}
inline uint4 operator*(uint4 a, uint b)
{
    return make_uint4(a.x * b, a.y * b, a.z * b,  a.w * b);
}
inline uint4 operator*(uint b, uint4 a)
{
    return make_uint4(b * a.x, b * a.y, b * a.z, b * a.w);
}
inline void operator*=(uint4 &a, uint b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
}

////////////////////////////////////////////////////////////////////////////////
// divide
////////////////////////////////////////////////////////////////////////////////

inline float2 operator/(float2 a, float2 b)
{
    return float2(a.x / b.x, a.y / b.y);
}
inline void operator/=(float2 &a, float2 b)
{
    a.x /= b.x;
    a.y /= b.y;
}
inline float2 operator/(float2 a, float b)
{
    return float2(a.x / b, a.y / b);
}
inline void operator/=(float2 &a, float b)
{
    a.x /= b;
    a.y /= b;
}
inline float2 operator/(float b, float2 a)
{
    return float2(b / a.x, b / a.y);
}

inline float3 operator/(float3 a, float3 b)
{
    return float3(a.x / b.x, a.y / b.y, a.z / b.z);
}
inline void operator/=(float3 &a, float3 b)
{
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
}
inline float3 operator/(float3 a, float b)
{
    return float3(a.x / b, a.y / b, a.z / b);
}
inline void operator/=(float3 &a, float b)
{
    a.x /= b;
    a.y /= b;
    a.z /= b;
}
inline float3 operator/(float b, float3 a)
{
    return float3(b / a.x, b / a.y, b / a.z);
}

inline float4 operator/(float4 a, float4 b)
{
    return float4(a.x / b.x, a.y / b.y, a.z / b.z,  a.w / b.w);
}
inline void operator/=(float4 &a, float4 b)
{
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
    a.w /= b.w;
}
inline float4 operator/(float4 a, float b)
{
    return float4(a.x / b, a.y / b, a.z / b,  a.w / b);
}
inline void operator/=(float4 &a, float b)
{
    a.x /= b;
    a.y /= b;
    a.z /= b;
    a.w /= b;
}
inline float4 operator/(float b, float4 a)
{
    return float4(b / a.x, b / a.y, b / a.z, b / a.w);
}

////////////////////////////////////////////////////////////////////////////////
// min
////////////////////////////////////////////////////////////////////////////////

inline  float2 fminf(float2 a, float2 b)
{
    return float2(fminf(a.x,b.x), fminf(a.y,b.y));
}
inline float3 fminf(float3 a, float3 b)
{
    return float3(fminf(a.x,b.x), fminf(a.y,b.y), fminf(a.z,b.z));
}
inline  float4 fminf(float4 a, float4 b)
{
    return float4(fminf(a.x,b.x), fminf(a.y,b.y), fminf(a.z,b.z), fminf(a.w,b.w));
}

inline int2 min(int2 a, int2 b)
{
    return int2(min(a.x,b.x), min(a.y,b.y));
}
inline int3 min(int3 a, int3 b)
{
    return int3(min(a.x,b.x), min(a.y,b.y), min(a.z,b.z));
}
inline int4 min(int4 a, int4 b)
{
    return make_int4(min(a.x,b.x), min(a.y,b.y), min(a.z,b.z), min(a.w,b.w));
}

inline uint2 min(uint2 a, uint2 b)
{
    return make_uint2(min(a.x,b.x), min(a.y,b.y));
}
inline uint3 min(uint3 a, uint3 b)
{
    return make_uint3(min(a.x,b.x), min(a.y,b.y), min(a.z,b.z));
}
inline uint4 min(uint4 a, uint4 b)
{
    return make_uint4(min(a.x,b.x), min(a.y,b.y), min(a.z,b.z), min(a.w,b.w));
}

////////////////////////////////////////////////////////////////////////////////
// max
////////////////////////////////////////////////////////////////////////////////

inline float2 fmaxf(float2 a, float2 b)
{
    return float2(fmaxf(a.x,b.x), fmaxf(a.y,b.y));
}
inline float3 fmaxf(float3 a, float3 b)
{
    return float3(fmaxf(a.x,b.x), fmaxf(a.y,b.y), fmaxf(a.z,b.z));
}
inline float4 fmaxf(float4 a, float4 b)
{
    return float4(fmaxf(a.x,b.x), fmaxf(a.y,b.y), fmaxf(a.z,b.z), fmaxf(a.w,b.w));
}

inline int2 max(int2 a, int2 b)
{
    return int2(max(a.x,b.x), max(a.y,b.y));
}
inline int3 max(int3 a, int3 b)
{
    return int3(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z));
}
inline int4 max(int4 a, int4 b)
{
    return make_int4(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z), max(a.w,b.w));
}

inline uint2 max(uint2 a, uint2 b)
{
    return make_uint2(max(a.x,b.x), max(a.y,b.y));
}
inline uint3 max(uint3 a, uint3 b)
{
    return make_uint3(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z));
}
inline uint4 max(uint4 a, uint4 b)
{
    return make_uint4(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z), max(a.w,b.w));
}

////////////////////////////////////////////////////////////////////////////////
// lerp
// - linear interpolation between a and b, based on value t in [0, 1] range
////////////////////////////////////////////////////////////////////////////////

inline float lerp(float a, float b, float t)
{
    return a + t*(b-a);
}
inline float2 lerp(float2 a, float2 b, float t)
{
    return a + t*(b-a);
}
inline float3 lerp(float3 a, float3 b, float t)
{
    return a + t*(b-a);
}
inline float4 lerp(float4 a, float4 b, float t)
{
    return a + t*(b-a);
}

////////////////////////////////////////////////////////////////////////////////
// clamp
// - clamp the value v to be in the range [a, b]
////////////////////////////////////////////////////////////////////////////////

inline float clamp(float f, float a, float b)
{
    return fmaxf(a, fminf(f, b));
}
inline int clamp(int f, int a, int b)
{
    return max(a, min(f, b));
}
inline uint clamp(uint f, uint a, uint b)
{
    return max(a, min(f, b));
}

inline float2 clamp(float2 v, float a, float b)
{
    return float2(clamp(v.x, a, b), clamp(v.y, a, b));
}
inline float2 clamp(float2 v, float2 a, float2 b)
{
    return float2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y));
}
inline float3 clamp(float3 v, float a, float b)
{
    return float3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b));
}
inline float3 clamp(float3 v, float3 a, float3 b)
{
    return float3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z));
}
inline float4 clamp(float4 v, float a, float b)
{
    return float4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b), clamp(v.w, a, b));
}
inline float4 clamp(float4 v, float4 a, float4 b)
{
    return float4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w));
}

inline int2 clamp(int2 v, int a, int b)
{
    return int2(clamp(v.x, a, b), clamp(v.y, a, b));
}
inline int2 clamp(int2 v, int2 a, int2 b)
{
    return int2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y));
}
inline int3 clamp(int3 v, int a, int b)
{
    return int3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b));
}
inline int3 clamp(int3 v, int3 a, int3 b)
{
    return int3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z));
}
inline int4 clamp(int4 v, int a, int b)
{
    return make_int4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b), clamp(v.w, a, b));
}
inline int4 clamp(int4 v, int4 a, int4 b)
{
    return make_int4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w));
}

inline uint2 clamp(uint2 v, uint a, uint b)
{
    return make_uint2(clamp(v.x, a, b), clamp(v.y, a, b));
}
inline uint2 clamp(uint2 v, uint2 a, uint2 b)
{
    return make_uint2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y));
}
inline uint3 clamp(uint3 v, uint a, uint b)
{
    return make_uint3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b));
}
inline uint3 clamp(uint3 v, uint3 a, uint3 b)
{
    return make_uint3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z));
}
inline uint4 clamp(uint4 v, uint a, uint b)
{
    return make_uint4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b), clamp(v.w, a, b));
}
inline uint4 clamp(uint4 v, uint4 a, uint4 b)
{
    return make_uint4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// dot product
////////////////////////////////////////////////////////////////////////////////

inline float dot(float2 a, float2 b)
{
    return a.x * b.x + a.y * b.y;
}
inline float dot(float3 a, float3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline float dot(float4 a, float4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline int dot(int2 a, int2 b)
{
    return a.x * b.x + a.y * b.y;
}
inline int dot(int3 a, int3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline int dot(int4 a, int4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline uint dot(uint2 a, uint2 b)
{
    return a.x * b.x + a.y * b.y;
}
inline uint dot(uint3 a, uint3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline uint dot(uint4 a, uint4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

////////////////////////////////////////////////////////////////////////////////
// length
////////////////////////////////////////////////////////////////////////////////

inline float length(float2 v)
{
    return sqrtf(dot(v, v));
}
inline float length(float3 v)
{
    return sqrtf(dot(v, v));
}
inline float length(float4 v)
{
    return sqrtf(dot(v, v));
}

////////////////////////////////////////////////////////////////////////////////
// normalize
////////////////////////////////////////////////////////////////////////////////

inline float2 normalize(float2 v)
{
    float invLen = rsqrtf(dot(v, v));
    return v * invLen;
}
inline float3 normalize(float3 v)
{
    float invLen = rsqrtf(dot(v, v));
    return v * invLen;
}
inline float4 normalize(float4 v)
{
    float invLen = rsqrtf(dot(v, v));
    return v * invLen;
}

////////////////////////////////////////////////////////////////////////////////
// floor
////////////////////////////////////////////////////////////////////////////////

inline float2 floorf(float2 v)
{
    return float2(floorf(v.x), floorf(v.y));
}
inline float3 floorf(float3 v)
{
    return float3(floorf(v.x), floorf(v.y), floorf(v.z));
}
inline float4 floorf(float4 v)
{
    return float4(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// frac - returns the fractional portion of a scalar or each vector component
////////////////////////////////////////////////////////////////////////////////

inline float fracf(float v)
{
    return v - floorf(v);
}
inline float2 fracf(float2 v)
{
    return float2(fracf(v.x), fracf(v.y));
}
inline float3 fracf(float3 v)
{
    return float3(fracf(v.x), fracf(v.y), fracf(v.z));
}
inline float4 fracf(float4 v)
{
    return float4(fracf(v.x), fracf(v.y), fracf(v.z), fracf(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// fmod
////////////////////////////////////////////////////////////////////////////////

inline float2 fmodf(float2 a, float2 b)
{
    return float2(fmodf(a.x, b.x), fmodf(a.y, b.y));
}
inline float3 fmodf(float3 a, float3 b)
{
    return float3(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z));
}
inline float4 fmodf(float4 a, float4 b)
{
    return float4(fmodf(a.x, b.x), fmodf(a.y, b.y), fmodf(a.z, b.z), fmodf(a.w, b.w));
}

////////////////////////////////////////////////////////////////////////////////
// absolute value
////////////////////////////////////////////////////////////////////////////////

inline float2 fabs(float2 v)
{
    return float2(fabs(v.x), fabs(v.y));
}
inline float3 fabs(float3 v)
{
    return float3(fabs(v.x), fabs(v.y), fabs(v.z));
}
inline float4 fabs(float4 v)
{
    return float4(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w));
}

inline int2 abs(int2 v)
{
    return int2(abs(v.x), abs(v.y));
}
inline int3 abs(int3 v)
{
    return int3(abs(v.x), abs(v.y), abs(v.z));
}
inline int4 abs(int4 v)
{
    return make_int4(abs(v.x), abs(v.y), abs(v.z), abs(v.w));
}

////////////////////////////////////////////////////////////////////////////////
// reflect
// - returns reflection of incident ray I around surface normal N
// - N should be normalized, reflected vector's length is equal to length of I
////////////////////////////////////////////////////////////////////////////////

inline float3 reflect(float3 i, float3 n)
{
    return i - 2.0f * n * dot(n,i);
}

////////////////////////////////////////////////////////////////////////////////
// cross product
////////////////////////////////////////////////////////////////////////////////

inline float3 cross(float3 a, float3 b)
{
    return float3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

////////////////////////////////////////////////////////////////////////////////
// smoothstep
// - returns 0 if x < a
// - returns 1 if x > b
// - otherwise returns smooth interpolation between 0 and 1 based on x
////////////////////////////////////////////////////////////////////////////////

inline float smoothstep(float a, float b, float x)
{
    float y = clamp((x - a) / (b - a), 0.0f, 1.0f);
    return (y*y*(3.0f - (2.0f*y)));
}
inline float2 smoothstep(float2 a, float2 b, float2 x)
{
    float2 y = clamp((x - a) / (b - a), 0.0f, 1.0f);
    return (y*y*(float2(3.0f) - (float2(2.0f)*y)));
}
inline float3 smoothstep(float3 a, float3 b, float3 x)
{
    float3 y = clamp((x - a) / (b - a), 0.0f, 1.0f);
    return (y*y*(float3(3.0f) - (float3(2.0f)*y)));
}
inline float4 smoothstep(float4 a, float4 b, float4 x)
{
    float4 y = clamp((x - a) / (b - a), 0.0f, 1.0f);
    return (y*y*(float4(3.0f) - (float4(2.0f)*y)));
}

#endif


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
