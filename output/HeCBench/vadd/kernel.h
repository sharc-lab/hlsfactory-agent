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

// --- from main.cu ---
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <cassert>

#define GPU_CHECK(expr)                                               \
  do {                                                                \
    cudaError_t _err = (expr);                                        \
  } while (0)





static constexpr int vec_size = 8;



// ===========================================================================
// TV-layout element-wise add
//
//   Block tile: TILE_M=16 rows × TILE_N=256 cols  per thread-block.
//   Thread organisation inside the tile:
//     - 4 warps  (TILE_M / VALS_M  = 16/4  = 4 warps of 32 threads each)
//     - Threads in a warp are consecutive in the N direction (stride 1 in N)
//       matching the row-major layout for coalesced access.
//   Each thread loads:
//     - VALS_M=4 rows
//     - VALS_N=8 consecutive columns per row  (8×fp16 = 128-bit load)
//   Total values per thread = 4 × 8 = 32.
//   Total threads per block = 4 warps × 32 = 128.
//
// Memory access pattern:
//   Thread (warp_id, lane_id) handles rows
//     [warp_id*VALS_M .. warp_id*VALS_M + VALS_M)
//   and columns
//     [lane_id*VALS_N .. lane_id*VALS_N + VALS_N)
// ===========================================================================

static constexpr int WARP_SIZE  = 32;
static constexpr int TILE_M     = 16;   // block tile rows
static constexpr int TILE_N     = 256;  // block tile cols  (= 32 lanes × 8 values)
static constexpr int VALS_M     =  4;   // values per thread in M dimension
static constexpr int VALS_N     =  8;   // values per thread in N dimension (128-bit load)
static constexpr int WARPS      =  TILE_M / VALS_M;



// helpers
struct BenchResult {
  float avg_ms;   // mean over all timed iterations (ms)
  float gbps;     // effective memory bandwidth (GB/s) based on best time
};


