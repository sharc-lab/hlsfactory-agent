#include "kernel.h"

// --- from heat.cu ---
extern "C"
void initial_value(const unsigned int n, const double dx, const double length, double * u) {
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=dx
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=dx
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int idx = BLOCK_DIM_X * _bid_x + _tid_x;
            if (idx < n*n) {
            int i = idx % n;
            int j = idx / n;
            double y = dx * (j+1); // Physical y position
            double x = dx * (i+1); // Physical x position
            u[i+j*n] = sin(acos(-1.0) * x / length) * sin(acos(-1.0) * y / length);
            }

        }
    }
 = idx / n;
            double y = dx * (j+1); // Physical y position
            double x = dx * (i+1); // Physical x position
            u[i+j*n] = sin(acos(-1.0) * x / length) * sin(acos(-1.0) * y / length);
            }

        }
    }
}
extern "C"

void zero(const unsigned int n, double * u) {
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int idx = BLOCK_DIM_X * _bid_x + _tid_x;
            if (idx < n*n) u[idx] = 0.0;

        }
    }
_tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int idx = BLOCK_DIM_X * _bid_x + _tid_x;
            if (idx < n*n) u[idx] = 0.0;

        }
    }
}
extern "C"

void solve(const unsigned int n, const double alpha, const double dx, const double dt, 
		const double r, const double r2,
		double *  u, double *  u_tmp) {
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=alpha
    #pragma HLS INTERFACE s_axilite port=dx
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE s_axilite port=r
    #pragma HLS INTERFACE s_axilite port=r2
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=u_tmp offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=alpha
    #pragma HLS INTERFACE s_axilite port=dx
    #pragma HLS INTERFACE s_axilite port=dt
    #pragma HLS INTERFACE s_axilite port=r
    #pragma HLS INTERFACE s_axilite port=r2
    #pragma HLS INTERFACE m_axi port=u offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=u_tmp offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int idx = BLOCK_DIM_X * _bid_x + _tid_x;
            if (idx < n * n) {
            int i = idx % n;
            int j = idx / n;
            // Boundaries are zero because the MMS solution is zero there.
            u_tmp[i+j*n] =  r2 * u[i+j*n] +
            r * ((i < n-1) ? u[i+1+j*n] : 0.0) +
            r * ((i > 0)   ? u[i-1+j*n] : 0.0) +
            r * ((j < n-1) ? u[i+(j+1)*n] : 0.0) +
            r * ((j > 0)   ? u[i+(j-1)*n] : 0.0);
            }

        }
    }
     u_tmp[i+j*n] =  r2 * u[i+j*n] +
            r * ((i < n-1) ? u[i+1+j*n] : 0.0) +
            r * ((i > 0)   ? u[i-1+j*n] : 0.0) +
            r * ((j < n-1) ? u[i+(j+1)*n] : 0.0) +
            r * ((j > 0)   ? u[i+(j-1)*n] : 0.0);
            }

        }
    }
}
