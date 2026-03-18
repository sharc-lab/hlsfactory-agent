#include "kernel.h"

// --- from opt1.cu ---
extern "C"
void hotspotOpt1(float *p, float* tIn, float *tOut, float sdc,
        int nx, int ny, int nz,
        float ce, float cw, 
        float cn, float cs,
        float ct, float cb, 
        float cc) 
{
    #pragma HLS INTERFACE m_axi port=p offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=tIn offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=tOut offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=sdc
    #pragma HLS INTERFACE s_axilite port=nx
    #pragma HLS INTERFACE s_axilite port=ny
    #pragma HLS INTERFACE s_axilite port=nz
    #pragma HLS INTERFACE s_axilite port=ce
    #pragma HLS INTERFACE s_axilite port=cw
    #pragma HLS INTERFACE s_axilite port=cn
    #pragma HLS INTERFACE s_axilite port=cs
    #pragma HLS INTERFACE s_axilite port=ct
    #pragma HLS INTERFACE s_axilite port=cb
    #pragma HLS INTERFACE s_axilite port=cc
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float amb_temp = 80.0;

                    int i = BLOCK_DIM_X * _bid_x + _tid_x;
                    int j = BLOCK_DIM_Y * _bid_y + _tid_y;

                    int c = i + j * nx;
                    int xy = nx * ny;

                    int W = (i == 0)        ? c : c - 1;
                    int E = (i == nx-1)     ? c : c + 1;
                    int N = (j == 0)        ? c : c - nx;
                    int S = (j == ny-1)     ? c : c + nx;

                    float temp1, temp2, temp3;
                    temp1 = temp2 = tIn[c];
                    temp3 = tIn[c+xy];
                    tOut[c] = cc * temp2 + cw * tIn[W] + ce * tIn[E] + cs * tIn[S]
                    + cn * tIn[N] + cb * temp1 + ct * temp3 + sdc * p[c] + ct * amb_temp;
                    c += xy;
                    W += xy;
                    E += xy;
                    N += xy;
                    S += xy;

                    for (int k = 1; k < nz-1; ++k) {
                    temp1 = temp2;
                    temp2 = temp3;
                    temp3 = tIn[c+xy];
                    tOut[c] = cc * temp2 + cw * tIn[W] + ce * tIn[E] + cs * tIn[S]
                    + cn * tIn[N] + cb * temp1 + ct * temp3 + sdc * p[c] + ct * amb_temp;
                    c += xy;
                    W += xy;
                    E += xy;
                    N += xy;
                    S += xy;
                    }
                    temp1 = temp2;
                    temp2 = temp3;
                    tOut[c] = cc * temp2 + cw * tIn[W] + ce * tIn[E] + cs * tIn[S]
                    + cn * tIn[N] + cb * temp1 + ct * temp3 + sdc * p[c] + ct * amb_temp;
                    return;

                }
            }
        }
    }
}
