#include "kernel.h"

// --- from collectives.cu ---
extern "C"
void kernel_add(const float* x, const float* y, const int N, float* out) {
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            for (int i = _bid_x * BLOCK_DIM_X + _tid_x; i < N; i += BLOCK_DIM_X * GRID_DIM_X) {
            out[i] = x[i] + y[i];
            }

        }
    }
}
