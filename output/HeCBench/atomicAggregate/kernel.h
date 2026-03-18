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

// --- from main.cu ---
#include <chrono>
#include <stdio.h>

// reference
// https://stackoverflow.com/questions/59879285/whats-the-alternative-for-match-any-sync-on-compute-capability-6

// increment the value at ptr by 1 and return the old value


