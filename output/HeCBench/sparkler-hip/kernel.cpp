#include "kernel.h"

// --- from main.cu ---
  static float zero() {return (float)0;}

  static float one() {return (float)1;}

  static float two() {return (float)2;}

    size_t nr() const {return num_row_;}

    size_t nc() const {return num_col_;}

    P& eltd(size_t i, size_t j) {
      return d_[i + num_row_up_ * j];
    }

    static P& eltd(size_t i, size_t j, P* d, size_t num_row_up) {
      return d[i + num_row_up * j];
    }

size_t nonzero_stride(const size_t& i) {
  enum {MAX = 499}; // Use prime number to randomize against sizes.
  return 1 + i % MAX;
}
extern "C"

void set_input_matrix_kernel(
  size_t nr, size_t nc, size_t nru, typename Matrix_t::P* d,
  size_t base_vector_num, typename Matrix_t::P value) {
    #pragma HLS INTERFACE s_axilite port=nr
    #pragma HLS INTERFACE s_axilite port=nc
    #pragma HLS INTERFACE s_axilite port=nru
    #pragma HLS INTERFACE m_axi port=d offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=base_vector_num
    #pragma HLS INTERFACE s_axilite port=value
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            const size_t index = _tid_x + BLOCK_DIM_X * _bid_x;
            if (index >= nr * nc)
            return;

            const size_t r = index % nr;
            const size_t c = index / nr;

            typedef typename Matrix_t::P P;
            const P zero = TCBufTypes<P>::zero();

            const size_t stride = nonzero_stride(r + base_vector_num);

            Matrix_t::eltd(r, c, d, nru) = c % stride ? zero : value;

        }
    }
}
