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
/*
 * GPU-accelerated AIDW interpolation algorithm 
 *
 * Implemented with / without Shared Memory
 *
 * By Dr.Gang Mei
 *
 * Created on 2015.11.06, China University of Geosciences, 
 *                        gang.mei@cugb.edu.cn
 * Revised on 2015.12.14, China University of Geosciences, 
 *                        gang.mei@cugb.edu.cn
 * 
 * Related publications:
 *  1) "Evaluating the Power of GPU Acceleration for IDW Interpolation Algorithm"
 *     http://www.hindawi.com/journals/tswj/2014/171574/
 *  2) "Accelerating Adaptive IDW Interpolation Algorithm on a Single GPU"
 *     http://arxiv.org/abs/1511.02186
 *
 * License: http://creativecommons.org/licenses/by/4.0/
 */

#include <cstdio>
#include <cstdlib>     
#include <vector>
#include <cmath>
#include <chrono>

// Calculate the power parameter, and then weighted interpolating
// Without using shared memory

// Calculate the power parameter, and then weighted interpolating
// With using shared memory (Tiled version of the stage 2)

