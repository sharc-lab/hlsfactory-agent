#include "kernel.h"

// --- from main.cu ---
extern "C"
void unfold_backward_elementwise_kernel(int total_n_elems, func_t f) {
    #pragma HLS INTERFACE s_axilite port=total_n_elems
    #pragma HLS INTERFACE s_axilite port=f
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            constexpr int total_work_block = n_threads * n_elems_per_thread;
            int idx = total_work_block * _bid_x + _tid_x;
            #pragma unroll
            for (int i = 0; i < n_elems_per_thread; ++i) {
            if (idx < total_n_elems) {
            f(idx);
            idx += n_threads;
            }
            }

        }
    }
}
