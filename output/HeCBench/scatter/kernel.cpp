#include "kernel.h"

// --- from main.cu ---
extern "C"
void scatter_kernel(const scalar_t *src_data,
               const TensorInfo<int64_t, int64_t> index_info,
               scalar_t *out_data, int E, int K, int N, int numel) {
    #pragma HLS INTERFACE m_axi port=src_data offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=TensorInfo<int64_t
    #pragma HLS INTERFACE s_axilite port=index_info
    #pragma HLS INTERFACE m_axi port=out_data offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=E
    #pragma HLS INTERFACE s_axilite port=K
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=numel
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int thread_idx = _bid_x * BLOCK_DIM_X + _tid_x;

            int b = thread_idx / (E * K);
            int k = thread_idx % K;

            if (thread_idx < numel) {
            int offset = IndexToOffset<int64_t, int64_t, -1>::get(
            thread_idx, index_info);
            int64_t idx = index_info.data[offset];

            Reducer<scalar_t, REDUCE>::atomic_write(out_data + b * N * K + idx * K + k,
            src_data[thread_idx]);
            }

        }
    }
}
