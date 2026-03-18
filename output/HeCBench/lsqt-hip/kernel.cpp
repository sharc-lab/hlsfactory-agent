#include "kernel.h"

// --- from hamiltonian.cu ---
extern "C"
void gpu_apply_hamiltonian(
  const int number_of_atoms,
  const real energy_max,
  const  int*  g_neighbor_number,
  const  int*  g_neighbor_list,
  const real*  g_potential,
  const real*  g_hopping_real,
  const real*  g_hopping_imag,
  const real*  g_state_in_real,
  const real*  g_state_in_imag,
        real*  g_state_out_real,
        real*  g_state_out_imag)
{
    #pragma HLS INTERFACE s_axilite port=number_of_atoms
    #pragma HLS INTERFACE s_axilite port=energy_max
    #pragma HLS INTERFACE m_axi port=g_neighbor_number offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_neighbor_list offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_potential offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_hopping_real offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=g_hopping_imag offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=g_state_in_real offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=g_state_in_imag offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=g_state_out_real offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=g_state_out_imag offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int n = _bid_x * BLOCK_DIM_X + _tid_x;
            if (n < number_of_atoms) {
            real temp_real = g_potential[n] * g_state_in_real[n]; // on-site
            real temp_imag = g_potential[n] * g_state_in_imag[n]; // on-site

            for (int m = 0; m < g_neighbor_number[n]; ++m) {
            int index_1 = m * number_of_atoms + n;
            int index_2 = g_neighbor_list[index_1];
            real a = g_hopping_real[index_1];
            real b = g_hopping_imag[index_1];
            real c = g_state_in_real[index_2];
            real d = g_state_in_imag[index_2];
            temp_real += a * c - b * d; // hopping
            temp_imag += a * d + b * c; // hopping
            }
            temp_real /= energy_max; // scale
            temp_imag /= energy_max; // scale
            g_state_out_real[n] = temp_real;
            g_state_out_imag[n] = temp_imag;
            }

        }
    }
}
extern "C"

void gpu_apply_commutator(
  int number_of_atoms,
  real energy_max,
  int* g_neighbor_number,
  int* g_neighbor_list,
  real* g_hopping_real,
  real* g_hopping_imag,
  real* g_xx,
  real* g_state_in_real,
  real* g_state_in_imag,
  real* g_state_out_real,
  real* g_state_out_imag)
{
    #pragma HLS INTERFACE s_axilite port=number_of_atoms
    #pragma HLS INTERFACE s_axilite port=energy_max
    #pragma HLS INTERFACE m_axi port=g_neighbor_number offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_neighbor_list offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_hopping_real offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_hopping_imag offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=g_xx offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=g_state_in_real offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=g_state_in_imag offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=g_state_out_real offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=g_state_out_imag offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int n = _bid_x * BLOCK_DIM_X + _tid_x;
            if (n < number_of_atoms) {
            real temp_real = 0.0;
            real temp_imag = 0.0;
            for (int m = 0; m < g_neighbor_number[n]; ++m) {
            int index_1 = m * number_of_atoms + n;
            int index_2 = g_neighbor_list[index_1];
            real a = g_hopping_real[index_1];
            real b = g_hopping_imag[index_1];
            real c = g_state_in_real[index_2];
            real d = g_state_in_imag[index_2];
            real xx = g_xx[index_1];
            temp_real -= (a * c - b * d) * xx;
            temp_imag -= (a * d + b * c) * xx;
            }
            g_state_out_real[n] = temp_real / energy_max; // scale
            g_state_out_imag[n] = temp_imag / energy_max; // scale
            }

        }
    }
}
extern "C"

void gpu_apply_current(
  const int number_of_atoms,
  const  int*  g_neighbor_number,
  const  int*  g_neighbor_list,
  const real*  g_hopping_real,
  const real*  g_hopping_imag,
  const real*  g_xx,
  const real*  g_state_in_real,
  const real*  g_state_in_imag,
        real*  g_state_out_real,
        real*  g_state_out_imag)
{
    #pragma HLS INTERFACE s_axilite port=number_of_atoms
    #pragma HLS INTERFACE m_axi port=g_neighbor_number offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_neighbor_list offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_hopping_real offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_hopping_imag offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=g_xx offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=g_state_in_real offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=g_state_in_imag offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=g_state_out_real offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=g_state_out_imag offset=slave bundle=gmem8
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int n = _bid_x * BLOCK_DIM_X + _tid_x;
            if (n < number_of_atoms) {
            real temp_real = 0.0;
            real temp_imag = 0.0;
            for (int m = 0; m < g_neighbor_number[n]; ++m) {
            int index_1 = m * number_of_atoms + n;
            int index_2 = g_neighbor_list[index_1];
            real a = g_hopping_real[index_1];
            real b = g_hopping_imag[index_1];
            real c = g_state_in_real[index_2];
            real d = g_state_in_imag[index_2];
            temp_real += (a * c - b * d) * g_xx[index_1];
            temp_imag += (a * d + b * c) * g_xx[index_1];
            }
            g_state_out_real[n] = +temp_imag;
            g_state_out_imag[n] = -temp_real;
            }

        }
    }
}
extern "C"

