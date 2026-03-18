#include "kernel.h"

// --- from main.cu ---
extern "C"
void dev_lapl_iter(float *out, const float *in, const float delta, const float norm, const int Lx, const int Ly)
{
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=delta
    #pragma HLS INTERFACE s_axilite port=norm
    #pragma HLS INTERFACE s_axilite port=Lx
    #pragma HLS INTERFACE s_axilite port=Ly
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            int x = i % Lx;
            int y = i / Lx;
            int v00 = y*Lx + x;
            int v0p = y*Lx + (x + 1)%Lx;
            int v0m = y*Lx + (Lx + x - 1)%Lx;
            int vp0 = ((y+1)%Ly)*Lx + x;
            int vm0 = ((Ly+y-1)%Ly)*Lx + x;
            out[v00] = norm*in[v00] + delta*(in[v0p] + in[v0m] + in[vp0] + in[vm0]);

        }
    }
}
