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
/*
GFC code: A GPU-based compressor for arrays of double-precision
floating-point values.

Copyright (c) 2011-2020, Texas State University. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of Texas State University nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL TEXAS STATE UNIVERSITY BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Authors: Molly A. O'Neil and Martin Burtscher

URL: The latest version of this code is available at
https://userweb.cs.txstate.edu/~burtscher/research/GFC/.

Publication: This work is described in detail in the following paper.
Molly A. O'Neil and Martin Burtscher. Floating-Point Data Compression at 75
Gb/s on a GPU. Proceedings of the Fourth Workshop on General Purpose Processing
Using GPUs, pp. 7:1-7:7. March 2011.
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <chrono>

}

/************************************************************************************/


/************************************************************************************/


/************************************************************************************/



// --- from kernels.h ---
#define ull unsigned long long
#define MAX (64*1024*1024)
#define WARPSIZE 32

/************************************************************************************/

/*
This is the GPU compression kernel, which should be launched using the block count
and warps/block:

Inputs
------
dimensionalityd: dimensionality of trace (from cmd line)
cbufd: ptr to the uncompressed data
cutd: ptr to array of chunk boundaries

Output
------
The compressed data, in dbufd 
Compressed chunk offsets for offset table, in offd
*/

void CompressionKernel(
  const int dimensionalityd,
  const ull * cbufd,
  char * dbufd,
  const int * cutd,
  int * offd)
{
  int offset, code, bcount, tmp, off, beg, end, lane, warp, iindex, lastidx, start, term;
  ull diff, prev;
  int ibufs[32 * (3 * WARPSIZE / 2)]; // shared space for prefix sum

  // index within this warp
  lane = _tid_x & 31;
  // index within shared prefix sum array
  iindex = _tid_x / WARPSIZE * (3 * WARPSIZE / 2) + lane;
  ibufs[iindex] = 0;
  iindex += WARPSIZE / 2;
  lastidx = (_tid_x / WARPSIZE + 1) * (3 * WARPSIZE / 2) - 1;
  // warp id
  warp = (_tid_x + _bid_x * BLOCK_DIM_X) / WARPSIZE;
  // prediction index within previous subchunk
  offset = WARPSIZE - (dimensionalityd - lane % dimensionalityd) - lane;

  // determine start and end of chunk to compress
  start = 0;
  if (warp > 0) start = cutd[warp-1];
  term = cutd[warp];
  off = ((start+1)/2*17);

  prev = 0;
  for (int i = start + lane; i < term; i += WARPSIZE) {
    // calculate delta between value to compress and prediction
    // and negate if negative
    if (cbufd[i] >= prev)
      diff = cbufd[i] - prev;
    else
      diff = prev - cbufd[i];

    // count leading zeros in positive delta
    bcount = 8 - (__clzll(diff) >> 3);
    if (bcount == 2) bcount = 3; // encode 6 lead-zero bytes as 5

    // prefix sum to determine start positions of non-zero delta bytes
    ibufs[iindex] = bcount;
    __threadfence_block();
    ibufs[iindex] += ibufs[iindex-1];
    __threadfence_block();
    ibufs[iindex] += ibufs[iindex-2];
    __threadfence_block();
    ibufs[iindex] += ibufs[iindex-4];
    __threadfence_block();
    ibufs[iindex] += ibufs[iindex-8];
    __threadfence_block();
    ibufs[iindex] += ibufs[iindex-16];
    __threadfence_block();

    // write out non-zero bytes of delta to compressed buffer
    beg = off + (WARPSIZE/2) + ibufs[iindex-1];
    end = beg + bcount;
    for (; beg < end; beg++) {
      dbufd[beg] = diff;
      diff >>= 8;
    }

    if (bcount >= 3) bcount--; // adjust byte count for the dropped encoding
    tmp = ibufs[lastidx];
    code |= bcount;
    ibufs[iindex] = code;
    __threadfence_block();

    // write out half-bytes of sign and leading-zero-byte count (every other thread
    // writes its half-byte and neighbor's half-byte)
    if ((lane & 1) != 0) {
      dbufd[off + (lane >> 1)] = ibufs[iindex-1] | (code << 4);
    }
    off += tmp + (WARPSIZE/2);

    // save prediction value from this subchunk (based on provided dimensionality)
    // for use in next subchunk
    prev = cbufd[i + offset];
  }

  // save final value of off, which is total bytes of compressed output for this chunk
  if (lane == 31) offd[warp] = off;
}
