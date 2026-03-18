#include "kernel.h"

// --- from main.cu ---
extern "C"
static void timedReduction(const float *input, float *output, clock_t *timer)
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=timer offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // float shared[2 * BLOCK_DIM_X];
            HIP_DYNAMIC_SHARED( float, shared)

            const int tid = _tid_x;
            const int bid = _bid_x;

            if (tid == 0) timer[bid] = clock();

            // Copy input.
            shared[tid] = input[tid];
            shared[tid + BLOCK_DIM_X] = input[tid + BLOCK_DIM_X];

            // Perform reduction to find minimum.
            for (int d = BLOCK_DIM_X; d > 0; d /= 2)
            {

            if (tid < d)
            {
            float f0 = shared[tid];
            float f1 = shared[tid + d];

            if (f1 < f0)
            {
            shared[tid] = f1;
            }
            }
            }

            // Write result.
            if (tid == 0) output[bid] = shared[0];

            if (tid == 0) timer[bid+GRID_DIM_X] = clock();

        }
    }
}
