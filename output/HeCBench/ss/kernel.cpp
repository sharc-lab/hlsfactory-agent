#include "kernel.h"

// --- from kernels.cu ---
int compare(const uchar* text, const uchar* pattern, uint length)
{
  for(uint l=0; l<length; ++l)
  {
    if (TOLOWER(text[l]) != pattern[l]) return 0;
  }
  return 1;
}
extern "C"

void StringSearchNaive (
    const uchar* text,
    const uint textLength,
    const uchar* pattern,
    const uint patternLength,
    uint* resultBuffer,
    uint* resultCountPerWG,
    const uint maxSearchLength)
{
    #pragma HLS INTERFACE m_axi port=text offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=textLength
    #pragma HLS INTERFACE m_axi port=pattern offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=patternLength
    #pragma HLS INTERFACE m_axi port=resultBuffer offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=resultCountPerWG offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=maxSearchLength
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=localPattern complete dim=1
    #pragma HLS ARRAY_PARTITION variable=localPattern complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uchar localPattern[4096];
            uint groupSuccessCounter;

            int localIdx = _tid_x;
            int localSize = BLOCK_DIM_X;
            int groupIdx = _bid_x;

            // Last search idx for all work items
            uint lastSearchIdx = textLength - patternLength + 1;

            // global idx for all work items in a WorkGroup
            uint beginSearchIdx = groupIdx * maxSearchLength;
            uint endSearchIdx = beginSearchIdx + maxSearchLength;
            if(beginSearchIdx > lastSearchIdx) return;
            if(endSearchIdx > lastSearchIdx) endSearchIdx = lastSearchIdx;

            // Copy the pattern from global to local buffer
            for(int idx = localIdx; idx < patternLength; idx+=localSize)
            {
            localPattern[idx] = TOLOWER(pattern[idx]);
            }

            if(localIdx == 0) groupSuccessCounter = 0;

            // loop over positions in global buffer
            for(uint stringPos=beginSearchIdx+localIdx; stringPos<endSearchIdx; stringPos+=localSize)
            {
            if (compare(text+stringPos, localPattern, patternLength) == 1)
            {
            int count = (groupSuccessCounter += (uint)1);
            resultBuffer[beginSearchIdx+count] = stringPos;
            }
            }
            if(localIdx == 0) resultCountPerWG[groupIdx] = groupSuccessCounter;

        }
    }
}
extern "C"

