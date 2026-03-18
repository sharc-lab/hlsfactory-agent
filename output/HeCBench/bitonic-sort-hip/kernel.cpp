#include "kernel.h"

// --- from main.cu ---
extern "C"
void bitonic_sort (const int seq_len, const int two_power, int *a)
{
    #pragma HLS INTERFACE s_axilite port=seq_len
    #pragma HLS INTERFACE s_axilite port=two_power
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = BLOCK_DIM_X * _bid_x + _tid_x;

            // Assign the bitonic sequence number.
            int seq_num = i / seq_len;

            // Variable used to identified the swapped element.
            int swapped_ele = -1;

            // Because the elements in the first half in the bitonic
            // sequence may swap with elements in the second half,
            // only the first half of elements in each sequence is
            // required (seq_len/2).
            int h_len = seq_len / 2;

            if (i < (seq_len * seq_num) + h_len) swapped_ele = i + h_len;

            // Check whether increasing or decreasing order.
            int odd = seq_num / two_power;

            // Boolean variable used to determine "increasing" or
            // "decreasing" order.
            bool increasing = ((odd % 2) == 0);

            // Swap the elements in the bitonic sequence if needed
            if (swapped_ele != -1) {
            if (((a[i] > a[swapped_ele]) && increasing) ||
            ((a[i] < a[swapped_ele]) && !increasing)) {
            int temp = a[i];
            a[i] = a[swapped_ele];
            a[swapped_ele] = temp;
            }
            }

        }
    }
}
