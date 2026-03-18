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
#include <hip/hip_runtime.h>

#define max(x,y)  ((x) > (y)? (x) : (y))
#define min(x,y)  ((x) < (y)? (x) : (y))
#define ceil(a,b) ((a) % (b) == 0 ? (a) / (b) : ((a) / (b)) + 1)





extern "C" void offload (double *h_flux_0, double *h_flux_1, double *h_flux_2, double *h_flux_3, double *h_flux_4, double *h_cons_1, double *h_cons_2, double *h_cons_3, double *h_cons_4, double *h_q_1, double *h_q_2, double *h_q_3, double *h_q_4, double dxinv0, double dxinv1, double dxinv2, int L, int M, int N, int repeat) {

  size_t vol_size = sizeof(double)*L*M*N;

  double *flux_0;
  hipMalloc (&flux_0, vol_size);
  check_error ("Failed to allocate device memory for flux_0\n");
  double *flux_1;
  hipMalloc (&flux_1, vol_size);
  check_error ("Failed to allocate device memory for flux_1\n");
  double *flux_2;
  hipMalloc (&flux_2, vol_size);
  check_error ("Failed to allocate device memory for flux_2\n");
  double *flux_3;
  hipMalloc (&flux_3, vol_size);
  check_error ("Failed to allocate device memory for flux_3\n");
  double *flux_4;
  hipMalloc (&flux_4, vol_size);
  check_error ("Failed to allocate device memory for flux_4\n");
  double *cons_1;
  hipMalloc (&cons_1, vol_size);
  check_error ("Failed to allocate device memory for cons_1\n");
  hipMemcpy (cons_1, h_cons_1, vol_size, hipMemcpyHostToDevice);
  double *cons_2;
  hipMalloc (&cons_2, vol_size);
  check_error ("Failed to allocate device memory for cons_2\n");
  hipMemcpy (cons_2, h_cons_2, vol_size, hipMemcpyHostToDevice);
  double *cons_3;
  hipMalloc (&cons_3, vol_size);
  check_error ("Failed to allocate device memory for cons_3\n");
  hipMemcpy (cons_3, h_cons_3, vol_size, hipMemcpyHostToDevice);
  double *cons_4;
  hipMalloc (&cons_4, vol_size);
  check_error ("Failed to allocate device memory for cons_4\n");
  hipMemcpy (cons_4, h_cons_4, vol_size, hipMemcpyHostToDevice);
  double *q_1;
  hipMalloc (&q_1, vol_size);
  check_error ("Failed to allocate device memory for q_1\n");
  hipMemcpy (q_1, h_q_1, vol_size, hipMemcpyHostToDevice);
  double *q_2;
  hipMalloc (&q_2, vol_size);
  check_error ("Failed to allocate device memory for q_2\n");
  hipMemcpy (q_2, h_q_2, vol_size, hipMemcpyHostToDevice);
  double *q_3;
  hipMalloc (&q_3, vol_size);
  check_error ("Failed to allocate device memory for q_3\n");
  hipMemcpy (q_3, h_q_3, vol_size, hipMemcpyHostToDevice);
  double *q_4;
  hipMalloc (&q_4, vol_size);
  check_error ("Failed to allocate device memory for q_4\n");
  hipMemcpy (q_4, h_q_4, vol_size, hipMemcpyHostToDevice);

  dim3 blockconfig (16, 4, 4);
  dim3 gridconfig (ceil(N, 16), ceil(M, 4), ceil(L, 4));

  long t1 = 0, t2 = 0, t3 = 0;


  printf("Average kernel execution time (k1): %f (ms)\n", t1 * 1e-6 / repeat);
  printf("Average kernel execution time (k2): %f (ms)\n", t2 * 1e-6 / repeat);
  printf("Average kernel execution time (k3): %f (ms)\n", t3 * 1e-6 / repeat);

  hipMemcpy (h_flux_0, flux_0, vol_size, hipMemcpyDeviceToHost);
  hipMemcpy (h_flux_1, flux_1, vol_size, hipMemcpyDeviceToHost);
  hipMemcpy (h_flux_2, flux_2, vol_size, hipMemcpyDeviceToHost);
  hipMemcpy (h_flux_3, flux_3, vol_size, hipMemcpyDeviceToHost);
  hipMemcpy (h_flux_4, flux_4, vol_size, hipMemcpyDeviceToHost);

  hipFree(cons_1);
  hipFree(cons_2);
  hipFree(cons_3);
  hipFree(cons_4);
  hipFree(q_1);
  hipFree(q_2);
  hipFree(q_3);
  hipFree(q_4);
  hipFree(flux_0);
  hipFree(flux_1);
  hipFree(flux_2);
  hipFree(flux_3);
  hipFree(flux_4);
}
