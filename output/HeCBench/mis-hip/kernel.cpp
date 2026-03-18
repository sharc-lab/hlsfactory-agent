#include "kernel.h"

// --- from main.cu ---
extern "C"
void findmins(const int nodes, 
    const int* const __restrict nidx,
    const int* const __restrict nlist,
    volatile stattype* const __restrict nstat)
{
    #pragma HLS INTERFACE s_axilite port=nodes
    #pragma HLS INTERFACE m_axi port=nidx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=nlist offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=nstat offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int from = _tid_x + _bid_x * ThreadsPerBlock;
            const int incr = GRID_DIM_X * ThreadsPerBlock;

            int missing;
            do {
            missing = 0;
            for (int v = from; v < nodes; v += incr) {
            const stattype nv = nstat[v];
            if (nv & 1) {
            int i = nidx[v];
            while ((i < nidx[v + 1]) && ((nv > nstat[nlist[i]]) || ((nv == nstat[nlist[i]]) && (v > nlist[i])))) {
            i++;
            }
            if (i < nidx[v + 1]) {
            missing = 1;
            } else {
            for (int i = nidx[v]; i < nidx[v + 1]; i++) {
            nstat[nlist[i]] = out;
            }
            nstat[v] = in;
            }
            }
            }
            } while (missing != 0);

        }
    }
}

unsigned int hash(unsigned int val)
{
  val = ((val >> 16) ^ val) * 0x45d9f3b;
  val = ((val >> 16) ^ val) * 0x45d9f3b;
  return (val >> 16) ^ val;
}
extern "C"

void init(const int nodes, 
    const int edges, 
    const int* const __restrict nidx,
    stattype* const __restrict nstat)
{
    #pragma HLS INTERFACE s_axilite port=nodes
    #pragma HLS INTERFACE s_axilite port=edges
    #pragma HLS INTERFACE m_axi port=nidx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=nstat offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int from = _tid_x + _bid_x * ThreadsPerBlock;
            const int incr = GRID_DIM_X * ThreadsPerBlock;

            const float avg = (float)edges / nodes;
            const float scaledavg = ((in / 2) - 1) * avg;

            for (int i = from; i < nodes; i += incr) {
            stattype val = in;
            const int degree = nidx[i + 1] - nidx[i];
            if (degree > 0) {
            float x = degree - (hash(i) * 0.00000000023283064365386962890625f);
            int res = int(scaledavg / (avg + x));
            val = (res + res) | 1;
            }
            nstat[i] = val;
            }

        }
    }
}
