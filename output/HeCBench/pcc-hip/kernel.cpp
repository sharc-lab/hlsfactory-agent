#include "kernel.h"

// --- from device.cu ---
extern "C"
void ker2(const float * cormat, float * upper, int n1, int n)
{
    #pragma HLS INTERFACE m_axi port=cormat offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=upper offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=n1
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t idx = BLOCK_DIM_X*_bid_x+_tid_x;
            if (idx < (size_t)n1 * n) {
            size_t i = idx/n;
            size_t j = idx%n;
            if(i<j && i<n1)
            {
            size_t t = n * i - i * (i+1) / 2 + j - i - 1;
            upper[t]=cormat[j*n1+i];
            }
            }

        }
    }
}
