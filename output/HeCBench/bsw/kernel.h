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

// --- from driver.cu ---
#include "utils.hpp"



// --- from kernel.cu ---
#include "kernel.hpp"

__inline__ short
warpReduceMax_with_index(short val, short& myIndex, short& myIndex2, unsigned lengthSeqB, bool inverse)
{
  int   warpSize = 32;
  short myMax    = 0;
  short newInd   = 0;
  short newInd2  = 0;
  short ind      = myIndex;
  short ind2     = myIndex2;
  myMax          = val;
  unsigned mask  = 0;  // BLOCK_DIM_X
  myIndex  = ind;
  myIndex2 = ind2;
  val      = myMax;
  return val;
}





// --- from main.cu ---
#include "utils.hpp"
#include "driver.hpp"




// --- from utils.cu ---
#include "utils.hpp"


