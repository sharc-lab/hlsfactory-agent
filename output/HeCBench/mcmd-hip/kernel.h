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

// --- from force_kernel.cu ---
#include <math.h>
#include <stdio.h>
#include <hip/hip_runtime.h>

// minimal data needed to compute forces on a device
typedef struct atom_t {
  double pos[3] = {0,0,0};
  double eps=0; // lj
  double sig=0; // lj
  double charge=0;
  double f[3] = {0,0,0}; // force
  int molid=0;
  int frozen=0;
  double u[3] = {0,0,0}; // dipole
  double polar=0; // polarizability
} d_atom;


