#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void fwtBatch2Kernel(
          float * d_Output,
    const float * d_Input,
    int stride)
{
    #pragma HLS INTERFACE m_axi port=d_Output offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_Input offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=stride
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                const int pos = _bid_x * BLOCK_DIM_X + _tid_x;
                const int   N = BLOCK_DIM_X *  GRID_DIM_X * 4;

                const float *d_Src = d_Input  + _bid_y * N;
                float *d_Dst = d_Output + _bid_y * N;

                int lo = pos & (stride - 1);
                int i0 = ((pos - lo) << 2) + lo;
                int i1 = i0 + stride;
                int i2 = i1 + stride;
                int i3 = i2 + stride;

                float D0 = d_Src[i0];
                float D1 = d_Src[i1];
                float D2 = d_Src[i2];
                float D3 = d_Src[i3];

                float T;
                T = D0;
                D0        = D0 + D2;
                D2        = T - D2;
                T = D1;
                D1        = D1 + D3;
                D3        = T - D3;
                T = D0;
                d_Dst[i0] = D0 + D1;
                d_Dst[i1] = T - D1;
                T = D2;
                d_Dst[i2] = D2 + D3;
                d_Dst[i3] = T - D3;

            }
        }
    }
}
extern "C"

void modulateKernel(      float * d_A, 
                    const float * d_B, 
                          int N,
                    const float rcpN)
{
    #pragma HLS INTERFACE m_axi port=d_A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_B offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=rcpN
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int        tid = _bid_x * BLOCK_DIM_X + _tid_x;
                int numThreads = BLOCK_DIM_X * GRID_DIM_X;

                for (int pos = tid; pos < N; pos += numThreads)
                {
                d_A[pos] *= d_B[pos] * rcpN;
                }

            }
        }
    }
}
