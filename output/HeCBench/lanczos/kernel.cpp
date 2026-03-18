#include "kernel.h"

// --- from lanczos.cu ---
extern "C"
void dot_product_kernel(const int N, const T *x, const T *y,
    T *z) {
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=z offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=result complete dim=1
    #pragma HLS ARRAY_PARTITION variable=result complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            T result[THREADS_PER_BLOCK];

            if (index < N) {
            result[_tid_x] = x[index] * y[index];
            } else {
            result[_tid_x] = 0;
            }

            int half = THREADS_PER_BLOCK / 2;
            while (half > 0) {
            if (_tid_x < half) {
            result[_tid_x] += result[_tid_x + half];
            }
            half /= 2;
            }

            if (_tid_x == 0) {
            z[_bid_x] = result[0];
            }

        }
    }
}
extern "C"

void multiply_inplace_kernel(const int N, T *x, const T k) {
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int index = _bid_x * BLOCK_DIM_X + _tid_x;

            if (index < N) {
            x[index] = x[index] * k;
            }

        }
    }
}
extern "C"

void saxpy_inplace_kernel(const int N, T *y, const T *x,
    const T a) {
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=a
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int index = _bid_x * BLOCK_DIM_X + _tid_x;

            if (index < N) {
            y[index] += a * x[index];
            }

        }
    }
}
extern "C"

void warp_multiply_kernel(const int group_size, const int rows,
    const int begin_row, const int *row_ptr, const int *col_ind,
    const T *values, const T *x, T *y) {
    #pragma HLS INTERFACE s_axilite port=group_size
    #pragma HLS INTERFACE s_axilite port=rows
    #pragma HLS INTERFACE s_axilite port=begin_row
    #pragma HLS INTERFACE m_axi port=row_ptr offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=col_ind offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=values offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=result complete dim=1
    #pragma HLS ARRAY_PARTITION variable=result complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            int r = index / group_size + begin_row;
            int lane = index % group_size;
            volatile T result[THREADS_PER_BLOCK];

            result[_tid_x] = 0;
            if (r < rows) {
            int start = row_ptr[r];
            int end = row_ptr[r + 1];
            for (int i = start + lane; i < end; i+= group_size) {
            result[_tid_x] += values[i] * x[col_ind[i]];
            }
            // Threads in a warp are synchronized, so we can do this
            int half = group_size / 2;
            while (half > 0) {
            if (lane < half) {
            result[_tid_x] += result[_tid_x + half];
            }
            half /= 2;
            }
            if (lane == 0) {
            y[r] = result[_tid_x];
            }
            }

        }
    }
}
