#include "kernel.h"

// --- from main.cu ---
extern "C"
void glu_kernel(
   const int M,
   const int split_dim_size,
   const int N,
   const float* Xdata,
         float* Ydata)
{
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=split_dim_size
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE m_axi port=Xdata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Ydata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if (index >= M * split_dim_size * N) return;

            const int xOffset = 2 * split_dim_size * N;
            const int yOffset = split_dim_size * N;
            const int i = index / split_dim_size / N;
            const int j = index / N % split_dim_size;
            const int k = index % N;
            const float x1 = Xdata[i * xOffset + j * N + k];
            const float x2 = Xdata[i * xOffset + (j + split_dim_size) * N + k];
            Ydata[i * yOffset + j * N + k] = x1 * (1.f / (1.f + expf(-x2)));

        }
    }
}
