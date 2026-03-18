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

// --- from kernels.cu ---
/*
 * Copyright (c) 2020-2021, NVIDIA CORPORATION.
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

/*
* Kernel distributes exact part of the kernel shap dataset
* Each block scatters the data of a row of `observations` into the (number of rows of
* background) in `dataset`, based on the row of `X`.
* So, given:
* background = [[0, 1, 2],
                [3, 4, 5]]
* observation = [100, 101, 102]
* X = [[1, 0, 1],
*      [0, 1, 1]]
*

/*
* Kernel distributes sampled part of the kernel shap dataset
* The first thread of each block calculates the sampling of `k` entries of `observation`
* to scatter into `dataset`. Afterwards each block scatters the data of a row of `X` into the (number of rows of
* background) in `dataset`.
* So, given:
* background = [[0, 1, 2, 3],
                [5, 6, 7, 8]]
* observation = [100, 101, 102, 103]
* nsamples = [3, 2]
*

template <typename DataT, typename IdxT>

template <typename DataT, typename IdxT>


// --- from main.cu ---
/*
 * Copyright (c) 2020-2021, NVIDIA CORPORATION.
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

#include <chrono>
#include <cmath>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

struct Dataset {
  int nrows_exact;
  int nrows_sampled;
  int ncols;
  int nrows_background;
  int max_samples;
  uint64_t seed;
};

typedef float T;

