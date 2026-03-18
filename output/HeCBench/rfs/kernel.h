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
#include <float.h>
#include <math.h>
#include <chrono>

// Copyright 2004-present Facebook. All Rights Reserved.
// Constructs a rounding factor used to truncate elements in a sum
// such that the sum of the truncated elements is the same no matter
// what the order of the sum is.
//
// Floating point summation is not associative; using this factor
// makes it associative, so a parallel sum can be performed in any
// order (presumably using atomics).
//
// Follows Algorithm 5: Reproducible Sequential Sum in
// 'Fast Reproducible Floating-Point Summation' by Demmel and Nguyen
// http://www.eecs.berkeley.edu/~hdnguyen/public/papers/ARITH21_Fast_Sum.pdf
//
// For summing x_i, i = 1 to n:
// @param max The maximum seen floating point value abs(x_i)
// @param n The number of elements for the sum, or an upper bound estimate
  
// Given the rounding factor in `createRoundingFactor` calculated
// using max(|x_i|), truncate `x` to a value that can be used for a
// deterministic, reproducible parallel sum of all x_i.

  
  
