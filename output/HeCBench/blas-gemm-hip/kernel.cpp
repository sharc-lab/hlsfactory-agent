#include "kernel.h"

// --- from main.cu ---
extern "C"
void matrix_mul(T *a, T *b, T *c, int M, int K, int N, T alpha, T beta) {
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=K
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=alpha
    #pragma HLS INTERFACE s_axilite port=beta
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int row = _bid_y * TILE_Y + _tid_y;
                    int col = _bid_x * TILE_X + _tid_x;
                    if (row < M && col < N) {
                    T s = 0;
                    for (int k = 0; k < K; k++)
                    s += a[row * K + k] * b[k * N + col];
                    c[row * N + col] = alpha * s + beta * c[row * N + col];
                    }

                }
            }
        }
    }
}
