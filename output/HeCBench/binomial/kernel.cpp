#include "kernel.h"

// --- from kernel.cu ---
inline float expiryCallValue(float S, float X, float vDt, int i)
{
  float d = S * __expf(vDt * (2.0f * i - NUM_STEPS)) - X;
  return (d > 0.0F) ? d : 0.0F;
}

inline double expiryCallValue(double S, double X, double vDt, int i)
{
  double d = S * exp(vDt * (2.0 * i - NUM_STEPS)) - X;
  return (d > 0.0) ? d : 0.0;
}
extern "C"

void binomialOptionsKernel(const __TOptionData *__restrict d_OptionData,
                                      real *__restrict d_CallValue)
{
    #pragma HLS INTERFACE m_axi port=d_OptionData offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_CallValue offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=call_exchange complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            real call_exchange[THREADBLOCK_SIZE + 1];

            const int     tid = _tid_x;
            const real      S = d_OptionData[_bid_x].S;
            const real      X = d_OptionData[_bid_x].X;
            const real    vDt = d_OptionData[_bid_x].vDt;
            const real puByDf = d_OptionData[_bid_x].puByDf;
            const real pdByDf = d_OptionData[_bid_x].pdByDf;

            real call[ELEMS_PER_THREAD + 1];
            #pragma unroll
            for(int i = 0; i < ELEMS_PER_THREAD; ++i)
            call[i] = expiryCallValue(S, X, vDt, tid * ELEMS_PER_THREAD + i);

            if (tid == 0)
            call_exchange[THREADBLOCK_SIZE] = expiryCallValue(S, X, vDt, NUM_STEPS);

            int final_it = max(0, tid * ELEMS_PER_THREAD - 1);

            #pragma unroll 16
            for(int i = NUM_STEPS; i > 0; --i)
            {
            call_exchange[tid] = call[0];
            call[ELEMS_PER_THREAD] = call_exchange[tid + 1];

            if (i > final_it)
            {
            #pragma unroll
            for(int j = 0; j < ELEMS_PER_THREAD; ++j)
            call[j] = puByDf * call[j + 1] + pdByDf * call[j];
            }
            }

            if (tid == 0)
            {
            d_CallValue[_bid_x] = call[0];
            }

        }
    }
}


// --- from reference.cu ---
static real expiryCallValue(real S, real X, real vDt, int i)
{
  real d = S * exp(vDt * (real)(2 * i - NUM_STEPS)) - X;
  return (d > (real)0) ? d : (real)0;
}
