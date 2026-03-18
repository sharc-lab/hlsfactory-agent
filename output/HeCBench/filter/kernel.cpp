#include "kernel.h"

// --- from main.cu ---
int atomicAggInc(int *ctr) {

  int warp_res = 0;
  if(g.thread_rank() == 0)
    warp_res = (*ctr += g.size());
  return g.shfl(warp_res, 0) + g.thread_rank();
}
extern "C"

void filter2 (int * dst,
              int * nres,
              const int* src,
              int n)
{
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=nres offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _tid_x + _bid_x * BLOCK_DIM_X;
            if(i < n && src[i] > 0)
            dst[atomicAggInc(nres)] = src[i];

        }
    }
}
