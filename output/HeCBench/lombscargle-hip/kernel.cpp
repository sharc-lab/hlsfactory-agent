#include "kernel.h"

// --- from main.cu ---
extern "C"
void lombscargle( const int x_shape,
    const int freqs_shape,
    const float * x,
    const float * y,
    const float * freqs,
    float * pgram,
    const float y_dot )
{
    #pragma HLS INTERFACE s_axilite port=x_shape
    #pragma HLS INTERFACE s_axilite port=freqs_shape
    #pragma HLS INTERFACE m_axi port=x offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=y offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=freqs offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=pgram offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=y_dot
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int tx  = ( _bid_x * BLOCK_DIM_X + _tid_x ) ;
            const int stride = ( BLOCK_DIM_X * GRID_DIM_X ) ;

            for ( int tid = tx; tid < freqs_shape; tid += stride ) {

            float freq = freqs[tid] ;

            float xc = 0;
            float xs = 0;
            float cc = 0;
            float ss = 0;
            float cs = 0;
            float c;
            float s;

            for ( int j = 0; j < x_shape; j++ ) {
            sincosf( freq * x[j], &s, &c );
            xc += y[j] * c;
            xs += y[j] * s;
            cc += c * c;
            ss += s * s;
            cs += c * s;
            }

            float c_tau;
            float s_tau;
            float tau = atan2f( 2.0f * cs, cc - ss ) / ( 2.0f * freq ) ;
            sincosf( freq * tau, &s_tau, &c_tau );
            float c_tau2 = c_tau * c_tau ;
            float s_tau2 = s_tau * s_tau ;
            float cs_tau = 2.0f * c_tau * s_tau ;

            pgram[tid] = ( 0.5f * ( ( ( c_tau * xc + s_tau * xs ) * ( c_tau * xc + s_tau * xs ) /
            ( c_tau2 * cc + cs_tau * cs + s_tau2 * ss ) ) +
            ( ( c_tau * xs - s_tau * xc ) * ( c_tau * xs - s_tau * xc ) /
            ( c_tau2 * ss - cs_tau * cs + s_tau2 * cc ) ) ) ) * y_dot;
            }

        }
    }
}
