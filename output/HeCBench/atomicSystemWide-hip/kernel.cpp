#include "kernel.h"

// --- from main.cu ---
extern "C"
void atomicKernel(int *atom_arr, const int loop_num)
{
    #pragma HLS INTERFACE m_axi port=atom_arr offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=loop_num
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = BLOCK_DIM_X * _bid_x + _tid_x;

            for (int i=0; i < loop_num; i++)
            {
            // Atomic addition
            atomicAdd_system(&atom_arr[0], 10);

            // Atomic exchange
            atomicExch_system(&atom_arr[1], tid);

            // Atomic maximum
            atomicMax_system(&atom_arr[2], tid);

            // Atomic minimum
            atomicMin_system(&atom_arr[3], tid);

            // Atomic increment (modulo 17+1)
            //atomicInc_system((unsigned int *)&atom_arr[4], 17);

            // Atomic decrement
            //atomicDec_system((unsigned int *)&atom_arr[5], 137);

            // Atomic compare-and-swap
            atomicCAS_system(&atom_arr[6], tid-1, tid);

            // Bitwise atomic instructions

            // Atomic AND
            atomicAnd_system(&atom_arr[7], 2*tid+7);

            // Atomic OR
            atomicOr_system(&atom_arr[8], 1 << tid);

            // Atomic XOR
            atomicXor_system(&atom_arr[9], tid);
            }

        }
    }
}
