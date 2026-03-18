#include "kernel.h"

// --- from main-wave64.cu ---
__inline__ int warp_scan(int val, volatile int *s_data)
{
  // initialize shared memory accessed by each warp with zeros
  int idx = 2 * _tid_x - (_tid_x & (warpSize-1));
  s_data[idx] = 0;
  idx += warpSize;
  int t = s_data[idx] = val;
  s_data[idx] = t += s_data[idx - 1];
  s_data[idx] = t += s_data[idx - 2];
  s_data[idx] = t += s_data[idx - 4];
  s_data[idx] = t += s_data[idx - 8];
  return s_data[idx-1];
}

__inline__ size_t lanemask_lt()
{
  const unsigned int lane = _tid_x & (warpSize-1);
  return (1UL << (lane)) - 1UL;
}

__inline__ unsigned int binary_warp_scan(bool p)
{
  const size_t mask = lanemask_lt();
  size_t b = __ballot(p);
  return __popcll(b & mask);
}

bool valid(int x) {
  return x > 0;
}

__inline__ int block_binary_prefix_sums(int x)
{
  int sdata[80];

  bool predicate = valid(x);

  // A. Compute exclusive prefix sums within each warp
  int warpPrefix = binary_warp_scan(predicate);
  int idx = _tid_x;
  int warpIdx = idx / warpSize;
  int laneIdx = idx & (warpSize - 1);
#ifdef DEBUG
  printf("A %d %d %d\n", warpIdx, laneIdx, warpPrefix);
#endif

  // B. The last thread of each warp stores inclusive
  // prefix sum to the warp’s index in shared memory
  if (laneIdx == warpSize - 1) {
    sdata[warpIdx] = warpPrefix + predicate;
#ifdef DEBUG
    printf("B %d %d\n", warpIdx, sdata[warpIdx]);
#endif
  }

  // C. One warp scans the warp partial sums
  if (idx < warpSize) {
    sdata[idx] = warp_scan(sdata[idx], sdata);
#ifdef DEBUG
    printf("C: %d %d\n", idx, sdata[idx]);
#endif
  }

  // D. Each thread adds prefix sums of warp partial
  // sums to its own intra−warp prefix sums
  return warpPrefix + sdata[warpIdx];
}
extern "C"

void binary_scan(
        int * g_odata,
  const int * g_idata)
{
    #pragma HLS INTERFACE m_axi port=g_odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE m_axi port=g_odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        int i = _tid_x;
        g_odata[i] = block_binary_prefix_sums(g_idata[i]);

    }
HLS PIPELINE II=1

        int i = _tid_x;
        g_odata[i] = block_binary_prefix_sums(g_idata[i]);

    }
}


// --- from main.cu ---
__inline__ int warp_scan(int val, volatile int *s_data)
{
  // initialize shared memory accessed by each warp with zeros
  int idx = 2 * _tid_x - (_tid_x & (warpSize-1));
  s_data[idx] = 0;
  idx += warpSize;
  int t = s_data[idx] = val;
  s_data[idx] = t += s_data[idx - 1];
  s_data[idx] = t += s_data[idx - 2];
  s_data[idx] = t += s_data[idx - 4];
  s_data[idx] = t += s_data[idx - 8];
  s_data[idx] = t += s_data[idx -16];
  return s_data[idx-1];
}

__inline__ unsigned int lanemask_lt()
{
  const unsigned int lane = _tid_x & (warpSize-1);
  return (1 << (lane)) - 1;
}

__inline__ unsigned int binary_warp_scan(bool p)
{
  const unsigned int mask = lanemask_lt();
  unsigned int b = __ballot(p);
  return __builtin_popcount(b & mask);
}

bool valid(int x) {
  return x > 0;
}

__inline__ int block_binary_prefix_sums(int x)
{
  // 2 x warpIdx's upper bound (1024/32)
  int sdata[64];

  bool predicate = valid(x);

  // A. Compute exclusive prefix sums within each warp
  int warpPrefix = binary_warp_scan(predicate);
  int idx = _tid_x;
  int warpIdx = idx / warpSize;
  int laneIdx = idx & (warpSize - 1);
#ifdef DEBUG
  printf("A %d %d %d\n", warpIdx, laneIdx, warpPrefix);
#endif

  // B. The last thread of each warp stores inclusive
  // prefix sum to the warp’s index in shared memory
  if (laneIdx == warpSize - 1) {
    sdata[warpIdx] = warpPrefix + predicate;
#ifdef DEBUG
    printf("B %d %d\n", warpIdx, sdata[warpIdx]);
#endif
  }

  // C. One warp scans the warp partial sums
  if (idx < warpSize) {
    sdata[idx] = warp_scan(sdata[idx], sdata);
#ifdef DEBUG
    printf("C: %d %d\n", idx, sdata[idx]);
#endif
  }

  // D. Each thread adds prefix sums of warp partial
  // sums to its own intra−warp prefix sums
  return warpPrefix + sdata[warpIdx];
}

void binary_scan(
        int * g_odata,
  const int * g_idata)
{
  int i = _tid_x;
  g_odata[i] = block_binary_prefix_sums(g_idata[i]);
}
