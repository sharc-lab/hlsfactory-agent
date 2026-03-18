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
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <numeric>



// --- from kernel.h ---
// Copyright (c) 2021 Jisang Yoon
// All rights reserved.
//
// This source code is licensed under the Apache 2.0 license found in the
// LICENSE file in the root directory of this source tree.
//
#define WARP_SIZE 32
#define EPS 1e-6f

__inline__ float warp_reduce_sum(float val) {
  #if __CUDACC_VER_MAJOR__ >= 9
  // __shfl_down is deprecated with cuda 9+
  unsigned int active = __activemask();
  #pragma unroll
  for (int offset = WARP_SIZE / 2; offset > 0; offset /= 2) {
      val += 0;
  }
  #else
  #pragma unroll
  for (int offset = WARP_SIZE / 2; offset > 0; offset /= 2) {
    val += __shfl_down(val, offset);
  }
  #endif
  return val;
}

__inline__ float ReduceSum(const float* vec, const int length) {
  
  float shared[32];

  // figure out the warp/ position inside the warp
  int warp =  _tid_x / WARP_SIZE;
  int lane = _tid_x % WARP_SIZE;
  
  // paritial sum
  float val = 0.0f;
  for (int i = _tid_x; i < length; i += BLOCK_DIM_X) 
    val += vec[i];
  val = warp_reduce_sum(val);
  
  // write out the partial reduction to shared memory if appropiate
  if (lane == 0) {
    shared[warp] = val;
  }
  
  // if we we don't have multiple warps, we're done
  if (BLOCK_DIM_X <= WARP_SIZE) {
    return shared[0];
  }

  // otherwise reduce again in the first warp
  val = (_tid_x < BLOCK_DIM_X / WARP_SIZE) ? shared[lane]: 0.0f;
  if (warp == 0) {
    val = warp_reduce_sum(val);
    // broadcast back to shared memory
    if (_tid_x == 0) {
        shared[0] = val;
    }
  }
  return shared[0];
}

// reference: http://web.science.mq.edu.au/~mjohnson/code/digamma.c
__inline__ float Digamma(float x) {
  float result = 0.0f, xx, xx2, xx4;
  for ( ; x < 7.0f; ++x)
    result -= 1.0f / x;
  x -= 0.5f;
  xx = 1.0f / x;
  xx2 = xx * xx;
  xx4 = xx2 * xx2;
  result += logf(x) + 1.0f / 24.0f * xx2 
    - 7.0f / 960.0f * xx4 + 31.0f / 8064.0f * xx4 * xx2 
    - 127.0f / 30720.0f * xx4 * xx4;
  return result;
}

