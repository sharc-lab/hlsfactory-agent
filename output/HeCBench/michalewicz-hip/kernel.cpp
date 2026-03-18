#include "kernel.h"

// --- from main.cu ---
float michalewicz(const float *xValues, const int dim) {
  float result = 0;
  for (int i = 0; i < dim; ++i) {
      float a = sinf(xValues[i]);
      float b = sinf(((i + 1) * xValues[i] * xValues[i]) / (float)M_PI);
      float c = powf(b, 20); // m = 10
      result += a * c;
  }
  return -1.0f * result;
}
extern "C"

void eval (const float *values, float *minima,
                      const size_t nVectors, const int dim)
{
    #pragma HLS INTERFACE m_axi port=values offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=minima offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=nVectors
    #pragma HLS INTERFACE s_axilite port=dim
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t n = _bid_x * BLOCK_DIM_X + _tid_x;
            if (n < nVectors) {
            (*minima = min(*minima, michalewicz(values + n * dim, dim)));
            }

        }
    }
}
