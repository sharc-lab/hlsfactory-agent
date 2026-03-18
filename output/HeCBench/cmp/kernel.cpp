#include "kernel.h"

// --- from main.cu ---
extern "C"
void init_c(real *c, real inc, real c0) 
{
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=inc
    #pragma HLS INTERFACE s_axilite port=c0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x;
            c[i] = c0 + inc*i;

        }
    }
}
extern "C"

void init_half(const real*  scalco, 
          const real*  gx, 
          const real*  gy, 
          const real*  sx, 
          const real*  sy, 
          real*  h) 
{
    #pragma HLS INTERFACE m_axi port=scalco offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=gx offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=gy offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=sx offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=sy offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=h offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x;
            real _s = scalco[i];

            if(-EPSILON < _s && _s < EPSILON) _s = 1.0f;
            else if(_s < 0) _s = 1.0f / _s;

            real hx = (gx[i] - sx[i]) * _s;
            real hy = (gy[i] - sy[i]) * _s;

            h[i] = 0.25f * (hx * hx + hy * hy) / FACTOR;

        }
    }
}
extern "C"

void compute_semblances(const real*  h, 
                   const real*  c, 
                   const real*  samples, 
                   real*  num,
                   real*  stt,
                   int t_id0, 
                   int t_idf,
                   real _idt,
                   real _dt,
                   int _tau,
                   int _w,
                   int nc,
                   int ns) 
{
    #pragma HLS INTERFACE m_axi port=h offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=samples offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=num offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=stt offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=t_id0
    #pragma HLS INTERFACE s_axilite port=t_idf
    #pragma HLS INTERFACE s_axilite port=_idt
    #pragma HLS INTERFACE s_axilite port=_dt
    #pragma HLS INTERFACE s_axilite port=_tau
    #pragma HLS INTERFACE s_axilite port=_w
    #pragma HLS INTERFACE s_axilite port=nc
    #pragma HLS INTERFACE s_axilite port=ns
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            real _den = 0.0f, _ac_linear = 0.0f, _ac_squared = 0.0f;
            real _num[MAX_W],  m = 0.0f;
            int err = 0;

            int i = _bid_x * NTHREADS + _tid_x;

            int t0 = i / nc;
            int c_id = i % nc;

            if(i < ns * nc)
            {
            real _c = c[c_id];
            real _t0 = _dt * t0;
            _t0 *= _t0;

            for(int j=0; j < _w; j++) _num[j] = 0.0f;

            for(int t_id=t_id0; t_id < t_idf; t_id++) {
            real t = sqrtf(_t0 + _c * h[t_id]) * _idt;

            int it = (int)( t );
            int ittau = it - _tau;
            real x = t - (real)it;

            if(ittau >= 0 && it + _tau + 1 < ns) {
            int k1 = ittau + (t_id-t_id0)*ns;
            real sk1p1 = samples[k1], sk1;

            for(int j=0; j < _w; j++) {
            k1++;
            sk1 = sk1p1;
            sk1p1 = samples[k1];
            // linear interpolation optmized for this problema
            real v = (sk1p1 - sk1) * x + sk1;

            _num[j] += v;
            _den += v * v;
            _ac_linear += v;
            }
            m += 1;
            } else { err++; }
            }

            // Reduction for num
            for(int j=0; j < _w; j++) _ac_squared += _num[j] * _num[j];

            // Evaluate semblances
            if(_den > EPSILON && m > EPSILON && _w > EPSILON && err < 2) {
            num[i] = _ac_squared / (_den * m);
            stt[i] = _ac_linear  / (_w   * m);
            }
            else {
            num[i] = -1.0f;
            stt[i] = -1.0f;
            }
            }

        }
    }
}
extern "C"

void redux_semblances(const real*  num, 
                 const real*  stt, 
                 int*   ctr, 
                 real*  str, 
                 real*  stk,
                 const int nc, 
                 const int cdp_id,
                 const int ns) 
{
    #pragma HLS INTERFACE m_axi port=num offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=stt offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=ctr offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=str offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=stk offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=nc
    #pragma HLS INTERFACE s_axilite port=cdp_id
    #pragma HLS INTERFACE s_axilite port=ns
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int t0 = _bid_x * NTHREADS + _tid_x;

            if(t0 < ns)
            {
            real max_sem = 0.0f;
            int max_c = -1;

            for(int it=t0*nc; it < (t0+1)*nc ; it++) {
            real _num = num[it];
            if(_num > max_sem) {
            max_sem = _num;
            max_c = it;
            }
            }

            ctr[cdp_id*ns + t0] = max_c % nc;
            str[cdp_id*ns + t0] = max_sem;
            stk[cdp_id*ns + t0] = max_c > -1 ? stt[max_c] : 0;
            }

        }
    }
}