void gpu_chebyshev_01(
  const int number_of_atoms,
  const real*  g_state_0_real,
  const real*  g_state_0_imag,
  const real*  g_state_1_real,
  const real*  g_state_1_imag,
        real*  g_state_real,
        real*  g_state_imag,
  const real b0,
  const real b1,
  const int direction)
{
    #pragma HLS INTERFACE s_axilite port=number_of_atoms
    #pragma HLS INTERFACE m_axi port=g_state_0_real offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_state_0_imag offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_state_1_real offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_state_1_imag offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=g_state_real offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=g_state_imag offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=b0
    #pragma HLS INTERFACE s_axilite port=b1
    #pragma HLS INTERFACE s_axilite port=direction
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int n = _bid_x * BLOCK_DIM_X + _tid_x;
            if (n < number_of_atoms) {
            real bessel_0 = b0;
            real bessel_1 = b1 * direction;
            g_state_real[n] = bessel_0 * g_state_0_real[n] + bessel_1 * g_state_1_imag[n];
            g_state_imag[n] = bessel_0 * g_state_0_imag[n] - bessel_1 * g_state_1_real[n];
            }

        }
    }
}
extern "C"

void gpu_chebyshev_2(
  const int number_of_atoms,
  const real energy_max,
  const  int*  g_neighbor_number,
  const  int*  g_neighbor_list,
  const real*  g_potential,
  const real*  g_hopping_real,
  const real*  g_hopping_imag,
  const real*  g_state_0_real,
  const real*  g_state_0_imag,
  const real*  g_state_1_real,
  const real*  g_state_1_imag,
        real*  g_state_2_real,
        real*  g_state_2_imag,
        real*  g_state_real,
        real*  g_state_imag,
  const real bessel_m,
  const int label)
{
    #pragma HLS INTERFACE s_axilite port=number_of_atoms
    #pragma HLS INTERFACE s_axilite port=energy_max
    #pragma HLS INTERFACE m_axi port=g_neighbor_number offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_neighbor_list offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_potential offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_hopping_real offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=g_hopping_imag offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=g_state_0_real offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=g_state_0_imag offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=g_state_1_real offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=g_state_1_imag offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=g_state_2_real offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=g_state_2_imag offset=slave bundle=gmem10
    #pragma HLS INTERFACE m_axi port=g_state_real offset=slave bundle=gmem11
    #pragma HLS INTERFACE m_axi port=g_state_imag offset=slave bundle=gmem12
    #pragma HLS INTERFACE s_axilite port=bessel_m
    #pragma HLS INTERFACE s_axilite port=label
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int n = _bid_x * BLOCK_DIM_X + _tid_x;
            if (n < number_of_atoms) {
            real temp_real = g_potential[n] * g_state_1_real[n]; // on-site
            real temp_imag = g_potential[n] * g_state_1_imag[n]; // on-site

            for (int m = 0; m < g_neighbor_number[n]; ++m) {
            int index_1 = m * number_of_atoms + n;
            int index_2 = g_neighbor_list[index_1];
            real a = g_hopping_real[index_1];
            real b = g_hopping_imag[index_1];
            real c = g_state_1_real[index_2];
            real d = g_state_1_imag[index_2];
            temp_real += a * c - b * d; // hopping
            temp_imag += a * d + b * c; // hopping
            }
            temp_real /= energy_max; // scale
            temp_imag /= energy_max; // scale

            temp_real = 2.0 * temp_real - g_state_0_real[n];
            temp_imag = 2.0 * temp_imag - g_state_0_imag[n];
            switch (label) {
            case 1: {
            g_state_real[n] += bessel_m * temp_real;
            g_state_imag[n] += bessel_m * temp_imag;
            break;
            }
            case 2: {
            g_state_real[n] -= bessel_m * temp_real;
            g_state_imag[n] -= bessel_m * temp_imag;
            break;
            }
            case 3: {
            g_state_real[n] += bessel_m * temp_imag;
            g_state_imag[n] -= bessel_m * temp_real;
            break;
            }
            case 4: {
            g_state_real[n] -= bessel_m * temp_imag;
            g_state_imag[n] += bessel_m * temp_real;
            break;
            }
            }
            g_state_2_real[n] = temp_real;
            g_state_2_imag[n] = temp_imag;
            }

        }
    }
}
extern "C"

