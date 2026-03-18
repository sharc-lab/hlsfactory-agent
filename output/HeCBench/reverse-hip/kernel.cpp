#include "kernel.h"

// --- from main.cu ---
extern "C"
void reverse (int *d, const int len)
{
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=len
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        int s[256];
        int t = _tid_x;
        s[t] = d[t];
        d[t] = s[len-t-1];

    }
}
