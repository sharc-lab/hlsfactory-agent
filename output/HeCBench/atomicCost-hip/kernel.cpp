#include "kernel.h"

// --- from main.cu ---
extern "C"
void woAtomicOnGlobalMem(T* result, int size)
{
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            for ( unsigned int i = tid * size; i < (tid + 1) * size; i++){
            result[tid] += i % 2;
            }

        }
    }
}
extern "C"

void wiAtomicOnGlobalMem(T* result, int size)
{
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            for ( unsigned int i = tid * size; i < (tid + 1) * size; i++){
            (result[tid] += i % 2);
            }

        }
    }
}
