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
   ECL-BH v4.5: Simulation of the gravitational forces in a star cluster using
   the Barnes-Hut n-body algorithm.

   Copyright (c) 2010-2020 Texas State University. All rights reserved.

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

Authors: Martin Burtscher and Sahar Azimi

URL: The latest version of this code is available at
https://userweb.cs.txstate.edu/~burtscher/research/ECL-BH/.

Publication: This work is described in detail in the following paper.
Martin Burtscher and Keshav Pingali. An Efficient CUDA Implementation of the
Tree-based Barnes Hut n-Body Algorithm. Chapter 6 in GPU Computing Gems
Emerald Edition, pp. 75-92. January 2011.
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <hip/hip_runtime.h>

// threads per block
#define THREADS1 256  // must be a power of 2
#define THREADS2 256
#define THREADS3 256 
#define THREADS4 256
#define THREADS5 256
#define THREADS6 256

// block count = factor * #SMs
#define FACTOR1 2
#define FACTOR2 2
#define FACTOR3 1  // must all be resident at the same time
#define FACTOR4 1  // must all be resident at the same time
#define FACTOR5 2
#define FACTOR6 2

#define WARPSIZE 32
#define MAXDEPTH 32

//
// compute center and radius
//

//
// build tree
//



//
// compute center of mass
//


//
// sort bodies
//

//
// compute force
//

//
// advance bodies
//


// random number generator (https://github.com/staceyson/splash2/blob/master/codes/apps/barnes/util.C)

static int randx = 7;

