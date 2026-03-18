#include "kernel.h"

// --- from kernel.cu ---
extern "C"
void dwtHaar1D( const float * inSignal,
                      float * coefsSignal,
                      float * AverageSignal,
                      const unsigned int tLevels,
                      const unsigned int signalLength,
                      const unsigned int levelsDone,
		      const unsigned int mLevels)
              
{
    #pragma HLS INTERFACE m_axi port=inSignal offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=coefsSignal offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=AverageSignal offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=tLevels
    #pragma HLS INTERFACE s_axilite port=signalLength
    #pragma HLS INTERFACE s_axilite port=levelsDone
    #pragma HLS INTERFACE s_axilite port=mLevels
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sharedArray complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t localId = _tid_x;
            size_t groupId = _bid_x;
            size_t localSize = BLOCK_DIM_X;

            float sharedArray[4096];

            /**
            * Read input signal data from global memory
            * to shared memory
            */
            float t0 = inSignal[groupId * localSize * 2 + localId];
            float t1 = inSignal[groupId * localSize * 2 + localSize + localId];
            // Divide with signal length for normalized decomposition
            if(0 == levelsDone)
            {
            float r = rsqrtf((float)signalLength);
            t0 *= r;
            t1 *= r;
            }
            sharedArray[localId] = t0;
            sharedArray[localSize + localId] = t1;

            unsigned int levels = tLevels > mLevels ? mLevels: tLevels;
            unsigned int activeThreads = (1 << levels) / 2;
            unsigned int midOutPos = signalLength / 2;

            const float rsqrt_two = 0.7071f;
            for(unsigned int i = 0; i < levels; ++i)
            {

            float data0, data1;
            if(localId < activeThreads)
            {
            data0 = sharedArray[2 * localId];
            data1 = sharedArray[2 * localId + 1];
            }

            /* make sure all work items have read from sharedArray before modifying it */

            if(localId < activeThreads)
            {
            sharedArray[localId] = (data0 + data1) * rsqrt_two;
            unsigned int globalPos = midOutPos + groupId * activeThreads + localId;
            coefsSignal[globalPos] = (data0 - data1) * rsqrt_two;

            midOutPos >>= 1;
            }
            activeThreads >>= 1;
            }

            /**
            * Write 0th element for the next decomposition
            * steps which are performed on host
            */

            if(0 == localId)
            AverageSignal[groupId] = sharedArray[0];

        }
    }
}
