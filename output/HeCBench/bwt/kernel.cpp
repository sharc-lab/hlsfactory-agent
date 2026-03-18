#include "kernel.h"

// --- from bwt.cu ---
extern "C"
void generate_table(int* table, int table_size, int n) {
    #pragma HLS INTERFACE m_axi port=table offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=table_size
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            int stride = BLOCK_DIM_X * GRID_DIM_X;
            for(int i = index; i < table_size; i+=stride)
            table[i] = (i < n) ? i : -1;

        }
    }
}

bool compare_rotations(const int& a, const int& b, const char* genome, int n) {
  if (a < 0) return false;
  if (b < 0) return true;
  for(int i = 0; i < n; i++) {
    if (genome[(a + i) % n] != genome[(b + i) % n]) {
      return genome[(a + i) % n] < genome[(b + i) % n];
    }
  }
  return false;
}
extern "C"

void bitonic_sort_step(int*__restrict table, int table_size, 
                                  int j, int k, const char*__restrict genome, int n) {
    #pragma HLS INTERFACE m_axi port=table offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=table_size
    #pragma HLS INTERFACE s_axilite port=j
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE m_axi port=genome offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int i = _tid_x + BLOCK_DIM_X * _bid_x;
            unsigned int ixj = i ^ j;
            if (i < table_size && ixj > i) {
            bool f = (i & k) == 0;
            int t1 = table[i];
            int t2 = table[ixj];
            if (compare_rotations(f ? t2 : t1, f ? t1 : t2, genome, n)) {
            table[i] = t2;
            table[ixj] = t1;
            }
            }

        }
    }
}
extern "C"

void reconstruct_sequence(const int*__restrict table, const char*__restrict sequence, 
                                     char*__restrict transformed_sequence, int n) {
    #pragma HLS INTERFACE m_axi port=table offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=sequence offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=transformed_sequence offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int index = _bid_x * BLOCK_DIM_X + _tid_x;
            int stride = BLOCK_DIM_X * GRID_DIM_X;
            for(int i = index; i < n; i += stride) {
            transformed_sequence[i] = sequence[(n + table[i] - 1) % n];
            }

        }
    }
}
