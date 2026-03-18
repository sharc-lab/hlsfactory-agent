#include "kernel.h"

// --- from gpu_solver.cu ---
extern "C"
void QRdel(int n, const float *A, const float *B, const float *C,
                      const float *D, float * b,
                      float * c, float * d,
                      float * Q, float * R,
                      float * Qint, float * Rint,
                      float * del) {
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=D offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=Q offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=R offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=Qint offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=Rint offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=del offset=slave bundle=gmem11
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=D offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=Q offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=R offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=Qint offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=Rint offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=del offset=slave bundle=gmem11
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i < n) {

            b[i] = 0.75f * (B[i] / A[i]);
            c[i] = 0.50f * (C[i] / A[i]);
            d[i] = 0.25f * (D[i] / A[i]);

            Q[i] = (c[i] / 3.f) - ((b[i] * b[i]) / 9.f);
            R[i] = (b[i] * c[i]) / 6.f - (b[i] * b[i] * b[i]) / 27.f - 0.5f * d[i];

            // round Q and R to get around problems caused by floating point precision
            Q[i] = roundf(Q[i] * 1E5f) / 1E5f;
            R[i] = roundf(R[i] * 1E5f) / 1E5f;

            Qint[i] = (Q[i] * Q[i] * Q[i]);
            Rint[i] = (R[i] * R[i]);

            del[i] = Rint[i] + Qint[i];
            // del[i] = (R[i] * R[i]) + (Q[i] * Q[i] * Q[i]); // why not just Q*Q*Q +
            // R*R? Heisenbug. Heisenbug in release code
            }

        }
    }
) / 1E5f;

            Qint[i] = (Q[i] * Q[i] * Q[i]);
            Rint[i] = (R[i] * R[i]);

            del[i] = Rint[i] + Qint[i];
            // del[i] = (R[i] * R[i]) + (Q[i] * Q[i] * Q[i]); // why not just Q*Q*Q +
            // R*R? Heisenbug. Heisenbug in release code
            }

        }
    }
}