void StringSearchLoadBalance (
    const uchar* text,
    const uint textLength,
    const uchar* pattern,
    const uint patternLength,
    uint* resultBuffer,
    uint* resultCountPerWG,
    const uint maxSearchLength)
{
    #pragma HLS INTERFACE m_axi port=text offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=textLength
    #pragma HLS INTERFACE m_axi port=pattern offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=patternLength
    #pragma HLS INTERFACE m_axi port=resultBuffer offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=resultCountPerWG offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=maxSearchLength
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=localPattern complete dim=1
    #pragma HLS ARRAY_PARTITION variable=localPattern complete dim=1
    #pragma HLS ARRAY_PARTITION variable=stack1 complete dim=1
    #pragma HLS ARRAY_PARTITION variable=stack2 complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uchar localPattern[4096];
            uint stack1[LOCAL_SIZE*2];
            uint stack2[LOCAL_SIZE*2];
            uint stack1Counter;
            uint stack2Counter;
            uint groupSuccessCounter;

            int localIdx = _tid_x;
            int localSize = BLOCK_DIM_X;
            int groupIdx = _bid_x;

            // Initialize the local variaables
            if(localIdx == 0)
            {
            groupSuccessCounter = 0;
            stack1Counter = 0;
            stack2Counter = 0;
            }

            // Last search idx for all work items
            uint lastSearchIdx = textLength - patternLength + 1;
            uint stackSize = 0;

            // global idx for all work items in a WorkGroup
            uint beginSearchIdx = groupIdx * maxSearchLength;
            uint endSearchIdx = beginSearchIdx + maxSearchLength;
            if(beginSearchIdx > lastSearchIdx) return;
            if(endSearchIdx > lastSearchIdx) endSearchIdx = lastSearchIdx;
            uint searchLength = endSearchIdx - beginSearchIdx;

            // Copy the pattern from global to local buffer
            for(uint idx = localIdx; idx < patternLength; idx+=localSize)
            {
            localPattern[idx] = TOLOWER(pattern[idx]);
            }

            uchar first = localPattern[0];
            uchar second = localPattern[1];
            int stringPos = localIdx;
            int stackPos = 0;
            int revStackPos = 0;

            while (true)    // loop over positions in global buffer
            {

            // Level-1 : Quick filter on 2 char match and store the good positions on stack1.
            if(stringPos < searchLength)
            {
            // Queue the initial match positions. Make sure queue has sufficient positions for each work-item.
            if ((first == TOLOWER(text[beginSearchIdx+stringPos])) && (second == TOLOWER(text[beginSearchIdx+stringPos+1])))
            {
            stackPos = (stack1Counter += (uint)1);
            stack1[stackPos] = stringPos;
            }
            }

            stringPos += localSize;     // next search idx
            stackSize = stack1Counter;

            // continue until stack1 has sufficient good positions for proceed to next Level
            if((stackSize < localSize) && ((((stringPos)/localSize)*localSize) < searchLength)) continue;

            #ifdef ENABLE_2ND_LEVEL_FILTER
            // Level-2 : (Processing the stack1 and filling the stack2) For large patterns roll over
            // another 8-bytes from the positions in stack1 and store the match positions in stack2.
            if(localIdx < stackSize)
            {
            revStackPos = atomicSub(&stack1Counter, (uint)1);
            uint pos = stack1[--revStackPos];
            bool status = (localPattern[2] == TOLOWER(text[beginSearchIdx+pos+2]));
            status = status && (localPattern[3] == TOLOWER(text[beginSearchIdx+pos+3]));
            status = status && (localPattern[4] == TOLOWER(text[beginSearchIdx+pos+4]));
            status = status && (localPattern[5] == TOLOWER(text[beginSearchIdx+pos+5]));
            status = status && (localPattern[6] == TOLOWER(text[beginSearchIdx+pos+6]));
            status = status && (localPattern[7] == TOLOWER(text[beginSearchIdx+pos+7]));
            status = status && (localPattern[8] == TOLOWER(text[beginSearchIdx+pos+8]));
            status = status && (localPattern[9] == TOLOWER(text[beginSearchIdx+pos+9]));

            if (status)
            {
            stackPos = (stack2Counter += (uint)1);
            stack2[stackPos] = pos;
            }
            }
            stackSize = stack2Counter;

            // continue until stack2 has sufficient good positions proceed to next level
            if((stackSize < localSize) && ((((stringPos)/localSize)*localSize) < searchLength)) continue;
            #endif

            // Level-3 : (Processing stack1/stack2) Check the remaining positions.
            if(localIdx < stackSize)
            {
            #ifdef ENABLE_2ND_LEVEL_FILTER
            revStackPos = atomicSub(&stack2Counter, (uint)1);
            int pos = stack2[--revStackPos];
            if (compare(text+beginSearchIdx+pos+10, localPattern+10, patternLength-10) == 1)
            #else
            revStackPos = atomicSub(&stack1Counter, (uint)1);
            int pos = stack1[--revStackPos];
            if (compare(text+beginSearchIdx+pos+2, localPattern+2, patternLength-2) == 1)
            #endif
            {
            // Full match found
            int count = (groupSuccessCounter += (uint)1);
            resultBuffer[beginSearchIdx+count] = beginSearchIdx+pos;
            }
            }
            if((((stringPos/localSize)*localSize) >= searchLength) &&
            (stack1Counter <= 0) && (stack2Counter <= 0)) break;
            }

            if(localIdx == 0) resultCountPerWG[groupIdx] = groupSuccessCounter;

        }
    }
}
