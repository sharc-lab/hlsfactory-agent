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
/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <random>


// This kernel computes ReluGrad by processing one half2, two fp16, at a time.
// Kernel operation is (feature > 0) ? gradient : 0

// This kernel computes ReluGrad by processing one half2, two fp16, at a time.
// Kernel operation is (feature > 0) ? gradient : 0
static constexpr int VectorSize = 8;


// This kernel computes Relu by processing one integer, four char, at a time.
// Kernel operation is max(input,  0)

// This kernel computes Relu by processing one integer, four char, at a time.
// Kernel operation is max(input,  0)

