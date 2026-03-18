#include "kernel.h"

// --- from main.cu ---
extern "C"
void adam (
        T*  p,
        T*  m,
        T*  v,
  const G*  g,
  const float b1,
  const float b2,
  const float eps,
  const float grad_scale,
  const float step_size,
  const int time_step,
  const size_t vector_size,
  adamMode_t mode,
  const float decay)
{
    #pragma HLS INTERFACE m_axi port=p offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=m offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=b1
    #pragma HLS INTERFACE s_axilite port=b2
    #pragma HLS INTERFACE s_axilite port=eps
    #pragma HLS INTERFACE s_axilite port=grad_scale
    #pragma HLS INTERFACE s_axilite port=step_size
    #pragma HLS INTERFACE s_axilite port=time_step
    #pragma HLS INTERFACE s_axilite port=vector_size
    #pragma HLS INTERFACE s_axilite port=mode
    #pragma HLS INTERFACE s_axilite port=decay
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const size_t i = _bid_x * BLOCK_DIM_X + _tid_x;
            const size_t totThreads = GRID_DIM_X*BLOCK_DIM_X;

            for (size_t j = i; j < vector_size; j += totThreads) {
            for (int t = 1; t <= time_step; t++) {
            T scaled_grad = g[j]/grad_scale;
            m[j] = b1*m[j] + (1.f-b1)*scaled_grad;
            v[j] = b2*v[j] + (1.f-b2)*scaled_grad*scaled_grad;
            float m_corrected = m[j] / (1.f-powf(b1, t));
            float v_corrected = v[j] / (1.f-powf(b2, t));
            float denom;
            if (mode == ADAM_MODE_0)
            denom = sqrtf(v_corrected + eps);
            else // Mode 1
            denom = sqrtf(v_corrected) + eps;
            float update = (m_corrected/denom) + (decay*p[j]);
            p[j] -= (step_size*update);
            }
            }

        }
    }
}
