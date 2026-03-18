#include "kernel.h"

// --- from main.cu ---
extern "C"
void perfKernel()
{
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int gid = _bid_x*BLOCK_DIM_X + _tid_x ;
            assert(gid <= BLOCK_DIM_X * GRID_DIM_X) ;
            int s = 0;
            for (int n = 1; n <= gid; n++) {
            s++; assert(s <= gid);
            }

        }
    }
}
extern "C"

void perfKernel2()
{
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int gid = _bid_x*BLOCK_DIM_X + _tid_x ;
            int s = 0;
            for (int n = 1; n <= gid; n++) {
            s++; assert(s <= gid);
            }

        }
    }
}
