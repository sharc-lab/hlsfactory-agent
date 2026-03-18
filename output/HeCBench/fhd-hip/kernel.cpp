#include "kernel.h"

// --- from main.cu ---
extern "C"
void cmpfhd(const float* rmu, 
            const float* imu,
                  float* rfhd,
                  float* ifhd,
            const float* x, 
            const float* y,
            const float* z,
            const int samples,
            const int voxels) 
{
    #pragma HLS INTERFACE m_axi port=rmu offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=imu offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=rfhd offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=ifhd offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=z offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=samples
    #pragma HLS INTERFACE s_axilite port=voxels
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int n = _bid_x * BLOCK_DIM_X + _tid_x;

            if (n < samples) {
            float xn = x[n], yn = y[n], zn = z[n];
            float rfhdn = rfhd[n], ifhdn = ifhd[n];
            for (int m = 0; m < voxels; m++) {
            float e = 2.f * (float)M_PI * (k[m].x * xn + k[m].y * yn + k[m].z * zn);
            float c = __cosf(e);
            float s = __sinf(e);
            rfhdn += rmu[m] * c - imu[m] * s;
            ifhdn += imu[m] * c + rmu[m] * s;
            }
            rfhd[n] = rfhdn, ifhd[n] = ifhdn;
            }

        }
    }
}
