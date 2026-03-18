#include "kernel.h"

// --- from main.cu ---
extern "C"
static void init1(
  const int nodes,
  mtype* const AdjMat,
  const int upper)
{
    #pragma HLS INTERFACE s_axilite port=nodes
    #pragma HLS INTERFACE m_axi port=AdjMat offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=upper
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int idx = _tid_x + _bid_x * BLOCK_DIM_X;
            const int i = idx / upper;
            if (i < upper) {
            const int j = idx % upper;
            AdjMat[idx] = ((i == j) && (i < nodes)) ? 0 : (INT_MAX / 2);
            }

        }
    }
}
extern "C"

static void init2(
  const ECLgraph g,
  mtype* const AdjMat,
  const int upper)
{
    #pragma HLS INTERFACE s_axilite port=g
    #pragma HLS INTERFACE m_axi port=AdjMat offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=upper
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = _tid_x + _bid_x * BLOCK_DIM_X;
            if (i < g.nodes) {
            for (int j = g.nindex[i]; j < g.nindex[i + 1]; j++) {
            const int nei = g.nlist[j];
            AdjMat[i * upper + nei] = g.eweight[j];
            }
            }

        }
    }
}
