#include "kernel.h"

// --- from haccmk.cu ---
extern "C"
void haccmk_kernel (
    const int n1,  // outer loop count
    const int n2,  // inner loop count
    const float * xx, 
    const float * yy,
    const float * zz,
    const float * mass,
          float * vx2,
          float * vy2,
          float * vz2,
    const float fsrmax,
    const float mp_rsm,
    const float fcoeff ) 
{
    #pragma HLS INTERFACE s_axilite port=n1
    #pragma HLS INTERFACE s_axilite port=n2
    #pragma HLS INTERFACE m_axi port=xx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=yy offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=zz offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=mass offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=vx2 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=vy2 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=vz2 offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=fsrmax
    #pragma HLS INTERFACE s_axilite port=mp_rsm
    #pragma HLS INTERFACE s_axilite port=fcoeff
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i >= n1) return;

            const float ma0 = 0.269327f;
            const float ma1 = -0.0750978f;
            const float ma2 = 0.0114808f;
            const float ma3 = -0.00109313f;
            const float ma4 = 0.0000605491f;
            const float ma5 = -0.00000147177f;

            float dxc, dyc, dzc, m, r2, f, xi, yi, zi;

            xi = 0.f;
            yi = 0.f;
            zi = 0.f;

            float xxi = xx[i];
            float yyi = yy[i];
            float zzi = zz[i];

            for ( int j = 0; j < n2; j++ ) {
            dxc = xx[j] - xxi;
            dyc = yy[j] - yyi;
            dzc = zz[j] - zzi;

            r2 = dxc * dxc + dyc * dyc + dzc * dzc;

            //if ( r2 < fsrmax ) m = mass[j]; else m = 0.f;
            m = mass[j] * (r2 < fsrmax);

            f = r2 + mp_rsm;
            f = m * (1.f / (f * sqrtf(f)) - (ma0 + r2*(ma1 + r2*(ma2 + r2*(ma3 + r2*(ma4 + r2*ma5))))));

            xi = xi + f * dxc;
            yi = yi + f * dyc;
            zi = zi + f * dzc;
            }

            vx2[i] += xi * fcoeff;
            vy2[i] += yi * fcoeff;
            vz2[i] += zi * fcoeff;

        }
    }
}
