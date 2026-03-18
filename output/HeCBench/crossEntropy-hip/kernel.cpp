#include "kernel.h"

// --- from main.cu ---
__inline__ float exponent (float x) { return __expf(x); }

__inline__ double exponent (double x) { return exp(x); }
extern "C"

void loss_bwd (
    const scalar_t*  log_softmax,
    const gscalar_t*  grad_output,
    const gscalar_t*  grad_output_neg,
    const int64_t*  target,
    const scalar_t*  weight,
    const int64_t*  mask,
          gscalar_t*  grad_predict)
{
    #pragma HLS INTERFACE m_axi port=log_softmax offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=grad_output offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=grad_output_neg offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=target offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=weight offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=mask offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=grad_predict offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int local_id_x = _tid_x;
                int group_id_bs = _bid_y;
                int group_id_x = _bid_x;

                int linear_x_id = group_id_x * threadX + local_id_x;

                if (linear_x_id >= H) return;

                int offset2d = group_id_bs * H + linear_x_id;
                int idx = target[offset2d];
                int sum_offset = group_id_bs * W * H + idx * H + linear_x_id;

                gscalar_t tmp_grad;
                if (mask[offset2d])
                tmp_grad = -(grad_output[offset2d] + grad_output_neg[offset2d]);
                else
                tmp_grad = -grad_output[offset2d];

                tmp_grad = tmp_grad * weight[offset2d];

                float sum_value = tmp_grad * log_softmax[sum_offset];

                #pragma unroll
                for (int i = 0; i < W; ++i) {
                int in_offset = group_id_bs * W * H + i * H + linear_x_id;
                float tmp_sfm = exponent(log_softmax[in_offset]) * sum_value;
                float res = 0.f;
                if (i == idx) {
                res = (float)tmp_grad - tmp_sfm;
                }
                else {
                res = -tmp_sfm;
                }
                grad_predict[in_offset] = res;
                }

            }
        }
    }
}
