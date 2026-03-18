#include "kernel.h"

// --- from MT.cu ---
void BoxMullerTrans(float *u1, float *u2)
{
  const float   r = sqrtf(-2.0f * logf(*u1));
  const float phi = 2 * PI * (*u2);
  *u1 = r * cosf(phi);
  *u2 = r * sinf(phi);
}
extern "C"

void boxmuller (float* Rand, const int nPerRng) 
{
    #pragma HLS INTERFACE m_axi port=Rand offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=nPerRng
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int globalID = BLOCK_DIM_X * _bid_x + _tid_x;
            for (int iOut = 0; iOut < nPerRng; iOut += 2) {
            BoxMullerTrans(&Rand[globalID + (iOut + 0) * MT_RNG_COUNT],
            &Rand[globalID + (iOut + 1) * MT_RNG_COUNT]);
            }

        }
    }
}
extern "C"

void mt (const mt_struct_stripped* MT, float* Rand, const int nPerRng) 
{
    #pragma HLS INTERFACE m_axi port=MT offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=Rand offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=nPerRng
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int globalID = BLOCK_DIM_X * _bid_x + _tid_x;

            int iState, iState1, iStateM, iOut;
            unsigned int mti, mti1, mtiM, x;
            unsigned int mt[MT_NN], matrix_a, mask_b, mask_c;

            //Load bit-vector Mersenne Twister parameters
            matrix_a = MT[globalID].matrix_a;
            mask_b   = MT[globalID].mask_b;
            mask_c   = MT[globalID].mask_c;

            //Initialize current state
            mt[0] = MT[globalID].seed;
            for (iState = 1; iState < MT_NN; iState++)
            mt[iState] = (1812433253U * (mt[iState - 1] ^ (mt[iState - 1] >> 30)) + iState) & MT_WMASK;

            iState = 0;
            mti1 = mt[0];
            for (iOut = 0; iOut < nPerRng; iOut++) {
            iState1 = iState + 1;
            iStateM = iState + MT_MM;
            if(iState1 >= MT_NN) iState1 -= MT_NN;
            if(iStateM >= MT_NN) iStateM -= MT_NN;
            mti  = mti1;
            mti1 = mt[iState1];
            mtiM = mt[iStateM];

            // MT recurrence
            x = (mti & MT_UMASK) | (mti1 & MT_LMASK);
            x = mtiM ^ (x >> 1) ^ ((x & 1) ? matrix_a : 0);

            mt[iState] = x;
            iState = iState1;

            //Tempering transformation
            x ^= (x >> MT_SHIFT0);
            x ^= (x << MT_SHIFTB) & mask_b;
            x ^= (x << MT_SHIFTC) & mask_c;
            x ^= (x >> MT_SHIFT1);

            //Convert to (0, 1] float and write to global memory
            Rand[globalID + iOut * MT_RNG_COUNT] = ((float)x + 1.0f) / 4294967296.0f;
            }

        }
    }
}
