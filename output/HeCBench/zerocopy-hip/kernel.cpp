#include "kernel.h"

// --- from main.cu ---
extern "C"
void vectorAddGPU(float * a,
                             float * b,
                             float * c,
                             int N) 
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int idx = _bid_x * BLOCK_DIM_X + _tid_x;
            if (idx < N) {
            c[idx] = a[idx] + b[idx];
            }

        }
    }
}
