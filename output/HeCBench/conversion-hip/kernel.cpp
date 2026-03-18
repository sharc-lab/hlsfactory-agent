#include "kernel.h"

// --- from main.cu ---
extern "C"
void cvt (      Td * dst,
          const Ts * src,
          const int nelems)
{
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=nelems
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i < nelems) {
            dst[i] = static_cast<Td>(src[i]);
            }

        }
    }
}
