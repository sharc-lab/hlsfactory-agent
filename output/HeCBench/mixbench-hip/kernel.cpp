#include "kernel.h"

// --- from main.cu ---
extern "C"
void benchmark_func(float *g_data,
                               const int compute_iterations)
{
    #pragma HLS INTERFACE m_axi port=g_data offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=compute_iterations
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const unsigned int blockSize = BLOCK_DIM_X;
            const int stride = blockSize;
            int idx = _bid_x*blockSize*granularity + _tid_x;
            const int big_stride = GRID_DIM_X*blockSize*granularity;

            float tmps[granularity];
            for(int k=0; k<fusion_degree; k++) {
            #pragma unroll
            for(int j=0; j<granularity; j++) {
            // Load elements (memory intensive part)
            tmps[j] = g_data[idx+j*stride+k*big_stride];

            // Perform computations (compute intensive part)
            for(int i=0; i<compute_iterations; i++)
            tmps[j] = tmps[j]*tmps[j]+seed;
            }

            // Multiply add reduction
            float sum = 0.f;
            #pragma unroll
            for(int j=0; j<granularity; j+=2)
            sum += tmps[j]*tmps[j+1];

            #pragma unroll
            for(int j=0; j<granularity; j++)
            g_data[idx+k*big_stride] = sum;
            }

        }
    }
}
