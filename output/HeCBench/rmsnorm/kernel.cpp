#include "kernel.h"

// --- from main.cu ---
extern "C"
void rmsnorm_fwd_two_scan_kernel(const T * input,
                                 const T * gamma,
                                       T *output,
                                 const int64_t inner_len, const float epsilon)
{
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=gamma offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=inner_len
    #pragma HLS INTERFACE s_axilite port=epsilon
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int BLOCKSIZE = BLOCK_DIM_X;
            const int bid       = _bid_x;
            const int warp_id   = _tid_x / THREADS_PER_WARP;
            const int lane_id   = _tid_x % THREADS_PER_WARP;

            const T *input_ptr  = input + bid * inner_len;
            const T *gamma_ptr  = gamma;
            T       *output_ptr = output + bid * inner_len;

            const int start_offset = warp_id * THREADS_PER_WARP * UNROLL + lane_id * UNROLL;
            T         ld_input_regs[UNROLL];
            float     local_squares_sum = 0.0f;
            for (int64_t offset = start_offset; offset < inner_len; offset += (BLOCKSIZE * UNROLL)) {
            load_data<T, UNROLL>(input_ptr + offset, ld_input_regs);
            #pragma unroll
            for (int i = 0; i < UNROLL; ++i) {
            const float val = static_cast<float>(ld_input_regs[i]);
            local_squares_sum += (val * val);
            }
            }

            const float mean_square = BlockReduce<SumOp, float>(local_squares_sum) / static_cast<float>(inner_len);
            const float norm_factor = rsqrtf(mean_square + epsilon);

            T ld_gamma_regs[UNROLL];
            T st_regs[UNROLL];
            for (int64_t offset = start_offset; offset < inner_len; offset += (BLOCKSIZE * UNROLL)) {
            load_data<T, UNROLL>(input_ptr + offset, ld_input_regs);
            load_data<T, UNROLL>(gamma_ptr + offset, ld_gamma_regs);

            #pragma unroll
            for (int i = 0; i < UNROLL; ++i) {
            float val = static_cast<float>(ld_input_regs[i]) * norm_factor *
            static_cast<float>(ld_gamma_regs[i]);
            st_regs[i] = static_cast<T>(val);
            }
            store_data<T, UNROLL>(output_ptr + offset, st_regs);
            }

        }
    }
}
