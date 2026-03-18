#include "kernel.h"

// --- from main.cu ---
extern "C"
void memcpy_kernel(int *dst, int *src, size_t n, bool wait) {
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=wait
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int num = GRID_DIM_X * BLOCK_DIM_X;
            int id = BLOCK_DIM_X * _bid_x + _tid_x;

            for (size_t i = id; i < n / sizeof(int); i += num) {
            int v = src[i];
            if (wait) {
            while (v--)
            dst[i] = v;
            }
            dst[i] = src[i];
            }

        }
    }
}
