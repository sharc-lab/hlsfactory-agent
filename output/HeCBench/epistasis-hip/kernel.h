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
#include <math.h>
#include <float.h>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <chrono>
#include <hip/hip_runtime.h>
#include "reference.h"

using namespace std::chrono;
typedef high_resolution_clock myclock;
typedef duration<float> myduration;

#define MAX_WG_SIZE 256

template <typename T>

template <typename T>



