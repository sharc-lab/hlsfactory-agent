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
#include <string.h>
#include <chrono>
#include <hip/hip_runtime.h>

#define BLOCK_SIZE 256

#include "reference.h"

#define CHECK_ERROR( err ) (CheckError( err, __FILE__, __LINE__ ))

template <typename T>

template <typename T>

template <typename T>

template <typename T>

template <typename T>

template <typename T>

template <typename T>

