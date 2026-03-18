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
#include <cassert>
#include <chrono>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <list>
#include <iostream>
#include <hip/hip_runtime.h>
#include "MD.h"
#include "reference.h"
#include "utils.h"


