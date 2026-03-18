#include "kernel.h"

// --- from main.cu ---
extern "C"
void AoSKernel(const AppleTree * trees, 
               int * outBuf)
{
    #pragma HLS INTERFACE m_axi port=trees offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=outBuf offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uint gid = _bid_x * BLOCK_DIM_X + _tid_x;
            uint res = 0;
            for(int i = 0; i < treeSize; i++)
            {
            res += trees[gid].apples[i];
            }
            outBuf[gid] = res;

        }
    }
}
extern "C"

void SoAKernel(const ApplesOnTrees * applesOnTrees,
               int * outBuf)
{
    #pragma HLS INTERFACE m_axi port=applesOnTrees offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=outBuf offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uint gid = _bid_x * BLOCK_DIM_X + _tid_x;
            uint res = 0;
            for(int i = 0; i < treeSize; i++)
            {
            res += applesOnTrees[i].trees[gid];
            }
            outBuf[gid] = res;

        }
    }
}
