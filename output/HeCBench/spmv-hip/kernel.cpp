#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void mv_dense(const size_t num_rows, const REAL* matrix, const REAL* x, REAL* y)
{
    #pragma HLS INTERFACE s_axilite port=num_rows
    #pragma HLS INTERFACE m_axi port=matrix offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i < num_rows) {
            REAL temp = 0;
            for (size_t j = 0; j < num_rows; j++) {
            if (matrix[i * num_rows + j] != (REAL)0)
            temp += matrix[i * num_rows + j] * x[j];
            }
            y[i] = temp;
            }

        }
    }
}
