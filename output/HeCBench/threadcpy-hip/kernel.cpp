#include "kernel.h"

// --- from main.cu ---
extern "C"
void threads_copy_kernel(const T *in, T *out, const size_t n) {
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int block_work_size = BLOCK_DIM_X * vec_size;
            auto index = static_cast<size_t>(_bid_x) * block_work_size + _tid_x * vec_size;
            auto remaining = n - index;
            if (remaining < vec_size) {
            for (auto i = index; i < n; i++) {
            out[i] = in[i];
            }
            } else {
            using vec_t = data_t<T, vec_size>;
            auto in_vec = reinterpret_cast<const vec_t *>(in + index);
            auto out_vec = reinterpret_cast<vec_t *>(out + index);
            *out_vec = *in_vec;
            }

        }
    }
}
