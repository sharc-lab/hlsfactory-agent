#include "kernel.h"

// --- from main.cu ---
inline float hd (const float2 ap, const float2 bp)
{
  return (ap.x - bp.x) * (ap.x - bp.x)
       + (ap.y - bp.y) * (ap.y - bp.y);
}

void atomic_max(float *address, float val)
{
  unsigned int ret = __float_as_uint(*address);
  while(val > __uint_as_float(ret))
  {
    unsigned int old = ret;
    if((ret = atomicCAS((unsigned int *)address, old, __float_as_uint(val))) == old)
      break;
  }
}
extern "C"

void computeDistance(const float2*  Apoints,
                     const float2*  Bpoints,
                           float*   distance,
                     const int numA, const int numB)
{
    #pragma HLS INTERFACE m_axi port=Apoints offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Bpoints offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=distance offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=numA
    #pragma HLS INTERFACE s_axilite port=numB
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i >= numA) return;

            float d = std::numeric_limits<float>::max();
            float2 p = Apoints[i];
            for (int j = 0; j < numB; j++)
            {
            float t = hd(p, Bpoints[j]);
            d = std::min(t, d);
            }

            atomic_max(distance, d);

        }
    }
}
