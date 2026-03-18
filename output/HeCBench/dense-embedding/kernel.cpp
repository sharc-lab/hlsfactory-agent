#include "kernel.h"

// --- from main.cu ---
extern "C"
void dense_esuhm(
    const T*  input,
    const T*  dense,
          T* output,
    int embedding_dim,
    const int*  offset)
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dense offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=embedding_dim
    #pragma HLS INTERFACE m_axi port=offset offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int batch_idx  = _bid_x; // each batch is handled by a block
            const int grain_size = BLOCK_DIM_X;
            const int tid = _tid_x;
            const int range = offset[batch_idx + 1] - offset[batch_idx];
            for (int idx = tid; idx < embedding_dim; idx += grain_size) {
            const T dense_elem = dense[batch_idx * embedding_dim + idx];
            for (int nested_idx = idx; nested_idx < range; nested_idx += embedding_dim) {
            output[offset[batch_idx] + nested_idx] = input[offset[batch_idx] + nested_idx] + dense_elem;
            }
            }

        }
    }
}
extern "C"

void dense_esuhm2(
    const T*  input,
    const T*  dense,
          T* output,
    int embedding_dim,
    const int*  offset)
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dense offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=embedding_dim
    #pragma HLS INTERFACE m_axi port=offset offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int batch_idx  = _bid_x;
            const int start = offset[batch_idx];
            const int range = offset[batch_idx + 1] - start;
            for (int idx = _tid_x; idx < embedding_dim; idx += BLOCK_DIM_X) {
            const T dense_elem = dense[batch_idx * embedding_dim + idx];
            for (int nested_idx = idx; nested_idx < range; nested_idx += embedding_dim) {
            output[start + nested_idx] = input[start + nested_idx] + dense_elem;
            }
            }

        }
    }
}
extern "C"

void dense_esuhm3(
    const T*  input,
    const T*  dense,
          T* output,
    int embedding_dim,
    const int*  offset)
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=dense offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=embedding_dim
    #pragma HLS INTERFACE m_axi port=offset offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int batch_idx = _bid_x;
            const int start = offset[batch_idx];
            const int range = offset[batch_idx + 1] - start;

            for (int s = 0; s < range; s += BLOCK_DIM_X) {
            int idx = s + _tid_x;
            if (idx < range) {
            T input_elem = input[start + idx];
            T dense_elem = dense[batch_idx * embedding_dim + idx % embedding_dim];
            output[start + idx] = input_elem + dense_elem;
            }
            }

        }
    }
}