void gpu_chebyshev_1x(
  const int number_of_atoms,
  const real*  g_state_1x_real,
  const real*  g_state_1x_imag,
        real*  g_state_real,
        real*  g_state_imag,
  const real g_bessel_1)
{
    #pragma HLS INTERFACE s_axilite port=number_of_atoms
    #pragma HLS INTERFACE m_axi port=g_state_1x_real offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_state_1x_imag offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_state_real offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_state_imag offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=g_bessel_1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int n = _bid_x * BLOCK_DIM_X + _tid_x;
            if (n < number_of_atoms) {
            real b1 = g_bessel_1;
            g_state_real[n] = +b1 * g_state_1x_imag[n];
            g_state_imag[n] = -b1 * g_state_1x_real[n];
            }

        }
    }
}
extern "C"

void gpu_kernel_polynomial(
  const int number_of_atoms,
  const real energy_max,
  const  int*  g_neighbor_number,
  const  int*  g_neighbor_list,
  const real*  g_potential,
  const real*  g_hopping_real,
  const real*  g_hopping_imag,
  const real*  g_state_0_real,
  const real*  g_state_0_imag,
  const real*  g_state_1_real,
  const real*  g_state_1_imag,
        real*  g_state_2_real,
        real*  g_state_2_imag)
{
    #pragma HLS INTERFACE s_axilite port=number_of_atoms
    #pragma HLS INTERFACE s_axilite port=energy_max
    #pragma HLS INTERFACE m_axi port=g_neighbor_number offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_neighbor_list offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_potential offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_hopping_real offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=g_hopping_imag offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=g_state_0_real offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=g_state_0_imag offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=g_state_1_real offset=slave bundle=gmem7
    #pragma HLS INTERFACE m_axi port=g_state_1_imag offset=slave bundle=gmem8
    #pragma HLS INTERFACE m_axi port=g_state_2_real offset=slave bundle=gmem9
    #pragma HLS INTERFACE m_axi port=g_state_2_imag offset=slave bundle=gmem10
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int n = _bid_x * BLOCK_DIM_X + _tid_x;
            if (n < number_of_atoms) {
            real temp_real = g_potential[n] * g_state_1_real[n]; // on-site
            real temp_imag = g_potential[n] * g_state_1_imag[n]; // on-site

            for (int m = 0; m < g_neighbor_number[n]; ++m) {
            int index_1 = m * number_of_atoms + n;
            int index_2 = g_neighbor_list[index_1];
            real a = g_hopping_real[index_1];
            real b = g_hopping_imag[index_1];
            real c = g_state_1_real[index_2];
            real d = g_state_1_imag[index_2];
            temp_real += a * c - b * d; // hopping
            temp_imag += a * d + b * c; // hopping
            }

            temp_real /= energy_max; // scale
            temp_imag /= energy_max; // scale

            temp_real = 2.0 * temp_real - g_state_0_real[n];
            temp_imag = 2.0 * temp_imag - g_state_0_imag[n];
            g_state_2_real[n] = temp_real;
            g_state_2_imag[n] = temp_imag;
            }

        }
    }
}


// --- from vector.cu ---
extern "C"
void gpu_set_zero(int number_of_elements, 
  real*  g_state_real, 
  real*  g_state_imag)
{
    #pragma HLS INTERFACE s_axilite port=number_of_elements
    #pragma HLS INTERFACE m_axi port=g_state_real offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_state_imag offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int n = _bid_x * BLOCK_DIM_X + _tid_x;
            if (n < number_of_elements) {
            g_state_real[n] = 0;
            g_state_imag[n] = 0;
            }

        }
    }
}
extern "C"

void gpu_add_state(
  const int n, 
  const real* in_real,
  const real* in_imag, 
        real* out_real, 
        real* out_imag)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=in_real offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=in_imag offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=out_real offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=out_imag offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x * BLOCK_DIM_X + _tid_x;
            if (i < n) {
            out_real[i] += in_real[i];
            out_imag[i] += in_imag[i];
            }

        }
    }
}

