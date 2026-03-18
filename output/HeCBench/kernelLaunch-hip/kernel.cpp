#include "kernel.h"

// --- from main.cu ---
extern "C"
void KernelWithSmallArgs(SmallKernelArgs args, char* out) {
    #pragma HLS INTERFACE s_axilite port=args
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1
            DO_NOT_OPTIMIZE_AWAY;
        }
    }
}
extern "C"

void KernelWithMediumArgs(MediumKernelArgs args, char* out) {
    #pragma HLS INTERFACE s_axilite port=args
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1
            DO_NOT_OPTIMIZE_AWAY;
        }
    }
}
extern "C"

void KernelWithLargeArgs(LargeKernelArgs args, char* out) {
    #pragma HLS INTERFACE s_axilite port=args
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1
            DO_NOT_OPTIMIZE_AWAY;
        }
    }
}
