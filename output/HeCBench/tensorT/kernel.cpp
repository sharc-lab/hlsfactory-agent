#include "kernel.h"

// --- from main.cu ---
extern "C"
void tensor_transpose(
    const int dim_input, 
    const int dim_output, 
    const int nblocks, 
    const int tile_size,
    const int *shape_input, 
    const int *shape_output, 
    const float *shape_input_r, 
    const float *shape_output_r, 
    const int *stride_input,
    const int *stride_output_local, 
    const int *stride_output_global,
    const double *input, 
    double *output) 
{
    #pragma HLS INTERFACE s_axilite port=dim_input
    #pragma HLS INTERFACE s_axilite port=dim_output
    #pragma HLS INTERFACE s_axilite port=nblocks
    #pragma HLS INTERFACE s_axilite port=tile_size
    #pragma HLS INTERFACE m_axi port=shape_input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=shape_output offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=shape_input_r offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=shape_output_r offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=stride_input offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=stride_output_local offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=stride_output_global offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=tile complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            double tile[TILE_SIZE];

            for (int block_idx = _bid_x; block_idx < nblocks; block_idx += GRID_DIM_X) {
            int it = block_idx, im = 0, offset1 = 0;
            for (int i = 0; i < dim_input; i++) {
            im = it * shape_input_r[i];  // replace division with multiplication
            offset1 += stride_input[i] * (it - im * shape_input[i]);
            it = im;
            }

            for (int i = _tid_x; i < tile_size; i += BLOCK_DIM_X) {
            tile[i] = input[i + block_idx * tile_size];
            }

            for (int i = _tid_x; i < tile_size; i += BLOCK_DIM_X) {
            it = i;
            int offset2 = 0, local_offset = 0;
            for (int j = 0; j < dim_output; j++) {
            im = it * shape_output_r[j];  // replace division with multiplication
            int tmp = it - im * shape_output[j];
            offset2 += stride_output_global[j] * tmp;
            local_offset += stride_output_local[j] * tmp;
            it = im;
            }
            output[offset1 + offset2] = tile[local_offset];
            }
            }

        }
    }
}
