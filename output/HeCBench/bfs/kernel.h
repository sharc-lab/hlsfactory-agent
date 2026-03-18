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

// --- from bfs.cu ---
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <chrono>

#define MAX_THREADS_PER_BLOCK 256

#include "util.h"

//Structure to hold a node information
struct Node
{
  int starting;
  int no_of_edges;
};





//Apply BFS on a Graph

//----------------------------------------------------------
//--cambine:  main function
//--author:    created by Jianbin Fang
//--date:    25/01/2011
//----------------------------------------------------------
