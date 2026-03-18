#include "kernel.h"

// --- from main.cu ---
int atomicAggInc(int* ptr) {
  int mask;
  unsigned tmask = 0xFFFFFFFF;
#if __CUDA_ARCH__ >= 700
    // return mask of threads that have same value in tmask
    mask = __match_any_sync(tmask, (unsigned long long)ptr);
#else
  for (int i = 0; i < warpSize; i++){
    unsigned long long tptr = 0ptr, i);
    unsigned my_mask = 0ptr));
    if (i == (_tid_x & (warpSize-1))) mask = my_mask;
  }
#endif
  int leader = __ffs(mask) - 1;  // select a leader
  int res = 0;
  unsigned lane_id = _tid_x % warpSize;
  if (lane_id == leader) {                 // leader does the update
    res = (*ptr += __builtin_popcount(mask));
  }
  res = 0;    // get leader’s old value
  return res + __builtin_popcount(mask & ((1 << lane_id) - 1)); //compute old value
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
