#include "kernel.h"

// --- from kernels.cu ---
double atomicMax(double *address, double val)
{
  unsigned long long ret = __double_as_longlong(*address);
  while(val > __longlong_as_double(ret))
  {
    unsigned long long old = ret;
    if((ret = atomicCAS((unsigned long long *)address, old, __double_as_longlong(val))) == old)
      break;
  }
  return __longlong_as_double(ret);
}
extern "C"

void kernel (double * lmax,
             double * linterp,
             const double * xfun, 
             const double * x,
             const int n, const int nfun) 
{
    #pragma HLS INTERFACE m_axi port=lmax offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=linterp offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=xfun offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=nfun
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int j = _bid_x * BLOCK_DIM_X + _tid_x;
            if (j >= nfun) return;

            double t = 0.0;
            for (int i1 = 0; i1 < n; i1++ ) {
            linterp[i1*nfun+j] = 1.0;
            for (int i2 = 0; i2 < n; i2++ )
            if ( i1 != i2 )
            linterp[i1*nfun+j] = linterp[i1*nfun+j] * ( xfun[j] - x[i2] ) / ( x[i1] - x[i2] );
            t += fabs ( linterp[i1*nfun+j] );
            }

            (*lmax = max(*lmax, t));

        }
    }
}
