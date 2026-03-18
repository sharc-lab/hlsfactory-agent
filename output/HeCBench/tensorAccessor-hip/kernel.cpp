#include "kernel.h"

// --- from main.cu ---
extern "C"
void tensor_packed_accessor_kernel (
    PackedTensorAccessor64<float, 1, RestrictPtrTraits> r,
    PackedTensorAccessor64<float, 2, RestrictPtrTraits> m,
    PackedTensorAccessor64<float, 1, RestrictPtrTraits> v)
{
    #pragma HLS INTERFACE s_axilite port=PackedTensorAccessor64<float
    #pragma HLS INTERFACE s_axilite port=1
    #pragma HLS INTERFACE s_axilite port=r
    #pragma HLS INTERFACE s_axilite port=PackedTensorAccessor64<float
    #pragma HLS INTERFACE s_axilite port=2
    #pragma HLS INTERFACE s_axilite port=m
    #pragma HLS INTERFACE s_axilite port=PackedTensorAccessor64<float
    #pragma HLS INTERFACE s_axilite port=1
    #pragma HLS INTERFACE s_axilite port=v
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int64_t i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i < r.size(0)) {
            float val = 0.0f;
            for (int64_t j = 0; j < m.size(1); j++) {
            val += m[i][j] * v[j];
            }
            r[i] = val;
            }

        }
    }
}
extern "C"

void raw_accessor_kernel (
    const int64_t nrow,
    const int64_t ncol,
          float * r,
    const float * m,
    const float * v)
{
    #pragma HLS INTERFACE s_axilite port=nrow
    #pragma HLS INTERFACE s_axilite port=ncol
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int64_t i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i < nrow) {
            float val = 0.0f;
            for (int64_t j = 0; j < ncol; j++) {
            val += m[i * ncol + j] * v[j];
            }
            r[i] = val;
            }

        }
    }
}
