#include "kernel.h"

// --- from FDTD3dGPU.cu ---
extern "C"
void finite_difference(
        float* output,
  const float* input,
  const float* coef, 
  const int dimx, const int dimy, const int dimz,
  const int padding)
{
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=coef offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=dimx
    #pragma HLS INTERFACE s_axilite port=dimy
    #pragma HLS INTERFACE s_axilite port=dimz
    #pragma HLS INTERFACE s_axilite port=padding
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float tile[k_blockDimMaxY + 2 * k_radius_default][k_blockDimMaxX + 2 * k_radius_default];

                    bool valid = true;
                    const int ltidx = _tid_x;
                    const int ltidy = _tid_y;
                    const int workx = BLOCK_DIM_X;
                    const int worky = BLOCK_DIM_Y;
                    const int gtidx = _bid_x * workx + ltidx;
                    const int gtidy = _bid_y * worky + ltidy;

                    const int stride_y = dimx + 2 * k_radius_default;
                    const int stride_z = stride_y * (dimy + 2 * k_radius_default);

                    int inputIndex  = 0;
                    int outputIndex = 0;

                    // Advance inputIndex to start of inner volume
                    inputIndex += k_radius_default * stride_y + k_radius_default + padding;

                    // Advance inputIndex to target element
                    inputIndex += gtidy * stride_y + gtidx;

                    float infront[k_radius_default];
                    float behind[k_radius_default];
                    float current;

                    const int tx = ltidx + k_radius_default;
                    const int ty = ltidy + k_radius_default;

                    if (gtidx >= dimx) valid = false;
                    if (gtidy >= dimy) valid = false;

                    // For simplicity we assume that the global size is equal to the actual
                    // problem size; since the global size must be a multiple of the local size
                    // this means the problem size must be a multiple of the local size (or
                    // padded to meet this constraint).
                    // Preload the "infront" and "behind" data
                    for (int i = k_radius_default - 2 ; i >= 0 ; i--)
                    {
                    behind[i] = input[inputIndex];
                    inputIndex += stride_z;
                    }

                    current = input[inputIndex];
                    outputIndex = inputIndex;
                    inputIndex += stride_z;

                    for (int i = 0 ; i < k_radius_default ; i++)
                    {
                    infront[i] = input[inputIndex];
                    inputIndex += stride_z;
                    }

                    // Step through the xy-planes
                    for (int iz = 0 ; iz < dimz ; iz++)
                    {
                    // Advance the slice (move the thread-front)
                    for (int i = k_radius_default - 1 ; i > 0 ; i--)
                    behind[i] = behind[i - 1];
                    behind[0] = current;
                    current = infront[0];
                    for (int i = 0 ; i < k_radius_default - 1 ; i++)
                    infront[i] = infront[i + 1];
                    infront[k_radius_default - 1] = input[inputIndex];

                    inputIndex  += stride_z;
                    outputIndex += stride_z;

                    // Note that for the work items on the boundary of the problem, the
                    // supplied index when reading the halo (below) may wrap to the
                    // previous/next row or even the previous/next xy-plane. This is
                    // acceptable since a) we disable the output write for these work
                    // items and b) there is at least one xy-plane before/after the
                    // current plane, so the access will be within bounds.

                    // Update the data slice in the local tile
                    // Halo above & below
                    if (ltidy < k_radius_default)
                    {
                    tile[ltidy][tx]                  = input[outputIndex - k_radius_default * stride_y];
                    tile[ltidy + worky + k_radius_default][tx] = input[outputIndex + worky * stride_y];
                    }
                    // Halo left & right
                    if (ltidx < k_radius_default)
                    {
                    tile[ty][ltidx]                  = input[outputIndex - k_radius_default];
                    tile[ty][ltidx + workx + k_radius_default] = input[outputIndex + workx];
                    }
                    tile[ty][tx] = current;

                    // Compute the output value
                    float value = coef[0] * current;
                    for (int i = 1 ; i <= k_radius_default ; i++)
                    {
                    value += coef[i] * (infront[i-1] + behind[i-1] + tile[ty - i][tx] +
                    tile[ty + i][tx] + tile[ty][tx - i] + tile[ty][tx + i]);
                    }

                    // Store the output value
                    if (valid) output[outputIndex] = value;
                    }

                }
            }
        }
    }
}
