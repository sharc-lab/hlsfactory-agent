#include "kernel.h"

// --- from main.cu ---
static inline void gpuAtomicAddNoReturn(int *address, int val) {
  (*address += val);
}
extern "C"

void bincount (
       output_t *output,
  const input_t *input,
  IndexType nbins,
  input_t minvalue,
  input_t maxvalue,
  IndexType input_size,
  IndexType output_size)
{
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=nbins
    #pragma HLS INTERFACE s_axilite port=minvalue
    #pragma HLS INTERFACE s_axilite port=maxvalue
    #pragma HLS INTERFACE s_axilite port=input_size
    #pragma HLS INTERFACE s_axilite port=output_size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=my_smem complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned char my_smem[4096];
            output_t* smem = nullptr;

            if (MemoryType == DeviceMemoryType::SHARED) {
            // atomically add to block specific shared memory
            // then atomically add to the global output tensor
            smem = reinterpret_cast<output_t*>(my_smem);
            for (IndexType i = _tid_x; i < nbins; i += BLOCK_DIM_X) {
            smem[i] = 0;
            }

            FOR_KERNEL_LOOP(linearIndex, input_size) {
            const auto v = input[linearIndex];

            if (v >= minvalue && v <= maxvalue) {
            const IndexType bin = getBin<input_t, IndexType>(
            v, minvalue, maxvalue, nbins);
            gpuAtomicAddNoReturn(&smem[bin], 1);
            }
            }

            // Atomically update output bin count.
            for (IndexType i = _tid_x; i < nbins; i += BLOCK_DIM_X) {
            gpuAtomicAddNoReturn(&output[i], smem[i]);
            }

            } else {
            ////////////////////////// Global memory //////////////////////////
            // atomically add to the output tensor
            // compute histogram for the block
            FOR_KERNEL_LOOP(linearIndex, input_size) {
            const auto v = input[linearIndex];
            if (v >= minvalue && v <= maxvalue) {
            const IndexType bin = getBin<input_t, IndexType>(
            v, minvalue, maxvalue, nbins);
            gpuAtomicAddNoReturn(&output[bin], 1);
            }
            }
            }

        }
    }
}
