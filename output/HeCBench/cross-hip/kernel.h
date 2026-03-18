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
#include <chrono>
#include <random>
#include <hip/hip_runtime.h>

// Reference
// https://pytorch.org/docs/stable/generated/torch.linalg.cross.html#torch.linalg.cross

template <typename T, typename StrideType>

template <typename T, typename StrideType>

template <typename T>

template <typename T>

