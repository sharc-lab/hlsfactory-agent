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
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <random>
#include <chrono>

// transpose



// --- from kernels.h ---
void compute_probs(
  const double*  alphas,
  const double*  rands,
        double*  probs,
  int n, int K, int M)
{
  // assign overall id/index of the thread = id of row
  int i = _bid_x * BLOCK_DIM_X + _tid_x; 

  if(i < n) {
    double maxval;    
    int m, k;
    int maxind;
    double M_d = (double) M; 
    double w[21]; // w[K]

    for(k = 0; k < K; ++k){   // initialize probs (though already done on CPU)
      probs[i*K + k] = 0.0;
    }

    // core computations
    for(m = 0; m < M; ++m){   // loop over Monte Carlo iterations
      for(k = 0; k < K; ++k){  // generate W ~ N(alpha, 1)
        w[k] = alphas[i*K + k] + rands[m*K + k];
      }

      // determine which category has max W
      maxind = K-1;
      maxval = w[K-1];
      for(k = 0; k < (K-1); ++k){
        if(w[k] > maxval){
          maxind = k;
          maxval = w[k];
        } 
      }
      probs[i*K + maxind] += 1.0;
    }

    // compute final proportions
    for(k = 0; k < K; ++k) {
      probs[i*K + k] /= M_d;
    }
  }
}

void compute_probs_unitStrides(
  const double*  alphas,
  const double*  rands,
        double*  probs,
  int n, int K, int M)
{
  // assign overall id/index of the thread = id of row
  int i = _bid_x * BLOCK_DIM_X + _tid_x; 

  if(i < n) {
    double maxval;    
    int m, k;
    int maxind;
    double M_d = (double) M; 
    double w[21]; // w[K]

    for(k = 0; k < K; ++k){  // initialize probs (though already done on CPU)
      probs[k*n + i] = 0.0;
    }

    // core computations
    for(m = 0; m < M; ++m){    // loop over Monte Carlo iterations
      for(k = 0; k < K; ++k){  // generate W ~ N(alpha, 1)
        // with +i we now have unit strides in inner loop
        w[k] = alphas[k*n + i] + rands[k*M + m];
      }

      // determine which category has max W
      maxind = K-1;
      maxval = w[K-1];
      for(k = 0; k < (K-1); ++k){
        if(w[k] > maxval){
          maxind = k;
          maxval = w[k];
        } 
      }
      probs[maxind*n + i] += 1.0;
    }

    // compute final proportions
    for(k = 0; k < K; ++k) {
      // unit strides
      probs[k*n + i] /= M_d;
    }
  }
}

void compute_probs_unitStrides_sharedMem(
  const double*  alphas,
  const double*  rands,
        double*  probs,
  int n, int K, int M)
{
  // assign overall id/index of the thread = id of row
  int i = _bid_x * BLOCK_DIM_X + _tid_x; 
  if (i >= n) return;

  int threads_per_block = BLOCK_DIM_X; 

  // set up shared memory: half for probs and half for w
  double shared[4096];
  double* probs_shared = shared;

  // shared mem is one big block, so need to index into latter portion of it to use for w
  double* w = &shared[K*threads_per_block];

  double maxval;    
  int m, k;
  int maxind;
  double M_d = (double) M; 

  // initialize shared memory probs
  for(k = 0; k < K; ++k) {
    probs_shared[k*threads_per_block + _tid_x] = 0.0;
  }

  // core computation
  for(m = 0; m < M; ++m){     // loop over Monte Carlo iterations 
    for(k = 0; k < K; ++k){   // generate W ~ N(alpha, 1)
      w[k*threads_per_block + _tid_x] = alphas[k*n + i] + rands[k*M + m];
    }
    maxind = K-1;
    maxval = w[(K-1)*threads_per_block + _tid_x];
    for(k = 0; k < (K-1); ++k){
      if(w[k*threads_per_block + _tid_x] > maxval){
        maxind = k;
        maxval = w[k*threads_per_block + _tid_x];
      } 
    }
    probs_shared[maxind*threads_per_block + _tid_x] += 1.0;
  }

  for(k = 0; k < K; ++k) {
    probs_shared[k*threads_per_block + _tid_x] /= M_d;
  }

  // copy to device memory so can be returned to CPU
  for(k = 0; k < K; ++k) {
    probs[k*n + i] = probs_shared[k*threads_per_block + _tid_x];
  }
}
