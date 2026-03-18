#include "kernel.h"

// --- from main.cu ---
extern "C"
void bilateralFilter(
    const float * in,
    float * out,
    int w, 
    int h, 
    float a_square,
    float variance_I,
    float variance_spatial)
{
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=w
    #pragma HLS INTERFACE s_axilite port=h
    #pragma HLS INTERFACE s_axilite port=a_square
    #pragma HLS INTERFACE s_axilite port=variance_I
    #pragma HLS INTERFACE s_axilite port=variance_spatial
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    const int idx = _bid_x*BLOCK_DIM_X + _tid_x;
                    const int idy = _bid_y*BLOCK_DIM_Y + _tid_y;

                    if(idx >= w || idy >= h) return;

                    int id = idy*w + idx;
                    float I = in[id];
                    float res = 0;
                    float normalization = 0;

                    // window centered at the coordinate (idx, idy)
                    #ifdef LOOP_UNROLL
                    #pragma unroll
                    #endif
                    for(int i = -R; i <= R; i++) {
                    #ifdef LOOP_UNROLL
                    #pragma unroll
                    #endif
                    for(int j = -R; j <= R; j++) {

                    int idk = idx+i;
                    int idl = idy+j;

                    // mirror edges
                    if( idk < 0) idk = -idk;
                    if( idl < 0) idl = -idl;
                    if( idk > w - 1) idk = w - 1 - i;
                    if( idl > h - 1) idl = h - 1 - j;

                    int id_w = idl*w + idk;
                    float I_w = in[id_w];

                    // range kernel for smoothing differences in intensities
                    float range = -(I-I_w) * (I-I_w) / (2.f * variance_I);

                    // spatial (or domain) kernel for smoothing differences in coordinates
                    float spatial = -((idk-idx)*(idk-idx) + (idl-idy)*(idl-idy)) /
                    (2.f * variance_spatial);

                    // the weight is assigned using the spatial closeness (using the spatial kernel)
                    // and the intensity difference (using the range kernel)
                    float weight = a_square * expf(spatial + range);

                    normalization += weight;
                    res += (I_w * weight);
                    }
                    }
                    out[id] = res/normalization;

                }
            }
        }
    }
}
