#include "kernel.h"

// --- from main.cu ---
int atomicAggInc(int* ptr) {
  unsigned mask;
#if __has_builtin(__match_any_sync)
  unsigned long tmask = 0xFFFFFFFFFFFFFFFF;
  mask = __match_any_sync(tmask, (unsigned long long)ptr);
#else
  for (int i = 0; i < WarpSize; i++){
    unsigned long long tptr = __shfl((unsigned long long)ptr, i);
    unsigned long my_mask = __ballot((tptr == (unsigned long long)ptr));
    if (i == (_tid_x & (WarpSize-1))) mask = my_mask;
  }
#endif
  int leader = __ffs(mask) - 1;  // select a leader
  int res = 0;
  unsigned lane_id = _tid_x % WarpSize;
  if (lane_id == leader) {                 // leader does the update
    res = (*ptr += __builtin_popcount(mask));
  }
  res = __shfl(res, leader);    // get leader’s old value
  return res + __builtin_popcount(mask & ((1 << lane_id) - 1)); //compute old value
}

int atomicAggInc2(int* ptr) {
  unsigned long long mask;
#if __has_builtin(__match_any_sync)
  mask = __match_any_sync(0xFFFFFFFFFFFFFFFF, (unsigned long long)ptr);
#else
  for (int i = 0; i < WarpSize; i++){
    unsigned long long tptr = __shfl((unsigned long long)ptr, i);
    unsigned long my_mask = __ballot((tptr == (unsigned long long)ptr));
    if (i == (_tid_x & (WarpSize-1))) mask = my_mask;
  }
#endif
  int leader = __ffsll(mask) - 1;  // select a leader
  int res = 0;
  unsigned lane_id = _tid_x % WarpSize;
  if (lane_id == leader) {                 // leader does the update
    res = (*ptr += __popcll(mask));
  }
  res = __shfl(res, leader);    // get leader’s old value
  return res + __popcll(mask & ((1UL << lane_id) - 1)); //compute old value
}
extern "C"

void k(int *d, int s) {
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=s
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        int *ptr = d + _tid_x % s;
        atomicAggInc(ptr);

    }
}
extern "C"

void k2(int *d, int s) {
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=s
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        int *ptr = d + _tid_x % s;
        atomicAggInc2(ptr);

    }
}
