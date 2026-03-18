#include "kernel.h"

// --- from computeQ.cu ---
extern "C"
void ComputePhiMag_GPU(
  const float*  phiR,
  const float*  phiI,
        float*  phiMag,
  const int numK)
{
    #pragma HLS INTERFACE m_axi port=phiR offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=phiI offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=phiMag offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=numK
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int indexK = _bid_x*KERNEL_PHI_MAG_THREADS_PER_BLOCK + _tid_x;
            if (indexK < numK) {
            float real = phiR[indexK];
            float imag = phiI[indexK];
            phiMag[indexK] = real*real + imag*imag;
            }

        }
    }
}
extern "C"

void ComputeQ_GPU(
  const int numK,
        int kGlobalIndex,
  const float*  x,
  const float*  y,
  const float*  z,
        float*  Qr,
        float*  Qi)
{
    #pragma HLS INTERFACE s_axilite port=numK
    #pragma HLS INTERFACE s_axilite port=kGlobalIndex
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=z offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=Qr offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=Qi offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // Determine the element of the X arrays computed by this thread
            int xIndex = _bid_x*KERNEL_Q_THREADS_PER_BLOCK + _tid_x;

            // Read block's X values from global mem to shared mem
            float sX = x[xIndex];
            float sY = y[xIndex];
            float sZ = z[xIndex];
            float sQr = Qr[xIndex];
            float sQi = Qi[xIndex];

            // Loop over all elements of K in constant mem to compute a partial value
            // for X.
            int kIndex = 0;
            if (numK % 2) {
            float expArg = PIx2 * (ck[0].Kx * sX + ck[0].Ky * sY + ck[0].Kz * sZ);
            sQr += ck[0].PhiMag * cosf(expArg);
            sQi += ck[0].PhiMag * sinf(expArg);
            kIndex++;
            kGlobalIndex++;
            }

            for (; (kIndex < KERNEL_Q_K_ELEMS_PER_GRID) && (kGlobalIndex < numK);
            kIndex += 2, kGlobalIndex += 2) {
            float expArg = PIx2 * (ck[kIndex].Kx * sX +
            ck[kIndex].Ky * sY +
            ck[kIndex].Kz * sZ);
            sQr += ck[kIndex].PhiMag * cosf(expArg);
            sQi += ck[kIndex].PhiMag * sinf(expArg);

            int kIndex1 = kIndex + 1;
            float expArg1 = PIx2 * (ck[kIndex1].Kx * sX +
            ck[kIndex1].Ky * sY +
            ck[kIndex1].Kz * sZ);
            sQr += ck[kIndex1].PhiMag * cosf(expArg1);
            sQi += ck[kIndex1].PhiMag * sinf(expArg1);
            }

            Qr[xIndex] = sQr;
            Qi[xIndex] = sQi;

        }
    }
}
