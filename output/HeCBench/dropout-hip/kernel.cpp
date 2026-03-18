#include "kernel.h"

// --- from main.cu ---
extern "C"
void fused_dropout_kernel(
  const scalar_t * a,
        scalar_t * b,
         mask_t * c,
  IndexType totalElements,
  accscalar_t p,
  std::pair<uint64_t, uint64_t> seeds) 
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=totalElements
    #pragma HLS INTERFACE s_axilite port=p
    #pragma HLS INTERFACE s_axilite port=std::pair<uint64_t
    #pragma HLS INTERFACE s_axilite port=seeds
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            accscalar_t scale = accscalar_t(1)/p;
            IndexType idx = _bid_x * BLOCK_DIM_X + _tid_x;

            hiprandStatePhilox4_32_10_t state;
            hiprand_init(seeds.first, idx, seeds.second, &state);

            IndexType rounded_size = ((totalElements - 1)/(BLOCK_DIM_X * GRID_DIM_X * UNROLL)+1) *
            BLOCK_DIM_X * GRID_DIM_X * UNROLL;

            for (IndexType linearIndex = idx;
            linearIndex < rounded_size;
            linearIndex += GRID_DIM_X * BLOCK_DIM_X * UNROLL) {

            float4 rand = hiprand_uniform4(&state);
            scalar_t src[UNROLL];
            rand.x = rand.x < p;
            rand.y = rand.y < p;
            rand.z = rand.z < p;
            rand.w = rand.w < p;

            #pragma unroll
            for (int ii = 0; ii < UNROLL; ii++) {
            IndexType li = linearIndex + BLOCK_DIM_X * GRID_DIM_X * ii;
            if (li < totalElements) {
            const IndexType aOffset = li;
            src[ii] = a[aOffset];
            }
            }

            #pragma unroll
            for (int ii = 0; ii < UNROLL; ii++) {
            IndexType li = linearIndex + BLOCK_DIM_X * GRID_DIM_X * ii;
            if (li < totalElements) {
            const IndexType bOffset = li;
            b[bOffset] = src[ii]*(&rand.x)[ii]*scale;
            c[bOffset] = (mask_t)(&rand.x)[ii];
            }
            }
            }

        }
    }
}
extern "C"

void fused_dropout_kernel_vec(
  const scalar_t * a,
        scalar_t * b,
         mask_t * c,
  IndexType totalElements,
  accscalar_t p,
  std::pair<uint64_t, uint64_t> seeds) 
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=totalElements
    #pragma HLS INTERFACE s_axilite port=p
    #pragma HLS INTERFACE s_axilite port=std::pair<uint64_t
    #pragma HLS INTERFACE s_axilite port=seeds
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            using LoadT = aligned_vector<scalar_t, VEC>;
            using MaskLoadT = aligned_vector<mask_t, VEC>;

            // Helps align the total number of times hiprand_uniform4 is called by each thread for the same totalElements
            bool gridxvec_loop_state = 0;

            accscalar_t scale = accscalar_t(1)/p;
            IndexType idx = _bid_x * BLOCK_DIM_X + _tid_x;

            hiprandStatePhilox4_32_10_t state;
            hiprand_init(seeds.first, idx, seeds.second, &state);

            // Note: Vectorized loads means we'll stride each thread by an additional VEC factor, as we'll load VEC elements at a time
            for (IndexType linearIndex = idx * VEC;
            linearIndex < totalElements;
            linearIndex += GRID_DIM_X * BLOCK_DIM_X * VEC) {

            scalar_t src[VEC];
            LoadT *value = reinterpret_cast<LoadT*>(&src);

            float4 rand;
            if ((VEC == 4) || (gridxvec_loop_state == 0)) {
            rand = hiprand_uniform4(&state);
            } else {
            // sets up the last two values we generated last iteration to be used this iteration.
            rand.x = rand.z;
            rand.y = rand.w;
            gridxvec_loop_state ^= 1;
            }
            rand.x = rand.x < p;
            rand.y = rand.y < p;
            if (VEC == 4) {
            rand.z = rand.z < p;
            rand.w = rand.w < p;
            }

            *value = *reinterpret_cast<const LoadT*>(&a[linearIndex]);

            scalar_t r[VEC];
            mask_t mask[VEC];

            // Perform the actual computation
            #pragma unroll
            for (int ii = 0; ii < VEC; ii++) {
            r[ii] = src[ii]*(&rand.x)[ii]*scale;
            mask[ii] = (mask_t)(&rand.x)[ii];
            }
            // Vectorized writes for both mask & result
            *(reinterpret_cast<LoadT*>(&b[linearIndex])) = *reinterpret_cast<LoadT*>(&r[0]);
            *(reinterpret_cast<MaskLoadT*>(&c[linearIndex])) = *reinterpret_cast<MaskLoadT*>(&mask[0]);
            }

        }
    }
}
