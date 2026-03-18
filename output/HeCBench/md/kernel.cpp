#include "kernel.h"

// --- from main.cu ---
extern "C"
void md (
  const POSVECTYPE*  position,
        FORCEVECTYPE*  force,
  const int*  neighborList, 
  const int nAtom,
  const int maxNeighbors, 
  const FPTYPE lj1_t,
  const FPTYPE lj2_t,
  const FPTYPE cutsq_t )
{
    #pragma HLS INTERFACE m_axi port=position offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=force offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=neighborList offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=nAtom
    #pragma HLS INTERFACE s_axilite port=maxNeighbors
    #pragma HLS INTERFACE s_axilite port=lj1_t
    #pragma HLS INTERFACE s_axilite port=lj2_t
    #pragma HLS INTERFACE s_axilite port=cutsq_t
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const uint idx = _bid_x * BLOCK_DIM_X + _tid_x;
            if (idx >= nAtom) return;

            POSVECTYPE ipos = position[idx];
            FORCEVECTYPE f = zero;

            int j = 0;
            while (j < maxNeighbors)
            {
            int jidx = neighborList[j*nAtom + idx];

            // Uncoalesced read
            POSVECTYPE jpos = position[jidx];

            // Calculate distance
            FPTYPE delx = ipos.x - jpos.x;
            FPTYPE dely = ipos.y - jpos.y;
            FPTYPE delz = ipos.z - jpos.z;
            FPTYPE r2inv = delx*delx + dely*dely + delz*delz;

            // If distance is less than cutoff, calculate force
            if (r2inv > 0 && r2inv < cutsq_t)
            {
            r2inv = (FPTYPE)1.0 / r2inv;
            FPTYPE r6inv = r2inv * r2inv * r2inv;
            FPTYPE forceC = r2inv * r6inv * (lj1_t * r6inv - lj2_t);

            f.x += delx * forceC;
            f.y += dely * forceC;
            f.z += delz * forceC;
            }
            j++;
            }
            force[idx] = f;

        }
    }
}
