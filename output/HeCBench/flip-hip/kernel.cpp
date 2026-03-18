#include "kernel.h"

// --- from main.cu ---
extern "C"
void flip_kernel(
    const scalar_t* in_tensor,
          scalar_t* out_tensor,
    int64_t  n,
    const int64_t* flip_dims,
    const int64_t  flip_dims_size,
    const int64_t* strides,
    const int64_t* strides_contiguous,
    const int64_t* shape,
    const int64_t  total_dims)
{
    #pragma HLS INTERFACE m_axi port=in_tensor offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out_tensor offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=flip_dims offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=flip_dims_size
    #pragma HLS INTERFACE m_axi port=strides offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=strides_contiguous offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=shape offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=total_dims
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int64_t linear_index = _bid_x * BLOCK_DIM_X + _tid_x;

            if (linear_index >= n) return;

            int64_t cur_indices = linear_index;
            int64_t rem = 0;
            int64_t dst_offset = 0;

            for (int64_t i = 0; i < total_dims; i++) {
            int64_t temp = cur_indices;
            cur_indices = cur_indices / strides_contiguous[i];
            rem = temp - cur_indices * strides_contiguous[i];
            for (int64_t j = 0; j < flip_dims_size; j++) {
            // flip the indices if it is in flip_dims
            if (i == flip_dims[j]) {
            cur_indices = shape[i] - 1 - cur_indices;
            }
            }
            dst_offset += cur_indices * strides[i];
            cur_indices = rem;
            }
            out_tensor[linear_index] = in_tensor[dst_offset];

        }
    }
}
