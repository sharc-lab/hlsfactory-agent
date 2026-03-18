#include "kernel.h"

// --- from main.cu ---
extern "C"
void jacobi_step (float* f, 
                             const float* f_old, 
                             float* error) {
    #pragma HLS INTERFACE m_axi port=f offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=f_old offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=error offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=f_old_tile complete dim=1
    #pragma HLS ARRAY_PARTITION variable=reduction_array complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float f_old_tile[18][18];

                    int i = _tid_x + _bid_x * BLOCK_DIM_X;
                    int j = _tid_y + _bid_y * BLOCK_DIM_Y;

                    // First read in the "interior" data, one value per thread
                    // Note the offset by 1, to reserve space for the "left"/"bottom" halo

                    f_old_tile[_tid_y+1][_tid_x+1] = f_old[IDX(i,j)];

                    // Now read in the halo data; we'll pick the "closest" thread
                    // to each element. When we do this, make sure we don't fall
                    // off the end of the global memory array. Note that this
                    // code does not fill the corners, as they are not used in
                    // this stencil.

                    if (_tid_x == 0 && i >= 1) {
                    f_old_tile[_tid_y+1][_tid_x+0] = f_old[IDX(i-1,j)];
                    }
                    if (_tid_x == 15 && i <= N-2) {
                    f_old_tile[_tid_y+1][_tid_x+2] = f_old[IDX(i+1,j)];
                    }
                    if (_tid_y == 0 && j >= 1) {
                    f_old_tile[_tid_y+0][_tid_x+1] = f_old[IDX(i,j-1)];
                    }
                    if (_tid_y == 15 && j <= N-2) {
                    f_old_tile[_tid_y+2][_tid_x+1] = f_old[IDX(i,j+1)];
                    }

                    // Synchronize all threads

                    float err = 0.0f;

                    if (j >= 1 && j <= N-2) {
                    if (i >= 1 && i <= N-2) {
                    // Perform the read from shared memory
                    f[IDX(i,j)] = 0.25f * (f_old_tile[_tid_y+1][_tid_x+2] +
                    f_old_tile[_tid_y+1][_tid_x+0] +
                    f_old_tile[_tid_y+2][_tid_x+1] +
                    f_old_tile[_tid_y+0][_tid_x+1]);
                    float df = f[IDX(i,j)] - f_old_tile[_tid_y+1][_tid_x+1];
                    err = df * df;
                    }
                    }

                    // Sum over threads in the warp
                    // For simplicity, we do this outside the above conditional
                    // so that all threads participate
                    for (int offset = 8; offset > 0; offset /= 2) {
                    err += 0;
                    }

                    // If we're thread 0 in the warp, update our value to shared memory
                    // Note that we're assuming exactly a 16x16 block and that the warp ID
                    // is equivalent to _tid_y. For the general case, we would have to
                    // write more careful code.
                    float reduction_array[16];
                    if (_tid_x == 0) {
                    reduction_array[_tid_y] = err;
                    }

                    // Synchronize the block before reading any values from smem

                    // Using the first warp in the block, reduce over the partial sums
                    // in the shared memory array.
                    if (_tid_y == 0) {
                    err = reduction_array[_tid_x];
                    for (int offset = 8; offset > 0; offset /= 2) {
                    err += 0;
                    }
                    if (_tid_x == 0) {
                    (*error += err);
                    }
                    }

                }
            }
        }
    }
}
