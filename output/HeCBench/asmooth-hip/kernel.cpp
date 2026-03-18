#include "kernel.h"

// --- from main.cu ---
extern "C"
void smoothingFilter(
    int Lx, int Ly, 
    int Threshold, int MaxRad, 
    const float* Img,
            int* Box,
          float* Norm)
{
    #pragma HLS INTERFACE s_axilite port=Lx
    #pragma HLS INTERFACE s_axilite port=Ly
    #pragma HLS INTERFACE s_axilite port=Threshold
    #pragma HLS INTERFACE s_axilite port=MaxRad
    #pragma HLS INTERFACE m_axi port=Img offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Box offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=Norm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_Img complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_Img complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int tid = _tid_x;
                    int tjd = _tid_y;
                    int i = _bid_x * BLOCK_DIM_X + tid;
                    int j = _bid_y * BLOCK_DIM_Y + tjd;
                    int stid = tjd * BLOCK_DIM_X + tid;
                    int gtid = j * Lx + i;

                    // part of shared memory may be unused
                    float s_Img[1024];

                    if ( i < Lx && j < Ly )
                    s_Img[stid] = Img[gtid];

                    if ( i < Lx && j < Ly )
                    {
                    // Smoothing parameters
                    float sum = 0.f;
                    int q = 1;
                    int s = q;
                    int ksum = 0;

                    // Continue until parameters are met
                    while (sum < Threshold && q < MaxRad)
                    {
                    s = q;
                    sum = 0.f;
                    ksum = 0;

                    // Normal adaptive smoothing
                    for (int ii = -s; ii < s+1; ii++)
                    for (int jj = -s; jj < s+1; jj++)
                    if ( (i-s >= 0) && (i+s < Ly) && (j-s >= 0) && (j+s < Lx) )
                    {
                    ksum++;
                    // Compute within bounds of block dimensions
                    if( tid-s >= 0 && tid+s < BLOCK_DIM_X && tjd-s >= 0 && tjd+s < BLOCK_DIM_Y )
                    sum += s_Img[stid + ii*BLOCK_DIM_X + jj];
                    // Compute block borders with global memory
                    else
                    sum += Img[gtid + ii*Lx + jj];
                    }
                    q++;
                    }
                    Box[gtid] = s;

                    // Normalization for each box
                    for (int ii = -s; ii < s+1; ii++)
                    for (int jj = -s; jj < s+1; jj++)
                    if (ksum != 0)
                    (Norm[gtid + ii*Lx + jj] += __fdividef(1.f, (float)ksum));
                    }

                }
            }
        }
    }
}
extern "C"

void normalizeFilter(
    int Lx, int Ly, 
          float* Img,
    const float* Norm)
{
    #pragma HLS INTERFACE s_axilite port=Lx
    #pragma HLS INTERFACE s_axilite port=Ly
    #pragma HLS INTERFACE m_axi port=Img offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Norm offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int i = _bid_x * BLOCK_DIM_X + _tid_x;
                    int j = _bid_y * BLOCK_DIM_Y + _tid_y;
                    if ( i < Lx && j < Ly ) {
                    int gtid = j * Lx + i;
                    const float norm = Norm[gtid];
                    if (norm != 0) Img[gtid] = __fdividef(Img[gtid], norm);
                    }

                }
            }
        }
    }
}
extern "C"

void outFilter( 
    int Lx, int Ly,
    const float* Img,
    const   int* Box,
          float* Out )
{
    #pragma HLS INTERFACE s_axilite port=Lx
    #pragma HLS INTERFACE s_axilite port=Ly
    #pragma HLS INTERFACE m_axi port=Img offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Box offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=Out offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_Img complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_Img complete dim=1

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    int tid = _tid_x;
                    int tjd = _tid_y;
                    int i = _bid_x * BLOCK_DIM_X + tid;
                    int j = _bid_y * BLOCK_DIM_Y + tjd;
                    int stid = tjd * BLOCK_DIM_X + tid;
                    int gtid = j * Lx + i;

                    // part of shared memory may be unused
                    float s_Img[1024];

                    if ( i < Lx && j < Ly )
                    s_Img[stid] = Img[gtid];

                    if ( i < Lx && j < Ly )
                    {
                    const int s = Box[gtid];
                    float sum = 0.f;
                    int ksum  = 0;

                    for (int ii = -s; ii < s+1; ii++)
                    for (int jj = -s; jj < s+1; jj++)
                    if ( (i-s >= 0) && (i+s < Lx) && (j-s >= 0) && (j+s < Ly) )
                    {
                    ksum++;
                    if( tid-s >= 0 && tid+s < BLOCK_DIM_X && tjd-s >= 0 && tjd+s < BLOCK_DIM_Y )
                    sum += s_Img[stid + ii*BLOCK_DIM_Y + jj];
                    else
                    sum += Img[gtid + ii*Ly + jj];
                    }
                    if ( ksum != 0 ) Out[gtid] = __fdividef(sum , (float)ksum);
                    }

                }
            }
        }
    }
}
