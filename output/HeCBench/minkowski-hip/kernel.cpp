#include "kernel.h"

// --- from main.cu ---
extern "C"
void minkowski(
  const float * a,
  const float * b,
        float * c,
  const float p,
  const float one_over_p)
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=p
    #pragma HLS INTERFACE s_axilite port=one_over_p
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int col = _bid_x * BLOCK_DIM_X + _tid_x;
                    int row = _bid_y * BLOCK_DIM_Y + _tid_y;
                    if( col < k && row < m)
                    {
                    float sum = 0;
                    for(int i = 0; i < n; i++)
                    {
                    sum += powf(fabsf(a[row * n + i] - b[i * k + col]), p);
                    }
                    c[row * k + col] = powf(sum, one_over_p);
                    }

                }
            }
        }
    }
}
