#include "kernel.h"

// --- from main.cu ---
inline FLOAT BezierBlend(int k, FLOAT mu, int n) {
  int nn, kn, nkn;
  FLOAT   blend = 1;
  nn        = n;
  kn        = k;
  nkn       = n - k;
  while(nn >= 1) {
    blend *= nn;
    nn--;
    if(kn > 1) {
      blend /= (FLOAT)kn;
      kn--;
    }
    if(nkn > 1) {
      blend /= (FLOAT)nkn;
      nkn--;
    }
  }
  if(k > 0)
    blend *= pow(mu, (FLOAT)k);
  if(n - k > 0)
    blend *= pow(1 - mu, (FLOAT)(n - k));
  return (blend);
}
extern "C"

void BezierGPU(const XYZ *inp, XYZ *outp, const int NI, const int NJ, const int RESOLUTIONI, const int RESOLUTIONJ) {
    #pragma HLS INTERFACE m_axi port=inp offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=outp offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=NI
    #pragma HLS INTERFACE s_axilite port=NJ
    #pragma HLS INTERFACE s_axilite port=RESOLUTIONI
    #pragma HLS INTERFACE s_axilite port=RESOLUTIONJ
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i, j, ki, kj;
            FLOAT   mui, muj, bi, bj;

            i = BLOCK_DIM_X * _bid_x + _tid_x;
            if (i > RESOLUTIONI) return;

            mui = i / (FLOAT)(RESOLUTIONI - 1);
            for(j = 0; j < RESOLUTIONJ; j++) {
            muj     = j / (FLOAT)(RESOLUTIONJ - 1);
            XYZ out = {0, 0, 0};
            //#pragma unroll
            for(ki = 0; ki <= NI; ki++) {
            bi = BezierBlend(ki, mui, NI);
            //#pragma unroll
            for(kj = 0; kj <= NJ; kj++) {
            bj = BezierBlend(kj, muj, NJ);
            out.x += (inp[ki * (NJ + 1) + kj].x * bi * bj);
            out.y += (inp[ki * (NJ + 1) + kj].y * bi * bj);
            out.z += (inp[ki * (NJ + 1) + kj].z * bi * bj);
            }
            }
            outp[i * RESOLUTIONJ + j] = out;
            }


        }
    }
}
