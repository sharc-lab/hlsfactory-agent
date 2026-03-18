#include "kernel.h"

// --- from main.cu ---
__inline__ opmath_t gelu(opmath_t x) {
    constexpr opmath_t kAlpha = M_SQRT1_2;
    return x * opmath_t(0.5) * (opmath_t(1) + erf(x * kAlpha));
}
extern "C"

void geglu_kernel(scalar_t *out, const scalar_t *x_and_gate)
{
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=x_and_gate offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            static_assert(DIM_LAST % (BLOCK_DIM_X * VEC_ELEMS) == 0, "cannot decide CHUNKS_PER_ROW");
            constexpr int CHUNKS_PER_ROW = DIM_LAST / (BLOCK_DIM_X * VEC_ELEMS);
            struct alignas(sizeof(scalar_t) * VEC_ELEMS) U {
            scalar_t data[VEC_ELEMS];
            };
            U ux[FOR_LOOP];
            U ugate[FOR_LOOP];
            U uout[FOR_LOOP];
            for (int k = 0; k < FOR_LOOP; k++) {
            int idxN = (_bid_x * FOR_LOOP + k) / CHUNKS_PER_ROW;
            int idxR = ((_bid_x * FOR_LOOP + k) % CHUNKS_PER_ROW * BLOCK_DIM_X + _tid_x) * VEC_ELEMS;
            ux[k]    = *reinterpret_cast<U const *>(&x_and_gate[(idxN * 2 + 0) * (int64_t)DIM_LAST + idxR]);
            ugate[k] = *reinterpret_cast<U const *>(&x_and_gate[(idxN * 2 + 1) * (int64_t)DIM_LAST + idxR]);
            }
            for (int k = 0; k < FOR_LOOP; k++) {
            for (int i = 0; i < VEC_ELEMS; i++) {
            opmath_t gelu_out = gelu(static_cast<opmath_t>(ugate[k].data[i]));
            uout[k].data[i] = static_cast<scalar_t>(static_cast<opmath_t>(ux[k].data[i]) * gelu_out);
            }
            }
            for (int k = 0; k < FOR_LOOP; k++) {
            int idxN = (_bid_x * FOR_LOOP + k) / CHUNKS_PER_ROW;
            int idxR = ((_bid_x * FOR_LOOP + k) % CHUNKS_PER_ROW * BLOCK_DIM_X + _tid_x) * VEC_ELEMS;
            *reinterpret_cast<U *>(&out[idxN * (int64_t)DIM_LAST + idxR]) = uout[k];
            }

        }
    }
}
