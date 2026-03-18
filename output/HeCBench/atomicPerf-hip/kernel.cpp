#include "kernel.h"

// --- from main.cu ---
extern "C"
void BlockRangeAtomicOnGlobalMem(T* data, int n)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int tid = (_bid_x * BLOCK_DIM_X) + _tid_x;
            for ( unsigned int i = tid; i < n; i += BLOCK_DIM_X*GRID_DIM_X){
            atomicAdd(data+_tid_x, (T)1);  //arbitrary number to add
            }

        }
    }
}
extern "C"

void WarpRangeAtomicOnGlobalMem(T* data, int n)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int tid = (_bid_x * BLOCK_DIM_X) + _tid_x;
            for ( unsigned int i = tid; i < n; i += BLOCK_DIM_X*GRID_DIM_X){
            atomicAdd(data+(i & 0x1F), (T)1); //arbitrary number to add
            }

        }
    }
}
extern "C"

void SingleRangeAtomicOnGlobalMem(T* data, int offset, int n)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=offset
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int tid = (_bid_x * BLOCK_DIM_X) + _tid_x;
            for ( unsigned int i = tid; i < n; i += BLOCK_DIM_X*GRID_DIM_X){
            atomicAdd(data+offset, (T)1);    //arbitrary number to add
            }

        }
    }
}
extern "C"

void BlockRangeAtomicOnSharedMem(T* data, int n)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=smem_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=smem_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=smem_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            T smem_data[BLOCK_SIZE];
            unsigned int tid = (_bid_x * BLOCK_DIM_X) + _tid_x;
            for ( unsigned int i = tid; i < n; i += BLOCK_DIM_X*GRID_DIM_X){
            atomicAdd(smem_data+_tid_x, (T)1);
            }
            if (_bid_x == GRID_DIM_X)
            data[_tid_x] = smem_data[_tid_x];

        }
    }
}
extern "C"

void WarpRangeAtomicOnSharedMem(T* data, int n)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=smem_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=smem_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=smem_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            T smem_data[32];
            unsigned int tid = (_bid_x * BLOCK_DIM_X) + _tid_x;
            for ( unsigned int i = tid; i < n; i += BLOCK_DIM_X*GRID_DIM_X){
            atomicAdd(smem_data+(i & 0x1F), (T)1);
            }
            if (_bid_x == GRID_DIM_X && _tid_x < 0x1F)
            data[_tid_x] = smem_data[_tid_x];

        }
    }
}
extern "C"

void SingleRangeAtomicOnSharedMem(T* data, int offset, int n)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=offset
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=smem_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=smem_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=smem_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            T smem_data[BLOCK_SIZE];
            unsigned int tid = (_bid_x * BLOCK_DIM_X) + _tid_x;
            for ( unsigned int i = tid; i < n; i += BLOCK_DIM_X*GRID_DIM_X){
            atomicAdd(smem_data + offset, (T)1);
            }
            if (_bid_x == GRID_DIM_X && _tid_x == 0)
            data[_tid_x] = smem_data[_tid_x];

        }
    }
}
