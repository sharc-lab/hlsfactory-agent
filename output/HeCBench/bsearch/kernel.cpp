#include "kernel.h"

// --- from main.cu ---
extern "C"
void kernel_BS (const T*  acc_a,
           const T*  acc_z,
            size_t*  acc_r,
           const size_t zSize,
           const size_t n)
{
    #pragma HLS INTERFACE m_axi port=acc_a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=acc_z offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=acc_r offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=zSize
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t i = _bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= zSize) return;
            T z = acc_z[i];
            size_t low = 0;
            size_t high = n;
            while (high - low > 1) {
            size_t mid = low + (high - low)/2;
            if (z < acc_a[mid])
            high = mid;
            else
            low = mid;
            }
            acc_r[i] = low;

        }
    }
}
extern "C"

void kernel_BS2 (const T*  acc_a,
            const T*  acc_z,
             size_t*  acc_r,
            const size_t zSize,
            const size_t n)
{
    #pragma HLS INTERFACE m_axi port=acc_a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=acc_z offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=acc_r offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=zSize
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t i = _bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= zSize) return;
            unsigned  nbits = 0;
            while (n >> nbits) nbits++;
            size_t k = 1ULL << (nbits - 1);
            T z = acc_z[i];
            size_t idx = (acc_a[k] <= z) ? k : 0;
            while (k >>= 1) {
            size_t r = idx | k;
            if (r < n && z >= acc_a[r]) {
            idx = r;
            }
            }
            acc_r[i] = idx;

        }
    }
}
extern "C"

void kernel_BS3 (const T*  acc_a,
            const T*  acc_z,
             size_t*  acc_r,
           const size_t zSize,
            const size_t n)
{
    #pragma HLS INTERFACE m_axi port=acc_a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=acc_z offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=acc_r offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=zSize
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t i = _bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= zSize) return;
            unsigned nbits = 0;
            while (n >> nbits) nbits++;
            size_t k = 1ULL << (nbits - 1);
            T z = acc_z[i];
            size_t idx = (acc_a[k] <= z) ? k : 0;
            while (k >>= 1) {
            size_t r = idx | k;
            size_t w = r < n ? r : n;
            if (z >= acc_a[w]) {
            idx = r;
            }
            }
            acc_r[i] = idx;

        }
    }
}
extern "C"

void kernel_BS4 (const T*  acc_a,
            const T*  acc_z,
             size_t*  acc_r,
            const size_t zSize,
            const size_t n)
{
    #pragma HLS INTERFACE m_axi port=acc_a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=acc_z offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=acc_r offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=zSize
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            size_t k;

            size_t i = _bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= zSize) return;
            size_t lid = _tid_x;

            if (lid == 0) {
            unsigned nbits = 0;
            while (n >> nbits) nbits++;
            k = 1ULL << (nbits - 1);
            }

            size_t p = k;
            T z = acc_z[i];
            size_t idx = (acc_a[p] <= z) ? p : 0;
            while (p >>= 1) {
            size_t r = idx | p;
            size_t w = r < n ? r : n;
            if (z >= acc_a[w]) {
            idx = r;
            }
            }
            acc_r[i] = idx;

        }
    }
}
