#include "kernel.h"

// --- from stencil_1d.cu ---
extern "C"
void stencil_1d(const int * in, int * out)
{
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=temp complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int temp[BLOCK_SIZE + 2 * RADIUS];
            int gindex = _tid_x + _bid_x * BLOCK_DIM_X;
            int lindex = _tid_x + RADIUS;

            // Read input elements into shared memory
            temp[lindex] = in[gindex];

            // At both end of a block, the sliding window moves beyond the block boundary.
            if (_tid_x < RADIUS) {
            temp[lindex - RADIUS] = (gindex < RADIUS) ? 0 : in[gindex - RADIUS];
            temp[lindex + BLOCK_SIZE] = in[gindex + BLOCK_SIZE];
            }

            // Synchronize (ensure all the threads will be completed before continue)

            // Apply the 1D stencil
            int result = 0;
            for (int offset = -RADIUS ; offset <= RADIUS ; offset++)
            result += temp[lindex + offset];

            // Store the result
            out[gindex] = result;

        }
    }
}
