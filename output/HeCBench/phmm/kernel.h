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
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <random>



// --- from constants_types.h ---
// constants for host and kernel codes
#define x_dim   11
#define y_dim   40
#define batch   4
#define states  3

// forward type
typedef double fArray[y_dim+1][batch][states-1];

// transition type
typedef double tArray[batch][states-1][states];

// likelihood type
typedef double lArray[2][batch][states-1];

// start type
typedef double sArray[states-1];



// --- from kernel.h ---
void pair_HMM_forward(
    const int cur_i,
    const int cur_j,
    //const double forward_matrix_in[x_dim+1][y_dim+1][batch][states-1],
    const fArray * forward_matrix_in,
    //const double transitions[x_dim+1][batch][states-1][states],
    const tArray * transitions,
    //const double emissions[x_dim+1][y_dim+1][batch][states-1],
    const fArray * emissions,
    // const double likelihood[2][2][batch][states-1],
    const lArray * likelihood,
    //const double start_transitions[batch][states-1],
    const sArray * start_transitions,
          //double forward_matrix_out[x_dim+1][y_dim+1][batch][states-1])
          fArray * forward_matrix_out)
{
  int batch_id = _bid_x;
  int states_id = _tid_x;

  double e[batch][states-1];
  double f01[1][batch][2];
  double mul_3d[1][batch][2];
  double mul_4d[4][batch][1][2];

  e[batch_id][states_id] = emissions[cur_i][cur_j][batch_id][states_id];

  double t[2][2][batch][2][2];
  for (int k = 0; k < 2; k++) {
    for (int l = 0; l < 2; l++) {
      t[0][0][batch_id][k][l] = transitions[cur_i - 1][batch_id][k][l];
      t[0][1][batch_id][k][l] = transitions[cur_i - 1][batch_id][k][l];
      t[1][0][batch_id][k][l] = transitions[cur_i][batch_id][k][l];
      t[1][1][batch_id][k][l] = transitions[cur_i][batch_id][k][l];
    }
  }

  if (cur_i > 0 && cur_j == 0) {
    if (cur_i == 1) {
      forward_matrix_out[1][0][batch_id][states_id] = 
        start_transitions[batch_id][states_id] * e[0][states_id];
    }
    else {
      double t01[batch][2][2];
      for (int j = 0; j < 2; j++) {
        for (int k = 0; k < 2; k++) {
          t01[batch_id][j][k] = t[0][1][batch_id][j][k];
        }
      }

      f01[0][batch_id][states_id] = 
        forward_matrix_in[cur_i - 1][cur_j][batch_id][states_id];

      double s = 0.0;
      for (int k = 0; k < 2; k++)
        s += f01[0][batch_id][k] * t01[batch_id][k][states_id];
      s *= (e[batch_id][states_id] * likelihood[0][1][batch_id][states_id]);
      mul_3d[0][batch_id][states_id] = s;

      forward_matrix_out[cur_i][0][batch_id][states_id] = mul_3d[0][batch_id][states_id];
    }
  }
  else if (cur_i > 0 and cur_j > 0) {

    double f[2][2][batch][1][2];
    for (int i = 0; i < 2; i++) {
      f[0][0][batch_id][0][i] = forward_matrix_in[cur_i-1][cur_j-1][batch_id][i];
      f[0][1][batch_id][0][i] = forward_matrix_in[cur_i-1][cur_j][batch_id][i];
      f[1][0][batch_id][0][i] = forward_matrix_in[cur_i][cur_j-1][batch_id][i];
      f[1][1][batch_id][0][i] = forward_matrix_in[cur_i][cur_j][batch_id][i];
    }

    double s0 = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;
    double s3 = 0.0;

    for (int k = 0; k < 2; k++) {
      s0 += f[0][0][batch_id][0][k] * t[0][0][batch_id][k][states_id];
      s1 += f[0][1][batch_id][0][k] * t[0][1][batch_id][k][states_id];
      s2 += f[1][0][batch_id][0][k] * t[1][0][batch_id][k][states_id];
      s3 += f[1][1][batch_id][0][k] * t[1][1][batch_id][k][states_id];
    }
    s0 *= likelihood[0][0][batch_id][states_id];
    s1 *= likelihood[0][1][batch_id][states_id];
    s2 *= likelihood[1][0][batch_id][states_id];
    s3 *= likelihood[1][1][batch_id][states_id];
    mul_4d[0][batch_id][0][states_id] = s0;
    mul_4d[1][batch_id][0][states_id] = s1;
    mul_4d[2][batch_id][0][states_id] = s2;
    mul_4d[3][batch_id][0][states_id] = s3;

    for (int j = 0; j < 2; j++) {
      double summation = mul_4d[0][batch_id][0][j] + 
                         mul_4d[1][batch_id][0][j] +
                         mul_4d[2][batch_id][0][j] +
                         mul_4d[3][batch_id][0][j];

      summation *= e[batch_id][j];

      forward_matrix_out[cur_i][cur_j][batch_id][j] = summation;
    }
  }
}