void warp_reduce(volatile real* s, int t)
{
  s[t] += s[t + 32];
  s[t] += s[t + 16];
  s[t] += s[t + 8];
  s[t] += s[t + 4];
  s[t] += s[t + 2];
  s[t] += s[t + 1];
}
extern "C"

void gpu_find_inner_product_1(
  const int number_of_atoms,
  const real*  g_final_state_real,
  const real*  g_final_state_imag,
  const real*  g_random_state_real,
  const real*  g_random_state_imag,
        real*  g_inner_product_real,
        real*  g_inner_product_imag,
  const int g_offset)
{
    #pragma HLS INTERFACE s_axilite port=number_of_atoms
    #pragma HLS INTERFACE m_axi port=g_final_state_real offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_final_state_imag offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_random_state_real offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_random_state_imag offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=g_inner_product_real offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=g_inner_product_imag offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=g_offset
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_data_real complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_data_imag complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_data_real complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_data_imag complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = _tid_x;
            int n = _bid_x * BLOCK_DIM_X + tid;
            int m;
            real a, b, c, d;
            real s_data_real[BLOCK_SIZE];
            real s_data_imag[BLOCK_SIZE];
            s_data_real[tid] = 0.0;
            s_data_imag[tid] = 0.0;

            if (n < number_of_atoms) {
            a = g_final_state_real[n];
            b = g_final_state_imag[n];
            c = g_random_state_real[n];
            d = g_random_state_imag[n];
            s_data_real[tid] = (a * c + b * d);
            s_data_imag[tid] = (b * c - a * d);
            }

            /*
            if (tid < 256) {
            m = tid + 256;
            s_data_real[tid] += s_data_real[m];
            s_data_imag[tid] += s_data_imag[m];
            }
            */

            if (tid < 128) {
            m = tid + 128;
            s_data_real[tid] += s_data_real[m];
            s_data_imag[tid] += s_data_imag[m];
            }
            if (tid < 64) {
            m = tid + 64;
            s_data_real[tid] += s_data_real[m];
            s_data_imag[tid] += s_data_imag[m];
            }
            if (tid < 32) {
            warp_reduce(s_data_real, tid);
            warp_reduce(s_data_imag, tid);
            }
            if (tid == 0) {
            g_inner_product_real[_bid_x + g_offset] = s_data_real[0];
            g_inner_product_imag[_bid_x + g_offset] = s_data_imag[0];
            }

        }
    }
}
extern "C"

void gpu_find_inner_product_2(
  const int number_of_atoms,
  const real*  g_inner_product_1_real,
  const real*  g_inner_product_1_imag,
        real*  g_inner_product_2_real,
        real*  g_inner_product_2_imag)
{
    #pragma HLS INTERFACE s_axilite port=number_of_atoms
    #pragma HLS INTERFACE m_axi port=g_inner_product_1_real offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=g_inner_product_1_imag offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=g_inner_product_2_real offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=g_inner_product_2_imag offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=s_data_real complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_data_imag complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_data_real complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s_data_imag complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            int tid = _tid_x;
            int patch, n, m;

            real s_data_real[BLOCK_SIZE];
            real s_data_imag[BLOCK_SIZE];
            s_data_real[tid] = 0.0;
            s_data_imag[tid] = 0.0;
            int number_of_blocks = (number_of_atoms - 1) / BLOCK_SIZE + 1;
            int number_of_patches = (number_of_blocks - 1) / BLOCK_SIZE + 1;

            for (patch = 0; patch < number_of_patches; ++patch) {
            n = tid + patch * BLOCK_SIZE;
            if (n < number_of_blocks) {
            m = _bid_x * number_of_blocks + n;
            s_data_real[tid] += g_inner_product_1_real[m];
            s_data_imag[tid] += g_inner_product_1_imag[m];
            }
            }

            /*
            if (tid < 256) {
            m = tid + 256;
            s_data_real[tid] += s_data_real[m];
            s_data_imag[tid] += s_data_imag[m];
            }
            */

            if (tid < 128) {
            m = tid + 128;
            s_data_real[tid] += s_data_real[m];
            s_data_imag[tid] += s_data_imag[m];
            }
            if (tid < 64) {
            m = tid + 64;
            s_data_real[tid] += s_data_real[m];
            s_data_imag[tid] += s_data_imag[m];
            }
            if (tid < 32) {
            warp_reduce(s_data_real, tid);
            warp_reduce(s_data_imag, tid);
            }
            if (tid == 0) {
            g_inner_product_2_real[_bid_x] = s_data_real[0];
            g_inner_product_2_imag[_bid_x] = s_data_imag[0];
            }

        }
    }
}
