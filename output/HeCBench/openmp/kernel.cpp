#include "kernel.h"

// --- from main.cu ---
extern "C"
void addConstant(int *g_a, const int b, const int repeat) {
    #pragma HLS INTERFACE m_axi port=g_a offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=b
    #pragma HLS INTERFACE s_axilite port=repeat
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int idx = _bid_x * BLOCK_DIM_X + _tid_x;
            for (int i = 0; i < repeat; i++)
            g_a[idx] += i % b;

        }
    }
}
