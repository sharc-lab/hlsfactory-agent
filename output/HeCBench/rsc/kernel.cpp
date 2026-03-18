#include "kernel.h"

// --- from model_eval.cu ---
extern "C"
void RANSAC_kernel_block(const float * model_param_local,
                         const flowvector * flowvectors,
                         int flowvector_count,
                         int max_iter,
                         int error_threshold,
                         float convergence_threshold,
                         int * g_out_id,
                         int * model_candidate,
                         int * outliers_candidate) {
    #pragma HLS INTERFACE m_axi port=model_param_local offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=flowvectors offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=flowvector_count
    #pragma HLS INTERFACE s_axilite port=max_iter
    #pragma HLS INTERFACE s_axilite port=error_threshold
    #pragma HLS INTERFACE s_axilite port=convergence_threshold
    #pragma HLS INTERFACE m_axi port=g_out_id offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=model_candidate offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=outliers_candidate offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=l_mem complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int l_mem[4096];
            int* outlier_block_count = l_mem;

            const int tx         = _tid_x;
            const int bx         = _bid_x;
            const int num_blocks = GRID_DIM_X;

            float vx_error, vy_error;
            int   outlier_local_count = 0;

            // Each block performs one iteration
            for(int loop_count = bx; loop_count < max_iter; loop_count += num_blocks) {

            // xc=model_param_sh[0], yc=model_param_sh[1], D=model_param_sh[2], R=model_param_sh[3]
            const float *model_param = &model_param_local [4 * loop_count];

            // Wait until CPU computes F-o-F model
            if(tx == 0) {
            outlier_block_count[0] = 0;
            }

            if(model_param[0] == -2011)
            continue;

            // Reset local outlier counter
            outlier_local_count = 0;

            // Compute number of outliers
            for(int i = tx; i < flowvector_count; i += BLOCK_DIM_X) {
            flowvector fvreg = flowvectors[i]; // x, y, vx, vy
            vx_error         = fvreg.x + ((int)((fvreg.x - model_param[0]) * model_param[2]) -
            (int)((fvreg.y - model_param[1]) * model_param[3])) - fvreg.vx;
            vy_error = fvreg.y + ((int)((fvreg.y - model_param[1]) * model_param[2]) +
            (int)((fvreg.x - model_param[0]) * model_param[3])) - fvreg.vy;
            if((fabs(vx_error) >= error_threshold) || (fabs(vy_error) >= error_threshold)) {
            outlier_local_count++;
            }
            }

            (*outlier_block_count += outlier_local_count);

            if(tx == 0) {
            // Compare to threshold
            if(outlier_block_count[0] < flowvector_count * convergence_threshold) {
            int index                 = (*g_out_id += 1);
            model_candidate[index]    = loop_count;
            outliers_candidate[index] = outlier_block_count[0];
            }
            }
            }

        }
    }
}
