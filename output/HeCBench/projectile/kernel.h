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

// --- from Projectile.cu ---
//==============================================================
// Copyright © 2020 Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#include <chrono>
#include <cstdlib>
#include <vector>
#include "Projectile.hpp"

static const int num_elements = 10000000;
const int BLOCK_SIZE = 256;

// Function to calculate the range, maximum height and total flight time of a
// projectile


// in_vect and out_vect are the vectors with N Projectile numbers and are inputs to the
// parallel function

