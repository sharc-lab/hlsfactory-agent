#include "kernel.h"

// --- from main.cu ---
inline float createRoundingFactor(float max, int n) {
  float delta = (max * (float)n) / (1.f - 2.f * (float)n * FLT_EPSILON);

  // Calculate ceil(log_2(delta)).
  // frexpf() calculates exp and returns `x` such that
  // delta = x * 2^exp, where `x` in (-1.0, -0.5] U [0.5, 1).
  // Because |x| < 1, exp is exactly ceil(log_2(delta)).
  int exp;
  frexpf(delta, &exp);

  // return M = 2 ^ ceil(log_2(delta))
  return ldexpf(1.f, exp);
}

inline float truncateWithRoundingFactor(float roundingFactor, float x) {
  return (roundingFactor + x) -  // rounded
         roundingFactor;         // exactly
}
extern "C"

void sumArray (
  const float factor, 
  const   int length,
  const float * x,
        float * r)
{
    #pragma HLS INTERFACE s_axilite port=factor
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            for (int i = BLOCK_DIM_X * _bid_x + _tid_x; i < length;
            i += BLOCK_DIM_X * GRID_DIM_X) {
            float q = truncateWithRoundingFactor(factor, x[i]);
            (*r += q);  // sum in any order
            }

        }
    }
}
extern "C"

void sumArrays (
  const int nArrays,
  const int length,
  const float * x,
        float * r,
  const float * maxVal)
{
    #pragma HLS INTERFACE s_axilite port=nArrays
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=maxVal offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            for (int i = BLOCK_DIM_X * _bid_x + _tid_x; i < nArrays;
            i += BLOCK_DIM_X * GRID_DIM_X) {
            x += i * length;
            float factor = createRoundingFactor(maxVal[i], length);
            float s = 0;
            for (int n = length-1; n >= 0; n--)  // sum in reverse order
            s += truncateWithRoundingFactor(factor, x[n]);
            r[i] = s;
            }

        }
    }
}
