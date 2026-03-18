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

// --- from vmc2.cu ---
#include <chrono>
#include <cstdio>
#include <cmath>
#include <hip/hip_runtime.h>

using namespace std;

using FLOAT = float;

#define CHECK(test) if (test != hipSuccess) throw "error";

const int NTHR_PER_BLK = 256;           // Number of CUDA threads per block
const int NBLOCK  = 56*4;               // Number of CUDA blocks (SMs on P100)
const int Npoint = NBLOCK*NTHR_PER_BLK; // No. of independent samples
const int Neq = 100000;                 // No. of generations to equilibrate 
const int Ngen_per_block = 5000;        // No. of generations per block
const float DELTA = 2.0;                // Random step size

// Explicitly typed constants so can easily work with both floats and floats
static const FLOAT FOUR = 4.0; 
static const FLOAT TWO  = 2.0;
static const FLOAT ONE  = 1.0;
static const FLOAT HALF = 0.5;
static const FLOAT ZERO = 0.0;



  





// Initialize random number generator

// ZERO stats counters on the GPU

// initializes samples



