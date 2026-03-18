#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void mv_csr(const size_t num_rows,
                       const size_t *row_indices,
                       const size_t *col_indices,
                       const REAL *values,
                       const REAL *x,
                             REAL *y)
{
    #pragma HLS INTERFACE s_axilite port=num_rows
    #pragma HLS INTERFACE m_axi port=row_indices offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=col_indices offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=values offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                size_t i = _bid_x * BLOCK_DIM_X + _tid_x;
                if (i < num_rows) {
                size_t row_start = row_indices[i];
                size_t row_end = row_indices[i+1];

                REAL temp = 0;
                for(size_t n = row_start; n < row_end; n++){
                temp += values[n] * x[col_indices[n]];
                }
                y[i] = temp;
                }

            }
        }
    }
}
extern "C"

void vector_mv_csr(const size_t num_rows,
                              const size_t *row_indices,
                              const size_t *col_indices,
                              const REAL *values,
                              const REAL *x,
                                    REAL *y)
{
    #pragma HLS INTERFACE s_axilite port=num_rows
    #pragma HLS INTERFACE m_axi port=row_indices offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=col_indices offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=values offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                size_t m = _bid_x * BLOCK_DIM_Y + _tid_y;
                if (m < num_rows) {
                size_t row_start = row_indices[m];
                size_t row_end = row_indices[m+1];

                REAL temp = 0;
                for(size_t n = row_start + _tid_x; n < row_end; n += BS){
                temp += values[n] * x[col_indices[n]];
                }
                #pragma unroll
                for (int i = BS >> 1; i > 0; i >>= 1)
                temp += __shfl_down(temp, i, BS);
                // temp += 0;

                y[m] = temp;
                }

            }
        }
    }
}
extern "C"

void mv_dense(const size_t num_rows, const REAL* matrix, const REAL* x, REAL* y)
{
    #pragma HLS INTERFACE s_axilite port=num_rows
    #pragma HLS INTERFACE m_axi port=matrix offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
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
}
