#include "kernel.h"

// --- from main.cu ---
extern "C"
void pathfinder (
    const int* gpuWall,
    const int* gpuSrc,
          int* gpuResult,
          int* outputBuffer,
    const int iteration,
    const int theHalo,
    const int borderCols,
    const int cols,
    const int t)
{
    #pragma HLS INTERFACE m_axi port=gpuWall offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=gpuSrc offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=gpuResult offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=outputBuffer offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=iteration
    #pragma HLS INTERFACE s_axilite port=theHalo
    #pragma HLS INTERFACE s_axilite port=borderCols
    #pragma HLS INTERFACE s_axilite port=cols
    #pragma HLS INTERFACE s_axilite port=t
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=prev complete dim=1
    #pragma HLS ARRAY_PARTITION variable=result complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int BLOCK_SIZE = BLOCK_DIM_X;
            int bx = _bid_x;
            int tx = _tid_x;
            int prev[250];
            int result[250];

            // Each block finally computes result for a small block
            // after N iterations.
            // it is the non-overlapping small blocks that cover
            // all the input data

            // calculate the small block size.
            int small_block_cols = BLOCK_SIZE - (iteration*theHalo*2);

            // calculate the boundary for the block according to
            // the boundary of its small block
            int blkX = (small_block_cols*bx) - borderCols;
            int blkXmax = blkX+BLOCK_SIZE-1;

            // calculate the global thread coordination
            int xidx = blkX+tx;

            // effective range within this block that falls within
            // the valid range of the input data
            // used to rule out computation outside the boundary.
            int validXmin = (blkX < 0) ? -blkX : 0;
            int validXmax = (blkXmax > cols-1) ? BLOCK_SIZE-1-(blkXmax-cols+1) : BLOCK_SIZE-1;

            int W = tx-1;
            int E = tx+1;

            W = (W < validXmin) ? validXmin : W;
            E = (E > validXmax) ? validXmax : E;

            bool isValid = IN_RANGE(tx, validXmin, validXmax);

            if(IN_RANGE(xidx, 0, cols-1))
            {
            prev[tx] = gpuSrc[xidx];
            }

            bool computed;
            for (int i = 0; i < iteration; i++)
            {
            computed = false;

            if( IN_RANGE(tx, i+1, BLOCK_SIZE-i-2) && isValid )
            {
            computed = true;
            int left = prev[W];
            int up = prev[tx];
            int right = prev[E];
            int shortest = MIN(left, up);
            shortest = MIN(shortest, right);

            int index = cols*(t+i)+xidx;
            result[tx] = shortest + gpuWall[index];

            // ===================================================================
            // add debugging info to the debug output buffer...
            if (tx==11 && i==0)
            {
            // set bufIndex to what value/range of values you want to know.
            int bufIndex = gpuSrc[xidx];
            // dont touch the line below.
            outputBuffer[bufIndex] = 1;
            }
            // ===================================================================
            }

            if(i==iteration-1)
            {
            // we are on the last iteration, and thus don't need to
            // compute for the next step.
            break;
            }

            if(computed)
            {
            //Assign the computation range
            prev[tx] = result[tx];
            }
            }

            // update the global memory
            // after the last iteration, only threads coordinated within the
            // small block perform the calculation and switch on "computed"
            if (computed)
            {
            gpuResult[xidx] = result[tx];
            }

        }
    }
}
