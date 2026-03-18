#include "kernel.h"

// --- from main.cu ---
extern "C"
void reference (const float *  A,
                           unsigned char *out, const size_t n)
{
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            for (size_t idx = (size_t)_bid_x * BLOCK_DIM_X + _tid_x;
            idx < n/4; idx += GRID_DIM_X * BLOCK_DIM_X) {
            const float4 v = reinterpret_cast<const float4*>(A)[idx];
            uchar4 o;
            o.x = (int)v.x;
            o.y = (int)v.y;
            o.z = (int)v.z;
            o.w = (int)v.w;
            reinterpret_cast<uchar4*>(out)[idx] = o;
            }

        }
    }
}
extern "C"

void kernel (const float *  A,
                        unsigned char *out, const size_t n)
{
    #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            float vals[ITEMS_PER_THREAD];
            unsigned char qvals[ITEMS_PER_THREAD];

            typedef BlockLoad<float, BLOCKSIZE, ITEMS_PER_THREAD> LoadFloat;
            typedef BlockStore<unsigned char, BLOCKSIZE, ITEMS_PER_THREAD> StoreChar;

            typename LoadFloat::TempStorage loadf_storage;
            typename StoreChar::TempStorage storec_storage;

            for (size_t i = (size_t)_bid_x * BLOCKSIZE * ITEMS_PER_THREAD;
            i < n; i += GRID_DIM_X * BLOCKSIZE * ITEMS_PER_THREAD)
            {
            int valid_items = min(n - i, (size_t)BLOCKSIZE * ITEMS_PER_THREAD);

            // Parameters:
            // block_src_it – [in] The thread block's base iterator for loading from
            // dst_items – [out] Destination to load data into
            // block_items_end – [in] Number of valid items to load
            LoadFloat(loadf_storage).Load(&(A[i]), vals, valid_items);

            #pragma unroll
            for(int j = 0; j < ITEMS_PER_THREAD; j++)
            qvals[j] = int(vals[j]);

            StoreChar(storec_storage).Store(&(out[i]), qvals, valid_items);
            }

        }
    }
}
