#include "kernel.h"

// --- from main.cu ---
extern "C"
void nll_loss_forward_reduce2d_kernel(
    scalar_t*  output,
    scalar_t*  total_weight,
    const scalar_t*  input,
    const index_t*   target,
    const scalar_t*  weights,
    bool size_average,
    int64_t nframe,
    int64_t kdim,
    int64_t ignore_index)
{
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=total_weight offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=target offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=weights offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=size_average
    #pragma HLS INTERFACE s_axilite port=nframe
    #pragma HLS INTERFACE s_axilite port=kdim
    #pragma HLS INTERFACE s_axilite port=ignore_index
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sm_inputs complete dim=1

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        accscalar_t sm_inputs[NLL_LOSS_THREADS],
        acc_weight[NLL_LOSS_THREADS];

        int tid = _tid_x;
        sm_inputs[tid] = static_cast<accscalar_t>(0);
        acc_weight[tid] = static_cast<accscalar_t>(0);

        for (int i = tid; i < nframe; i += NLL_LOSS_THREADS) {
        index_t t = target[i];
        if (t != ignore_index) {
        scalar_t cur_weight =
        weights != nullptr ? weights[t] : static_cast<scalar_t>(1);
        sm_inputs[tid] -= static_cast<accscalar_t>(input[i * kdim + t] * cur_weight);
        acc_weight[tid] += static_cast<accscalar_t>(cur_weight);
        }
        }

        if (tid == 0) {
        accscalar_t output_acc = 0;
        accscalar_t total_weight_acc = 0;
        for (int i = 0; i < NLL_LOSS_THREADS; ++i) {
        output_acc += sm_inputs[i];
        total_weight_acc += acc_weight[i];
        }
        *total_weight = static_cast<scalar_t>(total_weight_acc);
        if (size_average) {
        *output = static_cast<scalar_t>(output_acc / total_weight_acc);
        } else {
        *output = static_cast<scalar_t>(output_acc);
        }
        }

    }
}
