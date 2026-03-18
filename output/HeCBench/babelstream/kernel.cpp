#include "kernel.h"

// --- from main.cu ---
extern "C"
void init_kernel(
  T * a,
  T * b,
  T * c,
  T initA, T initB, T initC)
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=initA
    #pragma HLS INTERFACE s_axilite port=initB
    #pragma HLS INTERFACE s_axilite port=initC
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = BLOCK_DIM_X * _bid_x + _tid_x;
            a[i] = initA;
            b[i] = initB;
            c[i] = initC;

        }
    }
}
extern "C"

void copy_kernel(
  const T * a,
        T * c)
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = BLOCK_DIM_X * _bid_x + _tid_x;
            c[i] = a[i];

        }
    }
}
extern "C"

void mul_kernel(
        T * b,
  const T * c)
{
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const T scalar = SCALAR;
            const int i = BLOCK_DIM_X * _bid_x + _tid_x;
            b[i] = scalar * c[i];

        }
    }
}
extern "C"

void add_kernel(
  const T * a,
  const T * b,
        T * c)
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int i = BLOCK_DIM_X * _bid_x + _tid_x;
            c[i] = a[i] + b[i];

        }
    }
}
extern "C"

void triad_kernel(
        T * a,
  const T * b,
  const T * c)
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const T scalar = SCALAR;
            const int i = BLOCK_DIM_X * _bid_x + _tid_x;
            a[i] = b[i] + scalar * c[i];

        }
    }
}
extern "C"

void nstream_kernel(
        T * a,
  const T * b,
  const T * c)
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=c offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const T scalar = SCALAR;
            const int i = BLOCK_DIM_X * _bid_x + _tid_x;
            a[i] += b[i] + scalar * c[i];

        }
    }
}
extern "C"

void dot_kernel(
  const T * a,
  const T * b,
        T * sum,
  int array_size)
{
    #pragma HLS INTERFACE m_axi port=a offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=b offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=sum offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=array_size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=tb_sum complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            T tb_sum[TBSIZE];

            const size_t local_i = _tid_x;

            tb_sum[local_i] = 0.0;
            for (int i = BLOCK_DIM_X * _bid_x + _tid_x;
            i < array_size; i += BLOCK_DIM_X*GRID_DIM_X)
            tb_sum[local_i] += a[i] * b[i];

            for (int offset = BLOCK_DIM_X / 2; offset > 0; offset /= 2)
            {
            if (local_i < offset)
            {
            tb_sum[local_i] += tb_sum[local_i+offset];
            }
            }

            if (local_i == 0)
            sum[_bid_x] = tb_sum[local_i];

        }
    }
}
