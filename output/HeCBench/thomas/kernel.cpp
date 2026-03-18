#include "kernel.h"

// --- from cuThomasBatch.cu ---
extern "C"
void cuThomasBatch(
            const double *L, const double *D, double *U, double *RHS,
            const int M,
            const int BATCHCOUNT
    ) {
    #pragma HLS INTERFACE m_axi port=L offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=D offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=U offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=RHS offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=BATCHCOUNT
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE m_axi port=L offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=D offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=U offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=RHS offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=M
    #pragma HLS INTERFACE s_axilite port=BATCHCOUNT
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int tid = _tid_x + BLOCK_DIM_X*_bid_x;

            if(tid < BATCHCOUNT) {

            int first = tid;
            int last  = BATCHCOUNT*(M-1)+tid;

            U[first] /= D[first];
            RHS[first] /= D[first];

            for (int i = first + BATCHCOUNT; i < last; i+=BATCHCOUNT) {
            U[i] /= D[i] - L[i] * U[i-BATCHCOUNT];
            RHS[i] = ( RHS[i] - L[i] * RHS[i-BATCHCOUNT] ) /
            ( D[i] - L[i] * U[i-BATCHCOUNT] );
            }

            RHS[last] = ( RHS[last] - L[last] * RHS[last-BATCHCOUNT] ) /
            ( D[last] - L[last] * U[last-BATCHCOUNT] );

            for (int i = last-BATCHCOUNT; i >= first; i-=BATCHCOUNT) {
            RHS[i] -= U[i] * RHS[i+BATCHCOUNT];
            }
            }


        }
    }
UNT] );

            for (int i = last-BATCHCOUNT; i >= first; i-=BATCHCOUNT) {
            RHS[i] -= U[i] * RHS[i+BATCHCOUNT];
            }
            }


        }
    }
}
