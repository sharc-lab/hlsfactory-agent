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
// Copyright (c) 2019-2020, NVIDIA CORPORATION.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

///////////////////////////////////////////////////////////////////////////////
//                            LOMBSCARGLE                                    //
///////////////////////////////////////////////////////////////////////////////

/*
   import cupy as cp
   import matplotlib.pyplot as plt
   First define some input parameters for the signal:
   A = 2.
   w = 1.
   phi = 0.5 * cp.pi
   nin = 10000
   nout = 1000000
   r = cp.random.rand(nin)
   x = cp.linspace(0.01, 10*cp.pi, nin)
   Plot a sine wave for the selected times:
   y = A * cp.sin(w*x+phi)
   Define the array of frequencies for which to compute the periodogram:
   f = cp.linspace(0.01, 10, nout)
   Calculate Lomb-Scargle periodogram:
   pgram = cusignal.lombscargle(x, y, f, normalize=True)
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <hip/hip_runtime.h>



