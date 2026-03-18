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
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif
#ifndef GRID_DIM_Z
#define GRID_DIM_Z 1
#endif

// --- from main.cu ---
/*
 * Copyright (c) 2019, NVIDIA CORPORATION.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <chrono>

using namespace std;

#define MAX_KERNEL_THREADS 256
#define mask 0xFFFFFFFF

// float or double
typedef float vtype;
typedef vector<vector<vtype>> matrix;

template<typename T>

// Volume of neighboors (*weight_s)
template<bool weighted, typename T>

// Volume of intersections (*weight_i) and cumulated volume of neighboors (*weight_s)
// Note the number of columns is constrained by the number of rows
template<bool weighted, typename T>

// Reference https://github.com/SPEAR-UIC/CodeGreen/tree/main/lassi_solutions
template<bool weighted, typename T>

template<bool weighted, typename T>

template <bool weighted, typename T>

template <bool weighted, typename T>

// Utilities

template <typename T>

// Reference: https://www.geeksforgeeks.org/sparse-matrix-representations-set-3-csr/

