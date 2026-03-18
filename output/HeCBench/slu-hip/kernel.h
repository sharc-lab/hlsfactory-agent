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

// --- from numeric.cu ---
#include <iostream>
#include <cmath>
#include <hip/hip_runtime.h>
#include "symbolic.h"
#include "Timer.h"

using namespace std;

#define TMPMEMNUM  10353
#define  Nstreams  16







