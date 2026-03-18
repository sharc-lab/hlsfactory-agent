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
   ECL-GC code: ECL-GC is a graph-coloring algorithm with shortcutting. The CUDA
   implementation thereof is quite fast. It operates on graphs stored in binary
   CSR format.

   Copyright (c) 2020, Texas State University. All rights reserved.

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

Authors: Ghadeer Alabandi, Evan Powers, and Martin Burtscher

URL: The latest version of this code is available at
https://userweb.cs.txstate.edu/~burtscher/research/ECL-GC/.

Publication: This work is described in detail in the following paper.
Ghadeer Alabandi, Evan Powers, and Martin Burtscher. Increasing the Parallelism
of Graph Coloring via Shortcutting. Proceedings of the 2020 ACM SIGPLAN
Symposium on Principles and Practice of Parallel Programming, pp. 262-275.
February 2020.
 */

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include "graph.h"

static const int ThreadsPerBlock = 512;
static const unsigned int Warp = 0xffffffff;
static const int WS = 32;  // warp size and bits per int
static const int MSB = 1 << (WS - 1);
static const int Mask = (1 << (WS / 2)) - 1;

// https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key




