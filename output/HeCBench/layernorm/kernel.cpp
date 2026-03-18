#include "kernel.h"

// --- from main.cu ---
extern "C"
void layernorm_forward_kernel0(float*  out, float*  mean, float*  rstd,
                               const float*   inp, const float*   weight,
                               const float*  bias, int N, int C)
{
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=mean offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=rstd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=inp offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=weight offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=bias offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=C
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int BLOCKSIZE = BLOCK_DIM_X;
            const int bid       = _bid_x;
            const int warp_id   = _tid_x / THREADS_PER_WARP;
            const int lane_id   = _tid_x % THREADS_PER_WARP;

            const float *input_ptr  = inp + bid * C;
            const float *weight_ptr = weight;
            const float *bias_ptr   = bias;
            float       *output_ptr = out + bid * C;

            const int start_offset = warp_id * THREADS_PER_WARP * UNROLL + lane_id * UNROLL;
            float     ld_input_regs[UNROLL];
            float     local_sum = 0.0f;
            for (int64_t offset = start_offset; offset < C; offset += (BLOCKSIZE * UNROLL)) {
            load_data<float, UNROLL>(input_ptr + offset, ld_input_regs);
            #pragma unroll
            for (int i = 0; i < UNROLL; ++i) {
            const float val = static_cast<float>(ld_input_regs[i]);
            local_sum += val;
            }
            }

            const float mean_sum = BlockReduce<SumOp, float>(local_sum) / static_cast<float>(C);
            if (_tid_x == 0)
            __stcs(mean + bid, mean_sum);

            local_sum = 0.0f;
            for (int64_t offset = start_offset; offset < C; offset += (BLOCKSIZE * UNROLL)) {
            load_data<float, UNROLL>(input_ptr + offset, ld_input_regs);
            #pragma unroll
            for (int i = 0; i < UNROLL; ++i) {
            const float diff = static_cast<float>(ld_input_regs[i]) - mean_sum;
            local_sum += diff * diff;
            }
            }
            const float mean_square = BlockReduce<SumOp, float>(local_sum) / static_cast<float>(C);
            float s = rsqrtf(mean_square + 1e-5f);
            if(_tid_x == 0 && rstd != nullptr) {
            __stcs(rstd + bid, s);
            }

            float st_regs[UNROLL];
            float ld_weight_regs[UNROLL];
            float ld_bias_regs[UNROLL];

            for (int64_t offset = start_offset; offset < C; offset += (BLOCKSIZE * UNROLL)) {
            load_data<float, UNROLL>(input_ptr + offset, ld_input_regs);
            load_data<float, UNROLL>(weight_ptr + offset, ld_weight_regs);
            load_data<float, UNROLL>(bias_ptr + offset, ld_bias_regs);

            #pragma unroll
            for (int i = 0; i < UNROLL; ++i) {
            float n = (static_cast<float>(ld_input_regs[i]) - mean_sum) * s;
            st_regs[i] = n * static_cast<float>(ld_weight_regs[i]) +
            static_cast<float>(ld_bias_regs[i]);
            }
            store_data<float, UNROLL>(output_ptr + offset, st_regs);
            }

        }
    }
}
extern "C"

void layernorm_forward_kernel1(float*  out, float*  mean, float*  rstd,
                                    const float*   inp, const float*   weight,
                                    const float*  bias, int N, int C) {
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=mean offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=rstd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=inp offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=weight offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=bias offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=C
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            // meta_group_size is the number of warps in a block, and meta_group_rank is the warp index
            int idx = _bid_x * warp.meta_group_size() + warp.meta_group_rank();
            if(idx >= N) {
            return;
            }

            // the row of input that this group of threads is responsible for
            const float* x = inp + idx * C;

            // mean
            float sum = 0.0f;
            for (int i = warp.thread_rank(); i < C; i += warp.size()) {
            sum += x[i];
            }

            float m = sum / C;
            if(warp.thread_rank() == 0 && mean != nullptr) {
            __stcs(mean + idx, m);
            }

            // rstd
            sum = 0.0f;
            for (int i = warp.thread_rank(); i < C; i += warp.size()) {
            float diff = x[i] - m;
            sum += diff * diff;
            }

            float s = rsqrtf(sum / C + 1e-5f);
            if(warp.thread_rank() == 0 && rstd != nullptr) {
            __stcs(rstd + idx, s);
            }

            // final normalization and scaling by weight/bias
            float* o = out + idx * C;
            for (int c = warp.thread_rank(); c < C; c += warp.size()) {
            // load and store using the .cs "streaming" hint to the compiler,
            // indicating that this data will not be reused soon, and can be streamed through the caches
            // this allows the threads to get more cache-hits for the (shared) weight and bias parameters
            float n = s * (__ldcs(x+c) - m);
            __stcs(o+c, n * weight[c] + bias[c]);
            }

        }
    }
}
extern "C"

void layernorm_forward_kernel2(float*  out, float*  mean, float*  rstd,
                                    const float*   inp, const float*   weight,
                                    const float*  bias, int N, int C) {
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=mean offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=rstd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=inp offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=weight offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=bias offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=C
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int idx = _bid_x * warp.meta_group_size() + warp.meta_group_rank();
            if(idx >= N) {
            return;
            }

            // the row of input that this group of threads is responsible for
            const float* x = inp + idx * C;

            // thread coarsening through the row, reduce the sum in series
            float sum = 0.0; // stores sum(x)
            float sum2 = 0.0; // stores sum(x**2)
            for (int i = warp.thread_rank(); i < C; i += warp.size()) {
            float xi = x[i];
            sum += xi;
            sum2 += xi * xi;
            }
            // warp-level reduction at the end

            sum /= C; // mean(x)
            sum2 /= C; // mean(x**2)

            // mean, var, rstd
            float m = sum;
            float var = sum2 - sum * sum;
            float s = rsqrtf(var + 1e-5f);

            // store the mean, no need to cache it
            if(warp.thread_rank() == 0 && mean != nullptr) {
            __stcs(mean + idx, m);
            }
            // store the rstd, no need to cache it
            if(warp.thread_rank() == 0 && rstd != nullptr) {
            __stcs(rstd + idx, s);
            }
            // final normalization and scaling by weight/bias
            float* o = out + idx * C;
            for (int c = warp.thread_rank(); c < C; c += warp.size()) {
            float n = s * (__ldcs(x+c) - m);
            __stcs(o+c, n * weight[c] + bias[c]);
            }

        }
    }
}
