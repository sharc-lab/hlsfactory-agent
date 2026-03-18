#include "kernel.h"

// --- from main.cu ---
extern "C"
void add(int n, const float *x, float *y)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            int stride = BLOCK_DIM_X * GRID_DIM_X;
            for (int i = index; i < n; i += stride)
            y[i] += x[i];

        }
    }
}