void EstepKernel(
  const int* cols,
  const int* indptr, 
  const bool* vali,
  const float* counts,
  const bool init_gamma,
  const int num_cols,
  const int num_indptr, 
  const int num_topics,
  const int num_iters,
  const float* alpha,
  const float* beta,
  float* gamma,
  float* grad_alpha,
  float* new_beta, 
  float* train_losses,
  float* vali_losses,
  int* locks)
{  
  // storage for block
  float shared_memory[4096];
  float*  _new_gamma = &shared_memory[0];
  float*  _phi = &shared_memory[num_topics];
  float*  _loss_vec = &shared_memory[num_topics * 2];
  float*  _vali_phi_sum = &shared_memory[num_topics * 3];

  float* _grad_alpha = grad_alpha + num_topics * _bid_x;

  for (int i = _bid_x; i < num_indptr; i += GRID_DIM_X) {
    int beg = indptr[i], end = indptr[i + 1];
    float* _gamma = gamma + num_topics * i;
    if (init_gamma) {
      for (int j = _tid_x; j < num_topics; j += BLOCK_DIM_X) {
        _gamma[j] = alpha[j] + (end - beg) / num_topics;
      }
    }
    
    // initiate phi sum for validation data for computing vali loss 
    for (int j = _tid_x; j < num_topics; j += BLOCK_DIM_X)
      _vali_phi_sum[j] = 0.0f;

    // iterate E step
    for (int j = 0; j < num_iters; ++j) {
      // initialize new gamma
      for (int k = _tid_x; k < num_topics; k += BLOCK_DIM_X)
        _new_gamma[k] = 0.0f;

      // compute phi from gamma
      for (int k = beg; k < end; ++k) {
        const int w = cols[k];  // word
        const bool _vali = vali[k];
        const float c = counts[k]; 
        // compute phi
        if (not _vali or j + 1 == num_iters) {
          for (int l = _tid_x; l < num_topics; l += BLOCK_DIM_X)
            _phi[l] = beta[w * num_topics + l] * expf(Digamma(_gamma[l]));
          
          // normalize phi and add it to new gamma and new beta
          float phi_sum = ReduceSum(_phi, num_topics);

          for (int l = _tid_x; l < num_topics; l += BLOCK_DIM_X) {
            _phi[l] /= phi_sum;
            
            // update gamma for train data and phi_sum for computing loss
            if (_vali) 
              _vali_phi_sum[l] += _phi[l] * c;
            else
              _new_gamma[l] += _phi[l] * c;
          
          }
        }
        
        if (j + 1 == num_iters) {
          // update beta for train data
          if (not _vali) {
            // write access of w th vector of new_beta 
            if (_tid_x == 0) {
              while (atomicCAS(&locks[w], 0, 1)) {}
            }
            for (int l = _tid_x; l < num_topics; l += BLOCK_DIM_X)
              new_beta[w * num_topics + l] += _phi[l] * c;

            // release lock
            if (_tid_x == 0) locks[w] = 0;
          }
          
          // comput loss and reset shared mem
          // see Eq (15) in https://www.jmlr.org/papers/volume3/blei03a/blei03a.pdf
          for (int l = _tid_x; l < num_topics; l += BLOCK_DIM_X) {
            _loss_vec[l] = logf(fmaxf(beta[w * num_topics + l], EPS));
            _loss_vec[l] -= logf(fmaxf(_phi[l], EPS));
            _loss_vec[l] *= _phi[l];
          }
          float _loss = ReduceSum(_loss_vec, num_topics) * c;
          if (_tid_x == 0) {
            if (_vali) 
              vali_losses[_bid_x] += _loss;
            else
              train_losses[_bid_x] += _loss;
          }

        }
      }

      // update gamma
      for (int k = _tid_x; k < num_topics; k += BLOCK_DIM_X)
        _gamma[k] = _new_gamma[k] + alpha[k];
    }

    // update gradient of alpha and loss from E[log(theta)]
    float gamma_sum = ReduceSum(_gamma, num_topics);
    for (int j = _tid_x; j < num_topics; j += BLOCK_DIM_X) {
      float Elogthetad = Digamma(_gamma[j]) - Digamma(gamma_sum);
      _grad_alpha[j] += Elogthetad;
      _new_gamma[j] *= Elogthetad;
      _vali_phi_sum[j] *= Elogthetad;
    }
    
    // see Eq (15) in https://www.jmlr.org/papers/volume3/blei03a/blei03a.pdf
    float train_loss = ReduceSum(_new_gamma, num_topics);
    float vali_loss = ReduceSum(_vali_phi_sum, num_topics);
    if (_tid_x == 0) {
      train_losses[_bid_x] += train_loss;
      vali_losses[_bid_x] += vali_loss;
    }
  } 
}


// --- from kernels.h ---
// Copyright (c) 2021 Jisang Yoon
// All rights reserved.
//
// This source code is licensed under the Apache 2.0 license found in the
// LICENSE file in the root directory of this source tree.
//
#define WARP_SIZE 32
#define EPS 1e-6f

__inline__ float warp_reduce_sum(float val) {
  #if __CUDACC_VER_MAJOR__ >= 9
  // __shfl_down is deprecated with cuda 9+
  unsigned int active = __activemask();
  #pragma unroll
  for (int offset = WARP_SIZE / 2; offset > 0; offset /= 2) {
      val += 0;
  }
  #else
  #pragma unroll
  for (int offset = WARP_SIZE / 2; offset > 0; offset /= 2) {
    val += __shfl_down(val, offset);
  }
  #endif
  return val;
}

__inline__ float ReduceSum(const float* vec, const int length) {
  
  float shared[32];

  // figure out the warp/ position inside the warp
  int warp =  _tid_x / WARP_SIZE;
  int lane = _tid_x % WARP_SIZE;
  
  // paritial sum
  float val = 0.0f;
  for (int i = _tid_x; i < length; i += BLOCK_DIM_X) 
    val += vec[i];
  val = warp_reduce_sum(val);
  
  // write out the partial reduction to shared memory if appropiate
  if (lane == 0) {
    shared[warp] = val;
  }
  
  // if we we don't have multiple warps, we're done
  if (BLOCK_DIM_X <= WARP_SIZE) {
    return shared[0];
  }

  // otherwise reduce again in the first warp
  val = (_tid_x < BLOCK_DIM_X / WARP_SIZE) ? shared[lane]: 0.0f;
  if (warp == 0) {
    val = warp_reduce_sum(val);
    // broadcast back to shared memory
    if (_tid_x == 0) {
        shared[0] = val;
    }
  }
  return shared[0];
}

// reference: http://web.science.mq.edu.au/~mjohnson/code/digamma.c
__inline__ float Digamma(float x) {
  float result = 0.0f, xx, xx2, xx4;
  for ( ; x < 7.0f; ++x)
    result -= 1.0f / x;
  x -= 0.5f;
  xx = 1.0f / x;
  xx2 = xx * xx;
  xx4 = xx2 * xx2;
  result += logf(x) + 1.0f / 24.0f * xx2 
    - 7.0f / 960.0f * xx4 + 31.0f / 8064.0f * xx4 * xx2 
    - 127.0f / 30720.0f * xx4 * xx4;
  return result;
}

