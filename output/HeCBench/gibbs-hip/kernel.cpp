#include "kernel.h"

// --- from main.cu ---
float rgamma(float a, float b)
{
  float x;
  if (a > 1) {
    float d = a - 1.f / 3.f;
    float c = 1.f / sqrtf(9.f * d);
    bool flag = true;
    float V;
    while (flag) {
      // Generate a standard normal random variable
      float Z = rnorm();
      if (Z > -1.f / c) {
        V = powf(1.f + c * Z, 3.f);
        float U = rand() / (float)RAND_MAX;
        flag = logf(U) > (0.5f * Z * Z + d - d * V + d * logf(V));
      }
    }
    x = d * V / b;
  }
  else 
  {
    x = rgamma(a + 1.f, b);
    x = x * powf(rand() / (float)RAND_MAX, 1.f / a);
  }
  return x;
}
extern "C"

void setup_kernel(hiprandState *state, int num_sample, unsigned int seed)
{
    #pragma HLS INTERFACE m_axi port=state offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=num_sample
    #pragma HLS INTERFACE s_axilite port=seed
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int id = _tid_x + _bid_x * BLOCK_DIM_X;
            if (id < num_sample)
            /* Each thread gets same seed, a different sequence number, no offset */
            hiprand_init(seed, id, 0, &state[id]);

        }
    }
}
extern "C"

void sample_theta(
    hiprandState * state, 
    float * theta,
    const int * y,
    const float * n, 
    float a, float b, int num_sample)
{
    #pragma HLS INTERFACE m_axi port=state offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=theta offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=n offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=a
    #pragma HLS INTERFACE s_axilite port=b
    #pragma HLS INTERFACE s_axilite port=num_sample
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int id = _tid_x + _bid_x * BLOCK_DIM_X;
            if(id < num_sample) {
            hiprandState *s = state + id;
            const float hyperA = a + y[id];
            const float hyperB = b + n[id];
            theta[id] = (hyperA < 1.f) ?
            rgamma(s, hyperA + 1.f, hyperB) * powf(hiprand_uniform(s), 1.f / hyperA) :
            rgamma(s, hyperA, hyperB);
            }

        }
    }
}
