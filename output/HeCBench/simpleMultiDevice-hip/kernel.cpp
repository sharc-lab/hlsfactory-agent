#include "kernel.h"

// --- from main.cu ---
extern "C"
void reduceKernel(float *d_Result, const float *d_Input, int N)
{
    #pragma HLS INTERFACE m_axi port=d_Result offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_Input offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int     tid = _bid_x * BLOCK_DIM_X + _tid_x;
            const int threadN = GRID_DIM_X * BLOCK_DIM_X;
            float sum = 0;

            for (int pos = tid; pos < N; pos += threadN)
            sum += d_Input[pos];

            d_Result[tid] = sum;

        }
    }
}
