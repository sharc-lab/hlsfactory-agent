#include "kernel.h"

// --- from main.cu ---
extern "C"
void matern_kernel (
  const int num_targets,
  const float l,
  const float * sources,
  const float * targets,
  const float * weights,
        float * result)
{
    #pragma HLS INTERFACE s_axilite port=num_targets
    #pragma HLS INTERFACE s_axilite port=l
    #pragma HLS INTERFACE m_axi port=sources offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=targets offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=weights offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=local_result complete dim=1
    #pragma HLS ARRAY_PARTITION variable=local_targets complete dim=1
    #pragma HLS ARRAY_PARTITION variable=local_sources complete dim=1
    #pragma HLS ARRAY_PARTITION variable=local_weights complete dim=1

    for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
        for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
            for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
            #pragma HLS PIPELINE II=1

                int tx = _tid_x;
                int px = _bid_x * BLOCK_DIM_X + tx; // range [0:ntargets)
                if (px >= num_targets) return;

                int ty = _tid_y;
                int py = ty; // range [0:nsources)
                if (py >= SY) return;

                float local_result[SX * SY];
                float local_targets[SX * 3];
                float local_sources[SY * 3];
                float local_weights[SY];

                if (ty == 0) {
                for (int i = 0; i < 3; i++)
                local_targets[tx * 3 + i] = targets[px * 3 + i];
                }

                if (tx == 0) {
                for (int i = 0; i < 3; i++)
                local_sources[ty * 3 + i] = sources[py * 3 + i];
                local_weights[ty] = weights[ty];
                }

                float squared_diff = 0.f;

                for (int i = 0; i < 3; i++) {
                squared_diff += (local_targets[tx * 3 + i] - local_sources[ty * 3 + i]) *
                (local_targets[tx * 3 + i] - local_sources[ty * 3 + i]);
                }
                float diff = sqrtf(squared_diff);

                local_result[tx * SY + ty] =
                (1.f + sqrtf(5.f) * diff / l + 5.f * squared_diff / (3.f * l * l)) *
                expf(-sqrtf(5.f) * diff  / l) * local_weights[ty];

                if (ty == 0) {
                float res = 0.f;
                for (int i = 0; i < SY; i++)
                res += local_result[tx * SY + i];
                result[px] = res;
                }

            }
        }
    }
}
