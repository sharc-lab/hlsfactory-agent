#include "kernel.h"

// --- from kernels.cu ---
float calNumEigenValuesLessThan(
   const float x, 
   const uint width, 
   const float * diagonal, 
   const float * offDiagonal)
{
  uint count = 0;

  float prev_diff = (diagonal[0] - x);
  count += (prev_diff < 0)? 1 : 0;
  for(uint i = 1; i < width ; i += 1)
  {
    float diff = (diagonal[i] - x) - ((offDiagonal[i-1] * offDiagonal[i-1]) / prev_diff);

    count += (diff < 0) ? 1 : 0;
    prev_diff = diff;
  }
  return count;
}
extern "C"

void calNumEigenValueInterval(
    uint  * numEigenIntervals,
    const float * eigenIntervals,
    const float * diagonal, 
    const float * offDiagonal,
    const uint     width)
{
    #pragma HLS INTERFACE m_axi port=numEigenIntervals offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=eigenIntervals offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=diagonal offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=offDiagonal offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uint gid = _bid_x * BLOCK_DIM_X + _tid_x;
            uint lowerId = 2 * gid;
            uint upperId = lowerId + 1;
            float lowerLimit = eigenIntervals[lowerId];
            float upperLimit = eigenIntervals[upperId];
            uint lower = calNumEigenValuesLessThan(lowerLimit, width, diagonal, offDiagonal);
            uint upper = calNumEigenValuesLessThan(upperLimit, width, diagonal, offDiagonal);
            numEigenIntervals[gid] = upper - lower;

        }
    }
}
extern "C"

void recalculateEigenIntervals(
          float * newEigenIntervals,
    const float * eigenIntervals,
    const uint  * numEigenIntervals,
    const float * diagonal,
    const float * offDiagonal,
    const    uint    width,  
    const    float   tolerance)
{
    #pragma HLS INTERFACE m_axi port=newEigenIntervals offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=eigenIntervals offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=numEigenIntervals offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=diagonal offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=offDiagonal offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=width
    #pragma HLS INTERFACE s_axilite port=tolerance
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uint gid = _bid_x * BLOCK_DIM_X + _tid_x;
            uint lowerId = 2 * gid;
            uint upperId = lowerId + 1;
            uint currentIndex = gid;

            uint index = 0;
            while(currentIndex >= numEigenIntervals[index])
            {
            currentIndex -= numEigenIntervals[index];
            ++index;
            }

            uint lId = 2 * index;
            uint uId = lId + 1;

            /* if the number of eigenvalues in the interval is just 1 */
            if(numEigenIntervals[index] == 1)
            {
            float midValue = (eigenIntervals[uId] + eigenIntervals[lId])/2;
            float n        = calNumEigenValuesLessThan(midValue, width, diagonal, offDiagonal);
            n -= calNumEigenValuesLessThan(eigenIntervals[lId], width, diagonal, offDiagonal);

            /* check if the interval size is less than tolerance levels */
            if(eigenIntervals[uId] - eigenIntervals[lId] < tolerance)
            {
            newEigenIntervals[lowerId] = eigenIntervals[lId];
            newEigenIntervals[upperId] = eigenIntervals[uId];
            }
            else if(n == 0) /* if the eigenvalue lies in the right half of the interval */
            {
            newEigenIntervals[lowerId] = midValue;
            newEigenIntervals[upperId] = eigenIntervals[uId];
            }
            else           /* if the eigenvalue lies in the left half of the interval */
            {
            newEigenIntervals[lowerId] = eigenIntervals[lId];
            newEigenIntervals[upperId] = midValue;
            }
            }
            /* split the intervals into equal intervals of size divisionWidth */
            else /* (numEigenIntervals[index] > 1) */
            {
            float divisionWidth = (eigenIntervals[uId] - eigenIntervals[lId]) / numEigenIntervals[index];
            newEigenIntervals[lowerId] = eigenIntervals[lId] + divisionWidth * currentIndex;
            newEigenIntervals[upperId] = newEigenIntervals[lowerId] + divisionWidth;
            }

        }
    }
}
