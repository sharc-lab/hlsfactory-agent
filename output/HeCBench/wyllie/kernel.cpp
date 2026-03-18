#include "kernel.h"

// --- from main.cu ---
extern "C"
void wyllie ( long *list , const int size )
{
    #pragma HLS INTERFACE m_axi port=list offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            if(index < size )
            {
            long node, next;
            while ( ((node = list[index]) >> 32) != NIL &&
            ((next = list[node >> 32]) >> 32) != NIL )
            {
            long temp = (node & MASK) ;
            temp += (next & MASK) ;
            temp += (next >> 32) << 32;
            list [ index ] = temp ;
            }
            }

        }
    }
}
