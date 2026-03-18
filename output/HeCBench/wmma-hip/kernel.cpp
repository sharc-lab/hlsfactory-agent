#include "kernel.h"

// --- from main.cu ---
extern "C"
void gemm_impl0(const uint32_t m, const uint32_t n, const uint32_t k,
                           fp16 const * a,
                           fp16 const * b,
                           fp32 const *c,
                           fp32 *d, const uint32_t lda, const uint32_t ldb,
                           const uint32_t ldc, const uint32_t ldd,
                           const fp32 alpha, const fp32 beta) {
    #pragma HLS INTERFACE s_axilite port=m
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=lda
    #pragma HLS INTERFACE s_axilite port=ldb
    #pragma HLS INTERFACE s_axilite port=ldc
    #pragma HLS INTERFACE s_axilite port=ldd
    #pragma HLS INTERFACE s_axilite port=alpha
    #pragma HLS INTERFACE s_axilite port=beta
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Create frags
                    auto fragA = wmma::fragment<wmma::matrix_a, WMMA_M, WMMA_N, WMMA_K, fp16,
                    wmma::row_major>();
                    auto fragB = wmma::fragment<wmma::matrix_b, WMMA_M, WMMA_N, WMMA_K, fp16,
                    wmma::col_major>();
                    auto fragC = wmma::fragment<wmma::accumulator, WMMA_M, WMMA_N, WMMA_K, fp32>();
                    auto fragAcc = wmma::fragment<wmma::accumulator, WMMA_M, WMMA_N, WMMA_K, fp32>();

                    wmma::fill_fragment(fragAcc, 0.0f);

                    auto cRow = _bid_x * WMMA_M;
                    auto cCol = _bid_y * WMMA_N;

                    // Load the inputs
                    for (int n = 0; n < k; n += WMMA_K) {
                    // Because the mapping of elements to threads in a warp is opaque,
                    // each thread just passes the address of the first element
                    wmma::load_matrix_sync(fragA, a + cRow * lda + n, lda);
                    wmma::load_matrix_sync(fragB, b + cCol * ldb + n, ldb);

                    // Matrix multiply - accumulate using MFMA units
                    wmma::mma_sync(fragAcc, fragA, fragB, fragAcc);

                    // Fetch C matrix
                    wmma::load_matrix_sync(fragC, c + cRow * ldc + cCol, ldc, wmma::mem_row_major);

                    // D = alpha * A x B + beta * C
                    for (int i = 0; i < fragC.num_elements; ++i) {
                    fragC.x[i] = alpha * fragAcc.x[i] + beta * fragC.x[i];
                    }
                    }

                    // Store to D (by a single wave)
                    wmma::store_matrix_sync(d + cRow * ldd + cCol, fragC, ldd, wmma::mem_row_major);

                }
            }
        }
    }
}
extern "C"

void gemm_impl1(const uint32_t m, const uint32_t n, const uint32_t k,
                           fp16 const * a,
                           fp16 const * b,
                           fp32 const *c,
                           fp32 *d, const uint32_t lda, const uint32_t ldb,
                           const uint32_t ldc, const uint32_t ldd,
                           const fp32 alpha, const fp32 beta) {
    #pragma HLS INTERFACE s_axilite port=m
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=lda
    #pragma HLS INTERFACE s_axilite port=ldb
    #pragma HLS INTERFACE s_axilite port=ldc
    #pragma HLS INTERFACE s_axilite port=ldd
    #pragma HLS INTERFACE s_axilite port=alpha
    #pragma HLS INTERFACE s_axilite port=beta
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_y = 0; _bid_y < GRID_DIM_Y; _bid_y++) {
        for (int _tid_y = 0; _tid_y < BLOCK_DIM_Y; _tid_y++) {
            for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
                for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
                #pragma HLS PIPELINE II=1

                    // Create frags
                    auto fragA = wmma::fragment<wmma::matrix_a, WMMA_M, WMMA_N, WMMA_K, fp16,
                    wmma::row_major>();
                    auto fragB = wmma::fragment<wmma::matrix_b, WMMA_M, WMMA_N, WMMA_K, fp16,
                    wmma::col_major>();
                    auto fragC = wmma::fragment<wmma::accumulator, WMMA_M, WMMA_N, WMMA_K, fp32>();
                    auto fragAcc = wmma::fragment<wmma::accumulator, WMMA_M, WMMA_N, WMMA_K, fp32>();

                    wmma::fill_fragment(fragAcc, 0.0f);

                    // Map threadIdx to warpIdx
                    auto warpIdx = _tid_x / WAVE_SIZE;
                    auto warpIdy = _tid_y;

                    // Target C block
                    auto cRow = _bid_x * TILE_M + warpIdx * WMMA_M;
                    auto cCol = _bid_y * TILE_N + warpIdy * WMMA_N;

                    // Bounds check
                    for (int n = 0; n < k; n += WMMA_K) {
                    // Load the inputs
                    wmma::load_matrix_sync(fragA, a + (cRow * lda + n), lda);
                    wmma::load_matrix_sync(fragB, b + (cCol * ldb + n), ldb);

                    // Matrix multiply - accumulate using MFMA units
                    wmma::mma_sync(fragAcc, fragA, fragB, fragAcc);
                    }

                    // Fetch C matrix
                    wmma::load_matrix_sync(fragC, c + (cRow * ldc + cCol), ldc,
                    wmma::mem_row_major);

                    // D = alpha * A x B + beta * C
                    for (int i = 0; i < fragC.num_elements; ++i) {
                    fragC.x[i] = alpha * fragAcc.x[i] + beta * fragC.x[i];
                    }

                    // Store to D
                    wmma::store_matrix_sync(d + (cRow * ldd + cCol), fragC, ldd,
                    wmma::mem_row_major);

                }
            }
        }
    }
}
