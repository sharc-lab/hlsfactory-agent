#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void sampled_rows_kernel(
  const IdxT* nsamples,
  float* X,
  const IdxT nrows_X,
  const IdxT ncols,
  DataT* background,
  const IdxT nrows_background,
  DataT* dataset,
  const DataT* observation,
  uint64_t seed)
{
    #pragma HLS INTERFACE m_axi port=nsamples offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=X offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=nrows_X
    #pragma HLS INTERFACE s_axilite port=ncols
    #pragma HLS INTERFACE m_axi port=background offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=nrows_background
    #pragma HLS INTERFACE m_axi port=dataset offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=observation offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=seed
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // int tid = _tid_x + _bid_x * BLOCK_DIM_X;
            // see what k this block will generate
            int k_blk = nsamples[_bid_x];

            // First k threads of block generate samples
            if (_tid_x < k_blk) {
            int rand_idx = (int)(LCG_random_double(&seed) * ncols);

            // Since X is initialized to 0, we quickly check for collisions (if k_blk << ncols the likelyhood of collisions is low)
            while (atomicExch(&(X[2 * _bid_x * ncols + rand_idx]), 1) == 1) {
            rand_idx = (int)(LCG_random_double(&seed) * ncols);
            }
            }

            // Each block processes one row of X. Columns are iterated over by BLOCK_DIM_X at a time to ensure data coelescing
            int col_idx = _tid_x;
            while (col_idx < ncols) {
            // Load the X idx for the current column
            int curr_X = (int)X[2 * _bid_x * ncols + col_idx];
            X[(2 * _bid_x + 1) * ncols + col_idx] = 1 - curr_X;

            for (int bg_row_idx = 2 * _bid_x * nrows_background;
            bg_row_idx < 2 * _bid_x * nrows_background + nrows_background;
            bg_row_idx += 1) {
            if (curr_X == 0) {
            dataset[bg_row_idx * ncols + col_idx] =
            background[(bg_row_idx % nrows_background) * ncols + col_idx];
            } else {
            dataset[bg_row_idx * ncols + col_idx] = observation[col_idx];
            }
            }

            for (int bg_row_idx = (2 * _bid_x + 1) * nrows_background;
            bg_row_idx <
            (2 * _bid_x + 1) * nrows_background + nrows_background;
            bg_row_idx += 1) {
            if (curr_X == 0) {
            dataset[bg_row_idx * ncols + col_idx] = observation[col_idx];
            } else {
            // if(_tid_x == 0) printf("tid bg_row_idx: %d %d\n", tid, bg_row_idx);
            dataset[bg_row_idx * ncols + col_idx] =
            background[(bg_row_idx) % nrows_background * ncols + col_idx];
            }
            }

            col_idx += BLOCK_DIM_X;
            }

        }
    }
}
