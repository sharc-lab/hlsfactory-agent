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

// --- from kernel_gpu.cu ---
#include <chrono>
#include <hip/hip_runtime.h>
#include "kernel.h"




extern "C" void gpu_pso(int p, int r,
                        float *positions,float *velocities,float *pBests,float *gBest)
{
  int size = p*DIM;
  size_t size_byte = sizeof(float) * size;
  size_t res_size_byte = sizeof(float) * DIM;

  float *devPos;
  float *devVel;
  float *devPBest;
  float *devGBest;

  hipMalloc((void**)&devPos,size_byte);
  hipMalloc((void**)&devVel,size_byte);
  hipMalloc((void**)&devPBest,size_byte);
  hipMalloc((void**)&devGBest,res_size_byte);

  int threadNum=256;
  int blocksNum1=(size+threadNum-1)/threadNum;
  int blocksNum2=(p+threadNum-1)/threadNum;

  hipMemcpy(devPos,positions,size_byte,hipMemcpyHostToDevice);
  hipMemcpy(devVel,velocities,size_byte,hipMemcpyHostToDevice);
  hipMemcpy(devPBest,pBests,size_byte,hipMemcpyHostToDevice);
  hipMemcpy(devGBest,gBest,res_size_byte,hipMemcpyHostToDevice);

  hipDeviceSynchronize();
  auto start = std::chrono::steady_clock::now();


  hipDeviceSynchronize();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average kernel execution time %f (us)\n", time * 1e-3f / r);
  
  hipMemcpy(gBest,devGBest,res_size_byte,hipMemcpyDeviceToHost);
  hipMemcpy(pBests,devPBest,size_byte,hipMemcpyDeviceToHost);

  hipFree(devPos);
  hipFree(devVel);
  hipFree(devPBest);
  hipFree(devGBest);
}
