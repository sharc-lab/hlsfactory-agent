#include "kernel.h"

// --- from gaussianElim.cu ---
extern "C"
void fan1 (const float* a,
            float* m,
      const int size, const int t)
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=t
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int globalId = BLOCK_DIM_X * _bid_x + _tid_x;
                    if (globalId < size-1-t) {
                    m[size * (globalId + t + 1)+t] =
                    a[size * (globalId + t + 1) + t] / a[size * t + t];
                    }

                }
            }
        }
    }
}
extern "C"

void fan2 (float* a,
      float* b,
      const float* m,
      const int size, const int t)
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=t
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int globalIdy = BLOCK_DIM_X * _bid_x + _tid_x;
                    int globalIdx = BLOCK_DIM_Y * _bid_y + _tid_y;
                    if (globalIdx < size-1-t && globalIdy < size-t) {
                    a[size*(globalIdx+1+t)+(globalIdy+t)] -=
                    m[size*(globalIdx+1+t)+t] * a[size*t+(globalIdy+t)];

                    if(globalIdy == 0){
                    b[globalIdx+1+t] -=
                    m[size*(globalIdx+1+t)+(globalIdy+t)] * b[t];
                    }
                    }

                }
            }
        }
    }
}
