#include "kernel.h"

// --- from 3D.cu ---
extern "C"
void hotspot3d(
    const float* tIn, 
    const float* pIn, 
          float* tOut,
    const int numCols, 
    const int numRows, 
    const int layers,
    const float ce, 
    const float cw,
    const float cn, 
    const float cs,
    const float ct,
    const float cb,
    const float cc,
    const float stepDivCap)
{
    #pragma HLS INTERFACE m_axi port=tIn offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=pIn offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=tOut offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=numCols
    #pragma HLS INTERFACE s_axilite port=numRows
    #pragma HLS INTERFACE s_axilite port=layers
    #pragma HLS INTERFACE s_axilite port=ce
    #pragma HLS INTERFACE s_axilite port=cw
    #pragma HLS INTERFACE s_axilite port=cn
    #pragma HLS INTERFACE s_axilite port=cs
    #pragma HLS INTERFACE s_axilite port=ct
    #pragma HLS INTERFACE s_axilite port=cb
    #pragma HLS INTERFACE s_axilite port=cc
    #pragma HLS INTERFACE s_axilite port=stepDivCap
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    float amb_temp = 80.0;

                    int i = BLOCK_DIM_X * _bid_x + _tid_x;
                    int j = BLOCK_DIM_Y * _bid_y + _tid_y;
                    int c = i + j * numCols;
                    int xy = numCols * numRows;

                    int W = (i == 0)        ? c : c - 1;
                    int E = (i == numCols-1)     ? c : c + 1;
                    int N = (j == 0)        ? c : c - numCols;
                    int S = (j == numRows-1)     ? c : c + numCols;

                    float temp1, temp2, temp3;
                    temp1 = temp2 = tIn[c];
                    temp3 = tIn[c+xy];
                    tOut[c] = cc * temp2 + cw * tIn[W] + ce * tIn[E] + cs * tIn[S]
                    + cn * tIn[N] + cb * temp1 + ct * temp3 + stepDivCap * pIn[c] + ct * amb_temp;
                    c += xy;
                    W += xy;
                    E += xy;
                    N += xy;
                    S += xy;

                    for (int k = 1; k < layers-1; ++k) {
                    temp1 = temp2;
                    temp2 = temp3;
                    temp3 = tIn[c+xy];
                    tOut[c] = cc * temp2 + cw * tIn[W] + ce * tIn[E] + cs * tIn[S]
                    + cn * tIn[N] + cb * temp1 + ct * temp3 + stepDivCap * pIn[c] + ct * amb_temp;
                    c += xy;
                    W += xy;
                    E += xy;
                    N += xy;
                    S += xy;
                    }
                    temp1 = temp2;
                    temp2 = temp3;
                    tOut[c] = cc * temp2 + cw * tIn[W] + ce * tIn[E] + cs * tIn[S]
                    + cn * tIn[N] + cb * temp1 + ct * temp3 + stepDivCap * pIn[c] + ct * amb_temp;

                }
            }
        }
    }
}
