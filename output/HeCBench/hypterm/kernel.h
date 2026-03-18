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
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif
#ifndef GRID_DIM_Z
#define GRID_DIM_Z 1
#endif

// --- from kernels.cu ---
#include <stdio.h>
#include <chrono>

#define max(x,y)  ((x) > (y)? (x) : (y))
#define min(x,y)  ((x) < (y)? (x) : (y))
#define ceil(a,b) ((a) % (b) == 0 ? (a) / (b) : ((a) / (b)) + 1)





extern "C" void offload (double *h_flux_0, double *h_flux_1, double *h_flux_2, double *h_flux_3, double *h_flux_4, double *h_cons_1, double *h_cons_2, double *h_cons_3, double *h_cons_4, double *h_q_1, double *h_q_2, double *h_q_3, double *h_q_4, double dxinv0, double dxinv1, double dxinv2, int L, int M, int N, int repeat) {

  size_t vol_size = sizeof(double)*L*M*N;

  double *flux_0;

  check_error ("Failed to allocate device memory for flux_0\n");
  double *flux_1;

  check_error ("Failed to allocate device memory for flux_1\n");
  double *flux_2;

  check_error ("Failed to allocate device memory for flux_2\n");
  double *flux_3;

  check_error ("Failed to allocate device memory for flux_3\n");
  double *flux_4;

  check_error ("Failed to allocate device memory for flux_4\n");
  double *cons_1;

  check_error ("Failed to allocate device memory for cons_1\n");

  double *cons_2;

  check_error ("Failed to allocate device memory for cons_2\n");

  double *cons_3;

  check_error ("Failed to allocate device memory for cons_3\n");

  double *cons_4;

  check_error ("Failed to allocate device memory for cons_4\n");

  double *q_1;

  check_error ("Failed to allocate device memory for q_1\n");

  double *q_2;

  check_error ("Failed to allocate device memory for q_2\n");

  double *q_3;

  check_error ("Failed to allocate device memory for q_3\n");

  double *q_4;

  check_error ("Failed to allocate device memory for q_4\n");

  dim3 blockconfig (16, 4, 4);
  dim3 gridconfig (ceil(N, 16), ceil(M, 4), ceil(L, 4));

  long t1 = 0, t2 = 0, t3 = 0;


  printf("Average kernel execution time (k1): %f (ms)\n", t1 * 1e-6 / repeat);
  printf("Average kernel execution time (k2): %f (ms)\n", t2 * 1e-6 / repeat);
  printf("Average kernel execution time (k3): %f (ms)\n", t3 * 1e-6 / repeat);

}


// --- from utils.h ---
#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>

#define TOLERANCE 1e-3

#define D 308

template<typename T>
static T get_random() {
  return ((T)(rand())/(T)(RAND_MAX-1));
}

template<typename T>
static T* getRandom3DArray(int height, int width_y, int width_x) {
  T (*a)[D][D] = (T (*)[D][D])new T[height*D*D];
  for (int i = 0; i < height; i++)
    for (int j = 0; j < width_y; j++)
      for (int k = 0; k < width_x; k++) {
        a[i][j][k] = get_random<T>() + 0.02121;
      }
  return (T*)a;
}

template<typename T>
static T* getZero3DArray(int height, int width_y, int width_x) {
  T (*a)[D][D] = (T (*)[D][D])new T[height*D*D];
  memset((void*)a, 0, sizeof(T) * height * width_y * width_x);
  return (T*)a;
}

template<typename T>
static double checkError3D
(int width_y, int width_x, const T *l_output, const T *l_reference, int z_lb,
 int z_ub, int y_lb, int y_ub, int x_lb, int x_ub) {
  const T (*output)[D][D] = (const T (*)[D][D])(l_output);
  const T (*reference)[D][D] = (const T (*)[D][D])(l_reference);
  double error = 0.0;
  double max_error = TOLERANCE, sum = 0.0;
  for (int i = z_lb; i < z_ub; i++) {
    for (int j = y_lb; j < y_ub; j++) {
      for (int k = x_lb; k < x_ub; k++) {
        sum += output[i][j][k];
        //printf ("real var1[%d][%d][%d] = %.6f and %.6f\n", i, j, k, reference[i][j][k], output[i][j][k]);
        double curr_error = fabs(output[i][j][k] - reference[i][j][k]);
        error += curr_error * curr_error;
        if (curr_error > max_error) {
          printf ("Values at index (%d,%d,%d) differ : %.6f and %.6f\n", i, j, k, reference[i][j][k], output[i][j][k]);
        }
      }
    }
  }
  printf ("checksum = %e\n", sum);
  return sqrt(error / ( (z_ub - z_lb) * (y_ub - y_lb) * (x_ub - x_lb)));
}

#endif
