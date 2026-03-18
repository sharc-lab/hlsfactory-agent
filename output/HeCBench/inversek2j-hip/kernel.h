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
// Designed by: Amir Yazdanbakhsh
// Date: March 26th - 2015
// Alternative Computing Technologies Lab.
// Georgia Institute of Technology

#include <fstream>
#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <hip/hip_runtime.h>

#define MAX_LOOP 25
#define MAX_DIFF 0.15f
#define NUM_JOINTS 3
#define PI 3.14159265358979f
#define NUM_JOINTS_P1 (NUM_JOINTS + 1)
#define BLOCK_SIZE 128

using namespace std;



