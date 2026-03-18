#include "kernel.h"

// --- from main.cu ---
extern "C"
void conv3d_s1(const T *  X,
               const T *  W,
                     T *  Y,
               const int C,
               const int M,
               const int K,
               const int Hin,
               const int Win,
               const int Hout,
               const int Wout,
               const int W_grid)
{
    #pragma HLS INTERFACE m_axi port=X offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=W offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=Y offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=C
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=K
    #pragma HLS INTERFACE s_axilite port=Hin
    #pragma HLS INTERFACE s_axilite port=Win
    #pragma HLS INTERFACE s_axilite port=Hout
    #pragma HLS INTERFACE s_axilite port=Wout
    #pragma HLS INTERFACE s_axilite port=W_grid
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int n = _bid_x;
                        int m = _bid_y;
                        int h = _bid_z / W_grid * TILE_WIDTH + _tid_y;
                        int w = _bid_z % W_grid * TILE_WIDTH + _tid_x;
                        if (h < Hout && w < Wout) {
                        T s = 0;
                        for (int c = 0; c < C; c++) {
                        for (int p = 0; p < K; p++) {
                        for (int q = 0; q < K; q++) {
                        s += X[II(n, c, h+p, w+q)] * W[WI(m, c, p, q)];
                        }
                        }
                        }
                        Y[OI(n, m, h, w)] = s;
                        }

                    }
                }
            }
        }
    }
}
extern "C"

void conv3d_s2(const T *  X,
               const T *  W,
                     T *  Y,
               const int C,
               const int M,
               const int K,
               const int Hin,
               const int Win,
               const int Hout,
               const int Wout,
               const int W_grid)
{
    #pragma HLS INTERFACE m_axi port=X offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=W offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=Y offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=C
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=K
    #pragma HLS INTERFACE s_axilite port=Hin
    #pragma HLS INTERFACE s_axilite port=Win
    #pragma HLS INTERFACE s_axilite port=Hout
    #pragma HLS INTERFACE s_axilite port=Wout
    #pragma HLS INTERFACE s_axilite port=W_grid
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int m = _bid_x;
                        int h = _bid_y / W_grid * TILE_WIDTH + _tid_y;
                        int w = _bid_y % W_grid * TILE_WIDTH + _tid_x;
                        int n = _bid_z;
                        if (h < Hout && w < Wout) {
                        T s = 0;
                        for (int c = 0; c < C; c++) {
                        for (int p = 0; p < K; p++) {
                        for (int q = 0; q < K; q++) {
                        s += X[II(n, c, h+p, w+q)] * W[WI(m, c, p, q)];
                        }
                        }
                        }
                        Y[OI(n, m, h, w)] = s;
                        }

                    }
                }
            }
        }
    }
}
extern "C"

void conv3d_s3(const T *  X,
               const T *  W,
                     T *  Y,
               const int C,
               const int M,
               const int K,
               const int Hin,
               const int Win,
               const int Hout,
               const int Wout,
               const int W_grid)
{
    #pragma HLS INTERFACE m_axi port=X offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=W offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=Y offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=C
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=K
    #pragma HLS INTERFACE s_axilite port=Hin
    #pragma HLS INTERFACE s_axilite port=Win
    #pragma HLS INTERFACE s_axilite port=Hout
    #pragma HLS INTERFACE s_axilite port=Wout
    #pragma HLS INTERFACE s_axilite port=W_grid
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_z = 0; _bid_z < GRID_DIM_Z; _bid_z++) {
        for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
            for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
                for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                    #pragma HLS PIPELINE II=1

                        int h = _bid_x / W_grid * TILE_WIDTH + _tid_y;
                        int w = _bid_x % W_grid * TILE_WIDTH + _tid_x;
                        int n = _bid_y;
                        int m = _bid_z;
                        if (h < Hout && w < Wout) {
                        T s = 0;
                        for (int c = 0; c < C; c++) {
                        for (int p = 0; p < K; p++) {
                        for (int q = 0; q < K; q++) {
                        s += X[II(n, c, h+p, w+q)] * W[WI(m, c, p, q)];
                        }
                        }
                        }
                        Y[OI(n, m, h, w)] = s;
                        }

                    }
                }
            }
        }
    }
}
