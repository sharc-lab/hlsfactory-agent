#include "kernel.h"

// --- from main.cu ---
extern "C"
void scan_bcao (
  const int64_t nblocks,
        T * g_odata,
  const T * g_idata)
{
    #pragma HLS INTERFACE s_axilite port=nblocks
    #pragma HLS INTERFACE m_axi port=g_odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=temp complete dim=1
    #pragma HLS ARRAY_PARTITION variable=temp complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            T temp[2*N];

            for (int64_t bid = _bid_x; bid < nblocks; bid += GRID_DIM_X)
            {
            auto gi = g_idata + bid * N;
            auto go = g_odata + bid * N;

            int thid = _tid_x;
            int a = thid;
            int b = a + (N/2);
            int oa = OFFSET(a);
            int ob = OFFSET(b);

            temp[a + oa] = gi[a];
            temp[b + ob] = gi[b];

            int offset = 1;
            for (int d = N >> 1; d > 0; d >>= 1)
            {
            if (thid < d)
            {
            int ai = offset*(2*thid+1)-1;
            int bi = offset*(2*thid+2)-1;
            ai += OFFSET(ai);
            bi += OFFSET(bi);
            temp[bi] += temp[ai];
            }
            offset *= 2;
            }

            if (thid == 0) temp[N-1+OFFSET(N-1)] = 0; // clear the last elem
            for (int d = 1; d < N; d *= 2) // traverse down
            {
            offset >>= 1;
            if (thid < d)
            {
            int ai = offset*(2*thid+1)-1;
            int bi = offset*(2*thid+2)-1;
            ai += OFFSET(ai);
            bi += OFFSET(bi);
            T t = temp[ai];
            temp[ai] = temp[bi];
            temp[bi] += t;
            }
            }

            go[a] = temp[a + oa];
            go[b] = temp[b + ob];
            }

        }
    }
}
extern "C"

void scan(
  const int64_t nblocks,
        T * g_odata,
  const T * g_idata)
{
    #pragma HLS INTERFACE s_axilite port=nblocks
    #pragma HLS INTERFACE m_axi port=g_odata offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_idata offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=temp complete dim=1
    #pragma HLS ARRAY_PARTITION variable=temp complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            T temp[N];

            for (int64_t bid = _bid_x; bid < nblocks; bid += GRID_DIM_X)
            {
            auto gi = g_idata + bid * N;
            auto go = g_odata + bid * N;

            int thid = _tid_x;
            int offset = 1;
            temp[2*thid]   = gi[2*thid];
            temp[2*thid+1] = gi[2*thid+1];
            for (int d = N >> 1; d > 0; d >>= 1)
            {
            if (thid < d)
            {
            // e.g.
            // thread 0: ai = 0, bi = 1 (offset = 1) d = 4
            // thread 1: ai = 2, bi = 3 (offset = 1) d = 4
            // thread 2: ai = 4, bi = 5 (offset = 1) d = 4
            // thread 3: ai = 6, bi = 7 (offset = 1) d = 4
            // thread 0: ai = 1, bi = 3 (offset = 2) d = 2
            // thread 1: ai = 5, bi = 7 (offset = 2) d = 2
            // thread 0: ai = 3, bi = 7 (offset = 4) d = 1
            int ai = offset*(2*thid+1)-1;
            int bi = offset*(2*thid+2)-1;
            temp[bi] += temp[ai];
            }
            offset *= 2;
            }

            if (thid == 0) temp[N-1] = 0; // clear the last elem
            for (int d = 1; d < N; d *= 2) // traverse down
            {
            offset >>= 1;
            if (thid < d)
            {
            int ai = offset*(2*thid+1)-1;
            int bi = offset*(2*thid+2)-1;
            T t = temp[ai];
            temp[ai] = temp[bi];
            temp[bi] += t;
            }
            }
            go[2*thid] = temp[2*thid];
            go[2*thid+1] = temp[2*thid+1];
            }

        }
    }
}
