#include "kernel.h"

// --- from main.cu ---
extern "C"
void init()
{
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            // initializations
            //for step 2
            if (i < nrows){
            cover_row[i] = 0;
            column_of_star_at_row[i] = -1;
            }
            if (i < ncols){
            cover_column[i] = 0;
            row_of_star_at_column[i] = -1;
            }

        }
    }
}

void min_in_rows_warp_reduce(volatile data* sdata, int tid) {
  if (n_threads_reduction >= 64 && n_rows_per_block < 64) sdata[tid] = min(sdata[tid], sdata[tid + 32]);
  if (n_threads_reduction >= 32 && n_rows_per_block < 32) sdata[tid] = min(sdata[tid], sdata[tid + 16]);
  if (n_threads_reduction >= 16 && n_rows_per_block < 16) sdata[tid] = min(sdata[tid], sdata[tid + 8]);
  if (n_threads_reduction >= 8 && n_rows_per_block < 8) sdata[tid] = min(sdata[tid], sdata[tid + 4]);
  if (n_threads_reduction >= 4 && n_rows_per_block < 4) sdata[tid] = min(sdata[tid], sdata[tid + 2]);
  if (n_threads_reduction >= 2 && n_rows_per_block < 2) sdata[tid] = min(sdata[tid], sdata[tid + 1]);
}
