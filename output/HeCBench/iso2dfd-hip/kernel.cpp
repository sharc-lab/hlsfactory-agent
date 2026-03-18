#include "kernel.h"

// --- from iso2dfd.cu ---
extern "C"
void iso_2dfd_kernel(
        float* next,
  const float* prev,
  const float* vel, 
  const float dtDIVdxy, const size_t nRows, const size_t nCols)
{
    #pragma HLS INTERFACE m_axi port=next offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=prev offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=vel offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=dtDIVdxy
    #pragma HLS INTERFACE s_axilite port=nRows
    #pragma HLS INTERFACE s_axilite port=nCols
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Compute global id
                    // We can use the get.global.id() function of the item variable
                    //   to compute global id. The 2D array is laid out in memory in row major
                    //   order.
                    size_t gidCol = BLOCK_DIM_X * _bid_x + _tid_x;
                    size_t gidRow = BLOCK_DIM_Y * _bid_y + _tid_y;

                    if (gidRow < nRows && gidCol < nCols) {

                    size_t gid = (gidRow)*nCols + gidCol;

                    // Computation to solve wave equation in 2D
                    // First check if gid is inside the effective grid (not in halo)
                    if ((gidCol >= HALF_LENGTH && gidCol < nCols - HALF_LENGTH) &&
                    (gidRow >= HALF_LENGTH && gidRow < nRows - HALF_LENGTH)) {
                    // Stencil code to update grid point at position given by global id (gid)
                    // New time step for grid point is computed based on the values of the
                    //    the immediate neighbors in both the horizontal and vertical
                    //    directions, as well as the value of grid point at a previous time step
                    float value = 0.f;
                    value += prev[gid + 1] - 2.f * prev[gid] + prev[gid - 1];
                    value += prev[gid + nCols] - 2.f * prev[gid] + prev[gid - nCols];
                    value *= dtDIVdxy * vel[gid];
                    next[gid] = 2.f * prev[gid] - next[gid] + value;
                    }
                    }

                }
            }
        }
    }
}
