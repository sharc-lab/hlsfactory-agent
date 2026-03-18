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

  int threadNum=256;
  int blocksNum1=(size+threadNum-1)/threadNum;
  int blocksNum2=(p+threadNum-1)/threadNum;

  auto start = std::chrono::steady_clock::now();


  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average kernel execution time %f (us)\n", time * 1e-3f / r);
  

}


// --- from kernel.h ---
#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define F(x) (1.f + ((x) - 1.f) / 4.f)

const   int DIM = 30;  // number of dimensions
const float START_RANGE_MIN = -5.12f;
const float START_RANGE_MAX = 5.12f;
const float OMEGA = 0.5f;
const float c1 = 1.5f;
const float c2 = 1.5f;
const float phi = 3.1415f;

float getRandom(float low,float high);
float getRandomClamped(int seed);
float host_fitness_function(float x[]);

void pso(int p,int r,float *positions,float *velocities,float *pBests,float *gBest);
extern "C" void gpu_pso(int p,int r,float *positions,float *velocities,float *pBests,float *gBest);

#endif /* KERNEL_H_ */
