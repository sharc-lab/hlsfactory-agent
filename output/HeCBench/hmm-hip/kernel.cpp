#include "kernel.h"

// --- from ViterbiGPU.cu ---
extern "C"
void viterbi (const float* maxProbOld, 
         const float* mtState, 
         const float* mtEmit, 
                 int* obs, 
               float* maxProbNew, 
                 int* path, 
         const int nState,
         const int t)
{
    #pragma HLS INTERFACE m_axi port=maxProbOld offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=mtState offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=mtEmit offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=obs offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=maxProbNew offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=path offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=nState
    #pragma HLS INTERFACE s_axilite port=t
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            // find the most probable previous state leading to iState
            int iState = BLOCK_DIM_X * _bid_x + _tid_x;
            if (iState < nState) {
            float maxProb = 0.0;
            int maxState = -1;
            for (int preState = 0; preState < nState; preState++)
            {
            float p = maxProbOld[preState] + mtState[iState*nState + preState];
            if (p > maxProb)
            {
            maxProb = p;
            maxState = preState;
            }
            }
            maxProbNew[iState] = maxProb + mtEmit[obs[t]*nState+iState];
            path[(t-1)*nState+iState] = maxState;
            }

        }
    }
}
