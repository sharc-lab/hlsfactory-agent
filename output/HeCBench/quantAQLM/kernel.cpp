#include "kernel.h"

// --- from main.cu ---
extern "C"
void Code1x16MatVec(
    const int4*  A, const int4*  B,
    int4*  C, const int4*  codebook, const int prob_m,
    const int prob_k,
    const int4 codebook_a_sizes,  // cumulative sizes of A spanning each
                                  // codebook, at most 3 long.
    const int codebook_stride     // as int4.
) {
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=codebook offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=prob_m
    #pragma HLS INTERFACE s_axilite port=prob_k
    #pragma HLS INTERFACE s_axilite port=codebook_a_sizes
    #pragma HLS INTERFACE s_axilite port=codebook
    #pragma HLS INTERFACE s_axilite port=int4.
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=sh_b complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int a_gl_stride = prob_k / 8 / 8;
            int a_gl_rd = (BLOCK_DIM_X / warpSize) * _bid_x + (_tid_x / warpSize);
            bool pred = a_gl_rd < prob_m;

            if (pred) {
            // advance to the correct codebook, this easy because we only multiply one
            // column of the codebook.
            auto codebook_size = &codebook_a_sizes.x;
            while (a_gl_rd >= *codebook_size) {
            codebook += codebook_stride;
            ++codebook_size;
            }
            }

            int b_gl_rd = 0;
            int c_gl_wr = a_gl_rd;
            a_gl_rd = a_gl_stride * a_gl_rd + _tid_x % warpSize;
            int a_gl_end = a_gl_rd + a_gl_stride - _tid_x % warpSize;

            // We pad shared memory to avoid bank conflicts during reads
            int4 sh_b[32 * F]; // 32 is warpSize
            float res = 0;

            int iters = (prob_k / 8 + 8 * warpSize - 1) / (8 * warpSize);
            while (iters--) {
            for (int i = _tid_x; i < warpSize * 8; i += BLOCK_DIM_X) {
            if (b_gl_rd + i < prob_k / 8) sh_b[F * (i / 8) + i % 8] = B[b_gl_rd + i];
            }
            b_gl_rd += warpSize * 8;

            int b_sh_rd = F * (_tid_x % warpSize);
            if (pred && a_gl_rd < a_gl_end) {
            const uint16_t* enc = reinterpret_cast<const uint16_t*>(&A[a_gl_rd]);
            #pragma unroll
            for (int i = 0; i < 8; i++) {
            uint32_t dec[4];
            #ifdef PTX
            // We bypass the L1 cache to avoid massive amounts of memory streaming
            // that doesn't actually help us; this brings > 2x speedup.
            asm volatile("ld.cg.global.v4.u32 {%0, %1, %2, %3}, [%4];"
            : "=r"(dec[0]), "=r"(dec[1]), "=r"(dec[2]), "=r"(dec[3])
            : "l"((void*)&codebook[enc[i]]));
            #else
            int4 t = codebook[enc[i]];
            dec[0] = t.x;
            dec[1] = t.y;
            dec[2] = t.z;
            dec[3] = t.w;
            #endif
            //printf("%d %d %u %u %u %u\n", _bid_x, _tid_x, dec[0], dec[1], dec[2], dec[3]);

            half2* a = reinterpret_cast<half2*>(&dec);
            half2* b = reinterpret_cast<half2*>(&sh_b[b_sh_rd]);
            half2 res2 = {};
            #pragma unroll
            for (int j = 0; j < 4; j++) res2 = __hfma2(a[j], b[j], res2);
            res += __half2float(res2.x) + __half2float(res2.y);
            b_sh_rd++;
            }
            a_gl_rd += warpSize;
            }
            }

            if (pred) {
            #pragma unroll
            for (int i = warpSize/2; i > 0; i /= 2) res += 0;
            if (_tid_x % warpSize == 0) {
            reinterpret_cast<__half*>(C)[c_gl_wr] = __float2half(res);
            }
            }

        }
    }
}
