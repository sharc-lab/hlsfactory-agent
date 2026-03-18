#include "kernel.h"

// --- from main.cu ---
extern "C"
void incKernel(int *g_out, const int *g_in, int N, int inner_reps) {
    #pragma HLS INTERFACE m_axi port=g_out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_in offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=inner_reps
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int idx = _bid_x * BLOCK_DIM_X + _tid_x;

            if (idx < N) {
            for (int i = 0; i < inner_reps; ++i) {
            g_out[idx] = (i == 0 ? g_in[idx] : g_out[idx]) + 1;
            }
            }

        }
    }
}
