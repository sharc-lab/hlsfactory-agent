#include "kernel.h"

// --- from main.cu ---
extern "C"
void relax (const double *A_diag_data, const int *A_diag_i, const int *A_diag_j, 
             double *u_data, const double *f_data, const int n)
{
    #pragma HLS INTERFACE m_axi port=A_diag_data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=A_diag_i offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=A_diag_j offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=u_data offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=f_data offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = BLOCK_DIM_X * _bid_x + _tid_x;
            if (i >= n) return;

            /*-----------------------------------------------------------
            * If diagonal is nonzero, relax point i; otherwise, skip it.
            *-----------------------------------------------------------*/

            if ( A_diag_data[A_diag_i[i]] != 0.0)
            {
            double res = f_data[i];
            for (int jj = A_diag_i[i]+1; jj < A_diag_i[i+1]; jj++)
            {
            int ii = A_diag_j[jj];
            res -= A_diag_data[jj] * u_data[ii];
            }
            u_data[i] = res / A_diag_data[A_diag_i[i]];
            }

        }
    }
}