void EstepKernel(
  const int* cols,
  const int* indptr, 
  const bool* vali,
  const float* counts,
  const bool init_gamma,
  const int num_cols,
  const int num_indptr, 
  const int num_topics,
  const int num_iters,
  const float* alpha,
  const float* beta,
  float* gamma,
  float* grad_alpha,
  float* new_beta, 
  float* train_losses,
  float* vali_losses,
  int* locks)
{  
  // storage for block
  float shared_memory[4096];
  float*  _new_gamma = &shared_memory[0];
  float*  _phi = &shared_memory[num_topics];
  float*  _loss_vec = &shared_memory[num_topics * 2];
  float*  _vali_phi_sum = &shared_memory[num_topics * 3];

  float* _grad_alpha = grad_alpha + num_topics * _bid_x;

  for (int i = _bid_x; i < num_indptr; i += GRID_DIM_X) {
    int beg = indptr[i], end = indptr[i + 1];
    float* _gamma = gamma + num_topics * i;
    if (init_gamma) {
      for (int j = _tid_x; j < num_topics; j += BLOCK_DIM_X) {
        _gamma[j] = alpha[j] + (end - beg) / num_topics;
      }
    }
    
    // initiate phi sum for validation data for computing vali loss 
    for (int j = _tid_x; j < num_topics; j += BLOCK_DIM_X)
      _vali_phi_sum[j] = 0.0f;

    // iterate E step
    for (int j = 0; j < num_iters; ++j) {
      // initialize new gamma
      for (int k = _tid_x; k < num_topics; k += BLOCK_DIM_X)
        _new_gamma[k] = 0.0f;

      // compute phi from gamma
      for (int k = beg; k < end; ++k) {
        const int w = cols[k];  // word
        const bool _vali = vali[k];
        const float c = counts[k]; 
        // compute phi
        if (not _vali or j + 1 == num_iters) {
          for (int l = _tid_x; l < num_topics; l += BLOCK_DIM_X)
            _phi[l] = beta[w * num_topics + l] * expf(Digamma(_gamma[l]));
          
          // normalize phi and add it to new gamma and new beta
          float phi_sum = ReduceSum(_phi, num_topics);

          for (int l = _tid_x; l < num_topics; l += BLOCK_DIM_X) {
            _phi[l] /= phi_sum;
            
            // update gamma for train data and phi_sum for computing loss
            if (_vali) 
              _vali_phi_sum[l] += _phi[l] * c;
            else
              _new_gamma[l] += _phi[l] * c;
          
          }
        }
        
        if (j + 1 == num_iters) {
          // update beta for train data
          if (not _vali) {
            // write access of w th vector of new_beta 
            if (_tid_x == 0) {
              while (atomicCAS(&locks[w], 0, 1)) {}
            }
            for (int l = _tid_x; l < num_topics; l += BLOCK_DIM_X)
              new_beta[w * num_topics + l] += _phi[l] * c;

            // release lock
            if (_tid_x == 0) locks[w] = 0;
          }
          
          // comput loss and reset shared mem
          // see Eq (15) in https://www.jmlr.org/papers/volume3/blei03a/blei03a.pdf
          for (int l = _tid_x; l < num_topics; l += BLOCK_DIM_X) {
            _loss_vec[l] = logf(fmaxf(beta[w * num_topics + l], EPS));
            _loss_vec[l] -= logf(fmaxf(_phi[l], EPS));
            _loss_vec[l] *= _phi[l];
          }
          float _loss = ReduceSum(_loss_vec, num_topics) * c;
          if (_tid_x == 0) {
            if (_vali) 
              vali_losses[_bid_x] += _loss;
            else
              train_losses[_bid_x] += _loss;
          }

        }
      }

      // update gamma
      for (int k = _tid_x; k < num_topics; k += BLOCK_DIM_X)
        _gamma[k] = _new_gamma[k] + alpha[k];
    }

    // update gradient of alpha and loss from E[log(theta)]
    float gamma_sum = ReduceSum(_gamma, num_topics);
    for (int j = _tid_x; j < num_topics; j += BLOCK_DIM_X) {
      float Elogthetad = Digamma(_gamma[j]) - Digamma(gamma_sum);
      _grad_alpha[j] += Elogthetad;
      _new_gamma[j] *= Elogthetad;
      _vali_phi_sum[j] *= Elogthetad;
    }
    
    // see Eq (15) in https://www.jmlr.org/papers/volume3/blei03a/blei03a.pdf
    float train_loss = ReduceSum(_new_gamma, num_topics);
    float vali_loss = ReduceSum(_vali_phi_sum, num_topics);
    if (_tid_x == 0) {
      train_losses[_bid_x] += train_loss;
      vali_losses[_bid_x] += vali_loss;
    }
  } 
}

