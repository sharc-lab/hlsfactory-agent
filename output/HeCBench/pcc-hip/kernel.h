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

// --- from device.cu ---
#include <chrono>
#include <iostream>
#include <fstream>
#include <hipblas/hipblas.h>
#include <hip/hip_runtime.h>
#include "device.h"

size_t remaining_B(int , size_t );



