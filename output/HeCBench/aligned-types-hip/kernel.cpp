#include "kernel.h"

// --- from main.cu ---
template<class TData> void testKernel(
          TData *__restrict d_odata,
    const TData *__restrict d_idata,
    int numElements
    )
{
    #pragma HLS INTERFACE m_axi port=d_odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=numElements
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int pos = BLOCK_DIM_X * _bid_x + _tid_x;
            if (pos < numElements)
            {
            d_odata[pos] = d_idata[pos];
            }

        }
    }
}
