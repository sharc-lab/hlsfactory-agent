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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <random>
#include <chrono>
#include <hip/hip_runtime.h>
#include "utils.h"

void bond_wlcpowallvisc(
             r64*  force_x,
             r64*  force_y,
             r64*  force_z,
    const float4*  coord_merged,
    const float4*  veloc,
    const int*   nbond,
    const int2*  bonds,
    const r64*  bond_r0,
    const r32*  temp_global,
    const r32*  r0_global,
    const r32*  mu_targ_global,
    const r32*  qp_global,
    const r32*  gamc_global,
    const r32*  gamt_global,
    const r32*  sigc_global,
    const r32*  sigt_global,
    const float3 period,
    const int padding,
    const int n_type,
    const int n_local )
{
  r32 shared_data[4096];
  r32* temp    = &shared_data[0];
  r32* r0      = &shared_data[1*(n_type+1)];
  r32* mu_targ = &shared_data[2*(n_type+1)];
  r32* qp      = &shared_data[3*(n_type+1)];
  r32* gamc    = &shared_data[4*(n_type+1)];
  r32* gamt    = &shared_data[5*(n_type+1)];
  r32* sigc    = &shared_data[6*(n_type+1)];
  r32* sigt    = &shared_data[7*(n_type+1)];


}

template <typename T>

template <typename T>

template <typename T>

template <typename T>

template <typename T>

