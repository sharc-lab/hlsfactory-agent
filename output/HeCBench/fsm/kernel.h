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
   FSM_GA is a GPU-accelerated implementation of a genetic algorithm
   (GA) for finding well-performing finite-state machines (FSM) for predicting
   binary sequences.

   Copyright (c) 2013, Texas State University. All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted for academic, research, experimental, or personal use provided
   that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
 this list of conditions, and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions, and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 * Neither the name of Texas State University nor the names of its
 contributors may be used to endorse or promote products derived from this
 software without specific prior written permission.

 For all other uses, please contact the Office for Commercialization and Industry
 Relations at Texas State University <http://www.txstate.edu/ocir/>.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Authors: Martin Burtscher
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>



// --- from kernels.h ---
unsigned int LCG_random(unsigned int * seed) {
  const unsigned int m = 2147483648;
  const unsigned int a = 26757677;
  const unsigned int c = 1;
  *seed = (a * (*seed) + c) % m;
  return *seed;
}

void LCG_random_init(unsigned int * seed) {
  const unsigned int m = 2147483648;
  const unsigned int a = 26757677;
  const unsigned int c = 1;
  *seed = (a * (*seed) + c) % m;
}

void FSMKernel(
  const int length,
  const unsigned short * data,
  int * best,
  unsigned int * rndstate,
  unsigned char * bfsm,
  unsigned char * same,
  int * smax,
  int * sbest,
  int * oldmax)
{
  int i, d, pc, s, bit, id, misses, rnd;
  unsigned long long myresult, current;
  unsigned char *fsm, state[TABSIZE];
  unsigned char next[FSMSIZE * 2 * POPSIZE];

  fsm = &next[_tid_x * (FSMSIZE * 2)];

  if (_tid_x == 0) {
    oldmax[_bid_x] = 0;
    same[_bid_x] = 0;
  }

  id = _tid_x + _bid_x * BLOCK_DIM_X;
  rndstate[id] = SEED ^ id;
  LCG_random_init(&rndstate[id]);

  // initial population
  for (i = 0; i < FSMSIZE * 2; i++) {
    fsm[i] = LCG_random(rndstate+id) & (FSMSIZE - 1);
  }

  // run generations until cutoff times no improvement
  do {
    // reset miss counter and initial state
    memset(state, 0, TABSIZE);
    misses = 0;

    // evaluate FSM
#pragma unroll
    for (i = 0; i < length; i++) {
      d = (int)data[i];
      pc = (d >> 1) & (TABSIZE - 1);
      bit = d & 1;
      s = (int)state[pc];
      misses += bit ^ (s & 1);
      state[pc] = fsm[s + s + bit];
    }
    for (; i < length; i++) {
      d = (int)data[i];
      pc = (d >> 1) & (TABSIZE - 1);
      bit = d & 1;
      s = (int)state[pc];
      misses += bit ^ (s & 1);
      state[pc] = fsm[s + s + bit];
    }

    // determine best FSM
    if (_tid_x == 0) {
      (best[2] += 1);  // increment generation count
      smax[_bid_x] = 0;
      sbest[_bid_x] = 0;
    }
    atomicMax(&smax[_bid_x], length - misses);
    if (length - misses == smax[_bid_x]) atomicMax(&sbest[_bid_x], _tid_x);
    bit = 0;
    if (sbest[_bid_x] == _tid_x) {
      // check if there was an improvement
      same[_bid_x]++;
      if (oldmax[_bid_x] < smax[_bid_x]) {
        oldmax[_bid_x] = smax[_bid_x];
        same[_bid_x] = 0;
      }
    } else {
      // select 1/8 of threads for mutation (best FSM does crossover)
      if ((LCG_random(rndstate+id) & 7) == 0) bit = 1;
    }

    if (bit) {
      // mutate best FSM by flipping random bits with 1/4th probability
      for (i = 0; i < FSMSIZE * 2; i++) {
        rnd = LCG_random(rndstate+id) & LCG_random(rndstate+id);
        fsm[i] = (next[i + sbest[_bid_x] * FSMSIZE * 2] ^ rnd) & (FSMSIZE - 1);
      }
    } else {
      // crossover best FSM with random FSMs using 3/4 of bits from best FSM
      for (i = 0; i < FSMSIZE * 2; i++) {
        rnd = LCG_random(rndstate+id) & LCG_random(rndstate+id);
        fsm[i] = (fsm[i] & rnd) | (next[i + sbest[_bid_x] * FSMSIZE * 2] & ~rnd);
      }
    }
  } while (same[_bid_x] < CUTOFF);  // end of loop over generations

  // record best result of this block
  if (sbest[_bid_x] == _tid_x) {
    id = _bid_x;
    myresult = length - misses;
    myresult = (myresult << 32) + id;
    current = *((unsigned long long *)best);
    while (myresult > current) {
      atomicCAS((unsigned long long *)best, current, myresult);
      current = *((unsigned long long *)best);
    }
    for (i = 0; i < FSMSIZE * 2; i++) {
      bfsm[id * (FSMSIZE*2) + i] = fsm[i];
    }
  }
}

void MaxKernel(
  int * best, 
  const unsigned char * bfsm)
{
  // copy best FSM state assignment over
  int id = best[0];
  for (int i = 0; i < FSMSIZE * 2; i++) {
    best[i + 3] = bfsm[id * (FSMSIZE*2) + i];
  }
}



// --- from parameters.h ---
// repeat the kernel exeuction
#define REPEAT  10

// inital seed for random state
#define SEED    1234

// the size of the FSM
#define FSMSIZE 8

// the number of entries per state array
#define TABSIZE 32768

// the population count, which determines the number of blocks
#define POPCNT  1024

// the population size, which determines the number of threads per block,
#define POPSIZE 256

// 
#define CUTOFF  1

