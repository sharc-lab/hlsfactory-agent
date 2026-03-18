#include "kernel.h"

// --- from main.cu ---
extern "C"
void zero_point (
    const float* x_min,
    const float* x_max,
    int32_t qmin,
    int32_t qmax,
    int size,
    bool preserve_sparsity,
    float* scale,
    int32_t* zero_point)
{
    #pragma HLS INTERFACE m_axi port=x_min offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=x_max offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=qmin
    #pragma HLS INTERFACE s_axilite port=qmax
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=preserve_sparsity
    #pragma HLS INTERFACE m_axi port=scale offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=zero_point offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i < size) {
            float min_val = x_min[i];
            float max_val = x_max[i];

            if (min_val < 0 && max_val > 0 && preserve_sparsity) {
            int symmetric_qmin = -((qmax - qmin) / 2 + 1);
            int symmetric_qmax = (qmax - qmin) / 2;
            double max_scale = fmax(
            fabs(min_val / symmetric_qmin), fabs(max_val / symmetric_qmax));
            min_val = max_scale * symmetric_qmin;
            max_val = max_scale * symmetric_qmax;
            }

            // We extend the [min, max] interval to ensure that it contains 0.
            // Otherwise, we would not meet the requirement that 0 be an exactly
            // representable value.
            min_val = fminf(min_val, 0.f);
            max_val = fmaxf(max_val, 0.f);
            scale[i] = (static_cast<double>(max_val) - min_val) / (qmax - qmin);

            // Moving this check outside this function would result in extra Device to
            // Host copy of the min and max val which would result in a perf hit.
            if (scale[i] == 0.0f || isinf(1.0f / scale[i])) {
            scale[i] = 0.1;
            }

            double zero_point_from_min = qmin - min_val / static_cast<double>(scale[i]);
            double zero_point_from_max = qmax - max_val / static_cast<double>(scale[i]);
            double zero_point_from_min_error = abs(qmin) + abs(min_val / static_cast<double>(scale[i]));
            double zero_point_from_max_error = abs(qmax) + abs(max_val / static_cast<double>(scale[i]));
            double initial_zero_point = zero_point_from_min_error < zero_point_from_max_error
            ? zero_point_from_min
            : zero_point_from_max;

            // Note: preserve_sparsity here means symmetric quantization.
            // for symmetric quantization, we force zero_point
            // to be a middle value between qmin and qmax.
            // If either min or max is 0, then we just use 0 as zero_point.
            if (min_val < 0 && max_val > 0 && preserve_sparsity) {
            initial_zero_point = static_cast<double>(qmin + qmax) / 2;
            }
            // Now we need to nudge the zero point to be an integer
            // (our zero points are integer, and this is motivated by the
            // requirement to be able to represent the real value "0" exactly as a
            // quantized value, which is required in multiple places, for example in
            // Im2col with zero padding).
            int32_t nudged_zero_point = 0;
            if (initial_zero_point < qmin) {
            nudged_zero_point = qmin;
            } else if (initial_zero_point > qmax) {
            nudged_zero_point = qmax;
            } else {
            nudged_zero_point = nearbyint(initial_zero_point);
            }
            zero_point[i] = nudged_zero_point;
            }

        }
    }
}
