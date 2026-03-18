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

// --- from linear_iter.cu ---
#include <math.h>




// --- from linear_par.cu ---
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>

#define local_size TEMP_WORKGROUP_SIZE 

int cpu_offset;



static



// --- from main.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


clock_t start;
clock_t end;

extern int cpu_offset;

/* Read file */






// --- from kernel.h ---
#ifdef __NVCC__
inline void operator+=(float2 &a, const float2 &b)
{
    a.x += b.x;
    a.y += b.y;
}

inline void operator+=(float4 &a, const float4 &b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
}
#endif

void linear_regression(
  const float2 * dataset,
        float4 * result)
{
  float4 interns[4096];

  size_t loc_id   = _tid_x;
  size_t loc_size = BLOCK_DIM_X; 
  size_t glob_id  = _bid_x * loc_size + loc_id;

  /* Initialize local buffer */
  interns[loc_id].x = dataset[glob_id].x;
  interns[loc_id].y = dataset[glob_id].y;
  interns[loc_id].z = (dataset[glob_id].x * dataset[glob_id].y);
  interns[loc_id].w = (dataset[glob_id].x * dataset[glob_id].x);

  for (size_t i = (loc_size / 2), old_i = loc_size; i > 0; old_i = i, i /= 2)
  {
    if (loc_id < i) {
      // Only first half of workitems on each workgroup
      interns[loc_id] += interns[loc_id + i];
      if (loc_id == (i - 1) && old_i % 2 != 0) {
        // If there is an odd number of data
        interns[loc_id] += interns[old_i - 1];
      }
    }
  }

  if (loc_id == 0) result[_bid_x] = interns[0];
}

void rsquared(
  const float2 * dataset,
  const float mean,
  const float2 equation, // [a0,a1]
  float2 * result)
{
  float2 dist[4096];

  size_t loc_id   = _tid_x;
  size_t loc_size = BLOCK_DIM_X; 
  size_t glob_id  = _bid_x * loc_size + loc_id;

  dist[loc_id].x = powf((dataset[glob_id].y - mean), 2.f);

  float y_estimated = dataset[glob_id].x * equation.y + equation.x;
  dist[loc_id].y = powf((y_estimated - mean), 2.f);

  for (size_t i = (loc_size / 2), old_i = loc_size; i > 0; old_i = i, i /= 2)
  {
    if (loc_id < i) {
      // Only first half of workitems on each workgroup
      dist[loc_id] += dist[loc_id + i];
      if (loc_id == (i - 1) && old_i % 2 != 0) {
        // If there is an odd number of data
        dist[loc_id] += dist[old_i - 1];
      }
    }
  }

  if (loc_id == 0) result[_bid_x] = dist[0];
}


// --- from linear.h ---
#ifndef LINEAR_H__
#define LINEAR_H__

#ifdef __NVCC__
#else
#include <hip/hip_runtime.h>
#endif

#define RESULT_FILENAME "assets/_results.txt"
#define TEMP_FILENAME "assets/temperature.txt"

#define TEMP_SIZE 96453
#define TEMP_WORKGROUP_SIZE 63
#define TEMP_WORKGROUP_NBR (TEMP_SIZE / TEMP_WORKGROUP_SIZE)

#define LOG_DATASET() \
  for (int i = 0; i < DATASET_SIZE; i++) \
    printf("(%f, %f)\n", dataset[i].x, dataset[i].y);

#define LOG_DATA_T(var) printf("data_t:\n\tx: %f\n\ty: %f\n", var.x, var.y);
#define LOG_SUM_T(var) printf("sum_t:\n\tsumx: %f\n\tsumy: %f\n\tsumxy: %f\n\tsumxsq: %f\n", var.x, var.y, var.z, var.w);
#define LOG_RESULT_T(var) printf("result_t:\n\ta0: %f\n\ta1: %f\n\ttime: %f\n", var.a0, var.a1, var.time);
#define LOG_RSQUARED_T(var) printf("rsquared_t:\n\tactual: %f\n\testimated: %f\n", var.x, var.y)

/* a0 # a1 # time # rsquared */
#define WRITE_RESULT(file, result) \
  fprintf(file, "%.3f   %.3f   %3.f   %d\n", \
    result.a0, \
    result.a1, \
    result.ktime * 1e-6, \
    result.rsquared);

#define PRINT_RESULT(title, result) \
  printf("\t%s\n\t--------\n\t| Equation: y = %.3fx + %.3f\n\n", \
    title, \
    result.a1, \
    result.a0);

typedef struct {
    int repeat;
 char * filename;
 size_t size;
 size_t wg_size;
 size_t wg_count;
} linear_param_t;

typedef float2 data_t;

typedef float4 sum_t;

typedef struct {
  float a0;
  float a1;
  int rsquared;
  double ktime;  // total kernel execution time
} result_t;

typedef struct {
  result_t iterative;
  result_t parallelized;
} results_t;

typedef float2 rsquared_t;

void parallelized_regression(linear_param_t *, data_t *, result_t *);
void iterative_regression(linear_param_t *, data_t *, result_t *);

#endif
