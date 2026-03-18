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

// --- from main.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <chrono>



// --- from kernels.h ---
#define idx(i,j)   (i)*x_points+(j)

void core (
    double * u_new,
    double * v_new,
    const double * u,
    const double * v,
    const int x_points,
    const int y_points,
    const double nu,
    const double del_t,
    const double del_x,
    const double del_y)
{
  int i = _bid_y * BLOCK_DIM_Y + _tid_y + 1;
  int j = _bid_x * BLOCK_DIM_X + _tid_x + 1;
  if (j < x_points - 1 && i < y_points - 1) {
    u_new[idx(i,j)] = u[idx(i,j)] + 
      (nu*del_t/(del_x*del_x)) * (u[idx(i,j+1)] + u[idx(i,j-1)] - 2 * u[idx(i,j)]) + 
      (nu*del_t/(del_y*del_y)) * (u[idx(i+1,j)] + u[idx(i-1,j)] - 2 * u[idx(i,j)]) - 
      (del_t/del_x)*u[idx(i,j)] * (u[idx(i,j)] - u[idx(i,j-1)]) - 
      (del_t/del_y)*v[idx(i,j)] * (u[idx(i,j)] - u[idx(i-1,j)]);

    v_new[idx(i,j)] = v[idx(i,j)] +
      (nu*del_t/(del_x*del_x)) * (v[idx(i,j+1)] + v[idx(i,j-1)] - 2 * v[idx(i,j)]) + 
      (nu*del_t/(del_y*del_y)) * (v[idx(i+1,j)] + v[idx(i-1,j)] - 2 * v[idx(i,j)]) -
      (del_t/del_x)*u[idx(i,j)] * (v[idx(i,j)] - v[idx(i,j-1)]) - 
      (del_t/del_y)*v[idx(i,j)] * (v[idx(i,j)] - v[idx(i-1,j)]);
  }
}

void bound_h (
    double * u_new,
    double * v_new,
    const int x_points,
    const int y_points)
{
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  if (i < x_points) {
    u_new[idx(0,i)] = 1.0;
    v_new[idx(0,i)] = 1.0;
    u_new[idx(y_points-1,i)] = 1.0;
    v_new[idx(y_points-1,i)] = 1.0;
  }
}

void bound_v (
    double * u_new,
    double * v_new,
    const int x_points,
    const int y_points)
{
  int j = _bid_x * BLOCK_DIM_X + _tid_x;
  if (j < y_points) {
    u_new[idx(j,0)] = 1.0;
    v_new[idx(j,0)] = 1.0;
    u_new[idx(j,x_points-1)] = 1.0;
    v_new[idx(j,x_points-1)] = 1.0;
  }
}

void update (
    double * u,
    double * v,
    const double * u_new,
    const double * v_new,
    const int n)
{
  int i = _bid_x * BLOCK_DIM_X + _tid_x;
  if (i < n) {
    u[i] = u_new[i];
    v[i] = v_new[i];
  }
}

