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

// --- from kernels.cu ---
#include <vector>
#include <iostream>
#include <chrono>
#include <hip/hip_runtime.h>
#include "util.h"  // graph

#define DIAMETER_SAMPLES 512

// This will output the proper HIP error strings in the event that a HIP host call returns an error
#ifndef checkHipErrors
#define checkHipErrors(err)  __checkHipErrors (err, __FILE__, __LINE__)

// These are the inline versions for all of the SDK helper functions
#endif

//Note: N must be a power of two
//Simple/Naive bitonic sort. We're only sorting ~512 elements one time, so performance isn't important

void bc_kernel(
  float * bc,
  const int * R,
  const int * C,
  const int * F,
  const int n,
  const int m,
  const int * d,
  const unsigned long long * sigma,
  const float * delta,
  const int * Q,
  const int * Q2,
  const int * S,
  const int * endpoints,
  int * next_source,
  const size_t pitch_d,
  const size_t pitch_sigma,
  const size_t pitch_delta,
  const size_t pitch_Q,
  const size_t pitch_Q2,
  const size_t pitch_S,
  const size_t pitch_endpoints,
  const int start,
  const int end,
  int * jia,
  int * diameters,
  const int * source_vertices,
  const bool approx)
{
  int ind;
  int i;
  int *Q_row;
  int *Q2_row;
  int *S_row;
  int *endpoints_row;

  int j = _tid_x;
  int *d_row = (int*)((char*)d + _bid_x*pitch_d);
  unsigned long long *sigma_row = (unsigned long long*)((char*)sigma + _bid_x*pitch_sigma);
  float *delta_row = (float*)((char*)delta + _bid_x*pitch_delta);


}

std::vector<float> bc_gpu(
  graph g,
  int max_threads_per_block,
  int number_of_SMs,
  program_options op,
  const std::set<int> &source_vertices)
{
  float *bc_gpu = new float[g.n];
  int next_source = number_of_SMs; 

  float *bc_d, *delta_d;
  int *d_d, *R_d, *C_d, *F_d, *Q_d, *Q2_d, *S_d, *endpoints_d, *next_source_d, *source_vertices_d;
  unsigned long long *sigma_d;
  size_t pitch_d, pitch_sigma, pitch_delta, pitch_Q, pitch_Q2, pitch_S, pitch_endpoints;
  int *jia_d, *diameters_d;

  dim3 dimGrid (number_of_SMs, 1, 1);
  dim3 dimBlock (max_threads_per_block, 1, 1); 

  //Allocate and transfer data to the GPU
  checkHipErrors(hipMalloc((void**)&bc_d,sizeof(float)*g.n));
  checkHipErrors(hipMalloc((void**)&R_d,sizeof(int)*(g.n+1)));
  checkHipErrors(hipMalloc((void**)&C_d,sizeof(int)*(2*g.m)));
  checkHipErrors(hipMalloc((void**)&F_d,sizeof(int)*(2*g.m)));

  checkHipErrors(hipMallocPitch((void**)&d_d,&pitch_d,sizeof(int)*g.n,dimGrid.x));
  checkHipErrors(hipMallocPitch((void**)&sigma_d,&pitch_sigma,sizeof(unsigned long long)*g.n,dimGrid.x));
  checkHipErrors(hipMallocPitch((void**)&delta_d,&pitch_delta,sizeof(float)*g.n,dimGrid.x));
  //Making Queues/Stack of size O(n) since we won't duplicate
  checkHipErrors(hipMallocPitch((void**)&Q_d,&pitch_Q,sizeof(int)*g.n,dimGrid.x));
  checkHipErrors(hipMallocPitch((void**)&Q2_d,&pitch_Q2,sizeof(int)*g.n,dimGrid.x));
  checkHipErrors(hipMallocPitch((void**)&S_d,&pitch_S,sizeof(int)*g.n,dimGrid.x));
  checkHipErrors(hipMallocPitch((void**)&endpoints_d,&pitch_endpoints,sizeof(int)*(g.n+1),dimGrid.x));

  checkHipErrors(hipMalloc((void**)&next_source_d,sizeof(int)));

  // source_vertices of type "std::set" has no data() method
  std::vector<int> source_vertices_h(source_vertices.size());
  std::copy(source_vertices.begin(),source_vertices.end(),source_vertices_h.begin());

  checkHipErrors(hipMalloc((void**)&jia_d,sizeof(int)));
  checkHipErrors(hipMalloc((void**)&diameters_d,sizeof(int)*DIAMETER_SAMPLES));
  checkHipErrors(hipMemset(jia_d,0,sizeof(int)));
  checkHipErrors(hipMemset(diameters_d,0,sizeof(int)*DIAMETER_SAMPLES));

  checkHipErrors(hipMemcpy(R_d,g.R,sizeof(int)*(g.n+1),hipMemcpyHostToDevice));
  checkHipErrors(hipMemcpy(C_d,g.C,sizeof(int)*(2*g.m),hipMemcpyHostToDevice));
  checkHipErrors(hipMemcpy(F_d,g.F,sizeof(int)*(2*g.m),hipMemcpyHostToDevice));
  checkHipErrors(hipMemset(bc_d,0,sizeof(float)*g.n));
  checkHipErrors(hipMemcpy(next_source_d,&next_source,sizeof(int),hipMemcpyHostToDevice));

  int end;
  bool approx;

  hipDeviceSynchronize();
  auto start = std::chrono::steady_clock::now();

  hipLaunchKernelGGL(bc_kernel, dimGrid, dimBlock, 0, 0, 
      bc_d,
      R_d,
      C_d,
      F_d,
      g.n,
      g.m,
      d_d,
      sigma_d,
      delta_d,
      Q_d,
      Q2_d,
      S_d,
      endpoints_d,
      next_source_d,
      pitch_d,
      pitch_sigma,
      pitch_delta,
      pitch_Q,
      pitch_Q2,
      pitch_S,
      pitch_endpoints,
      0,
      end,
      jia_d,
      diameters_d,
      source_vertices_d,
      approx);

  hipDeviceSynchronize();
  auto stop = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
  std::cout << "Kernel execution time " << time * 1e-9f << " (s)\n";

  // GPU result
  checkHipErrors(hipMemcpy(bc_gpu,bc_d,sizeof(float)*g.n,hipMemcpyDeviceToHost));

  checkHipErrors(hipFree(bc_d));
  checkHipErrors(hipFree(R_d));
  checkHipErrors(hipFree(C_d));
  checkHipErrors(hipFree(F_d));
  checkHipErrors(hipFree(d_d));
  checkHipErrors(hipFree(sigma_d));
  checkHipErrors(hipFree(delta_d));
  checkHipErrors(hipFree(Q_d));
  checkHipErrors(hipFree(Q2_d));
  checkHipErrors(hipFree(S_d));
  checkHipErrors(hipFree(endpoints_d));
  checkHipErrors(hipFree(next_source_d));
  checkHipErrors(hipFree(jia_d));
  checkHipErrors(hipFree(diameters_d));
  checkHipErrors(hipFree(source_vertices_d));

  //Copy GPU result to a vector
  std::vector<float> bc_gpu_v(bc_gpu,bc_gpu+g.n);


  delete[] bc_gpu;
  return bc_gpu_v;
}

// query the properties of a single device for simplicity
