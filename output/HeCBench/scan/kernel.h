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

template<typename T>

// bank conflict aware optimization

#define LOG_MEM_BANKS 5
#define OFFSET(n) ((n) >> LOG_MEM_BANKS)

// N is the number of elements to scan in a thread block
template<typename T, int N>

template<typename T, int N>

template <typename T, int N>

template<int N>

