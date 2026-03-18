#include "kernel.h"

// --- from main.cu ---
extern "C"
void kernel (T * const out, const std::uint64_t n, const std::uint32_t seed)
{
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=seed
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const std::uint32_t tid = BLOCK_DIM_X * _bid_x + _tid_x;

            // generate initial local seed per thread
            const std::uint32_t local_seed =
            kiss::hashers::MurmurHash<std::uint32_t>::hash(seed+tid);

            Rng rng {local_seed};

            // grid-stride loop
            const auto grid_stride = BLOCK_DIM_X * GRID_DIM_X;
            for(std::uint64_t i = tid; i < n; i += grid_stride)
            {
            // generate random element and write to output
            out[i] = rng.template next<T>();
            }

        }
    }
}
