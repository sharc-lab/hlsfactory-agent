#include "kernel.h"

// --- from main.cu ---
extern "C"
void tissue(
    const   int * d_tisspoints,
    const float * d_gtt,
    const float * d_gbartt,
          float * d_ct,
    const float * d_ctprev,
    const float * d_qt,
    int nnt, int nntDev, int step, int isp)
{
    #pragma HLS INTERFACE m_axi port=d_tisspoints offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_gtt offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_gbartt offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_ct offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_ctprev offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=d_qt offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=nnt
    #pragma HLS INTERFACE s_axilite port=nntDev
    #pragma HLS INTERFACE s_axilite port=step
    #pragma HLS INTERFACE s_axilite port=isp
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = BLOCK_DIM_X * _bid_x + _tid_x;
            if (i >= step * nnt) return;

            int jtp,ixyz,ix,iy,iz,jx,jy,jz,istep;
            int nnt2 = 2*nnt;
            float p = 0.f;

            const int itp = i/step;
            const int itp1 = i%step;
            if(itp < nnt) {
            ix = d_tisspoints[itp];
            iy = d_tisspoints[itp+nnt];
            iz = d_tisspoints[itp+nnt2];
            for(jtp = itp1; jtp < nnt; jtp += step) {
            jx = d_tisspoints[jtp];
            jy = d_tisspoints[jtp+nnt];
            jz = d_tisspoints[jtp+nnt2];
            ixyz = abs(jx-ix) + abs(jy-iy) + abs(jz-iz) + (isp-1)*nntDev;
            p += d_gtt[ixyz]*d_ctprev[jtp] + d_gbartt[ixyz]*d_qt[jtp];
            }
            if(itp1 == 0) d_ct[itp] = p;
            }
            // d_ct is incremented in sequence from the needed threads
            for(istep=1; istep<step; istep++)
            if(itp1 == istep && itp < nnt) d_ct[itp] += p;

        }
    }
}
