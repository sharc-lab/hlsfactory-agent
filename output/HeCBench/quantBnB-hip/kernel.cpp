#include "kernel.h"

// --- from main.cu ---
extern "C"
void kQuantize(const float * code,
                          const float *  A,
                          uint8_t *out, const int n)
{
    #pragma HLS INTERFACE m_axi port=code offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=smem_code complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int bid = _bid_x;
            const int tid = _tid_x;
            const int n_full = GRID_DIM_X * BLOCK_SIZE;
            const int base_idx = bid * BLOCK_SIZE;

            float vals[NUM];
            uint8_t qvals[NUM];

            // 1D block of TH threads owning NUM items each
            typedef BlockLoad<float, TH, NUM> LoadFloat;
            typedef BlockStore<uint8_t, TH, NUM> StoreChar;

            typename LoadFloat::TempStorage loadf_storage;
            typename StoreChar::TempStorage storec_storage;
            float smem_code[256];

            for (int i = tid; i < 256; i += BLOCK_DIM_X)
            {
            smem_code[i] = code[i];
            }

            for (int i = base_idx; i < n; i += n_full)
            {
            int valid_items = min(n - i, BLOCK_SIZE);

            LoadFloat(loadf_storage).Load(&(A[i]), vals, valid_items);

            #pragma unroll
            for(int j = 0; j < NUM; j++)
            qvals[j] = dQuantize<0>(smem_code, 0.0f, vals[j]);

            StoreChar(storec_storage).Store(&(out[i]), qvals, valid_items);
            }

        }
    }
}
