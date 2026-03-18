#include "kernel.h"

// --- from main.cu ---
extern "C"
void cast1_intrinsics(const int n,
                      const double* input,
                            long long int* output)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _tid_x + _bid_x * BLOCK_DIM_X;
            if (i >= n) return;

            int r1 = 0;
            unsigned int r2 = 0;
            long long int r3 = 0;
            unsigned long long int r4 = 0;

            double x = input[i];

            r1 ^= __double2hiint(x);
            r1 ^= __double2loint(x);

            r1 ^= __double2int_rd(x);
            r1 ^= __double2int_rn(x);
            r1 ^= __double2int_ru(x);
            r1 ^= __double2int_rz(x);

            r1 ^= __float2int_rd(x);
            r1 ^= __float2int_rn(x);
            r1 ^= __float2int_ru(x);
            r1 ^= __float2int_rz(x);

            r1 ^= __float_as_int(x);

            r2 ^= __double2uint_rd(x);
            r2 ^= __double2uint_rn(x);
            r2 ^= __double2uint_ru(x);
            r2 ^= __double2uint_rz(x);

            r2 ^= __float2uint_rd(x);
            r2 ^= __float2uint_rn(x);
            r2 ^= __float2uint_ru(x);
            r2 ^= __float2uint_rz(x);

            r2 ^= __float_as_uint(x);

            r3 ^= __double2ll_rd(x);
            r3 ^= __double2ll_rn(x);
            r3 ^= __double2ll_ru(x);
            r3 ^= __double2ll_rz(x);

            r3 ^= __float2ll_rd(x);
            r3 ^= __float2ll_rn(x);
            r3 ^= __float2ll_ru(x);
            r3 ^= __float2ll_rz(x);

            r3 ^= __double_as_longlong(x);

            r4 ^= __double2ull_rd(x);
            r4 ^= __double2ull_rn(x);
            r4 ^= __double2ull_ru(x);
            r4 ^= __double2ull_rz(x);

            r4 ^= __float2ull_rd(x);
            r4 ^= __float2ull_rn(x);
            r4 ^= __float2ull_ru(x);
            r4 ^= __float2ull_rz(x);

            output[i] = (r1 + r2) + (r3 + r4);

        }
    }
}
extern "C"

void cast2_intrinsics(const int n,
                      const long long int* input,
                            long long int* output)
{
    #pragma HLS INTERFACE s_axilite port=n
    #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _tid_x + _bid_x * BLOCK_DIM_X;
            if (i >= n) return;

            float r1 = 0;
            double r2 = 0;

            long long int x = input[i];

            r1 += __hiloint2double(x >> 32, x);

            r1 += __int2float_rd(x);
            r1 += __int2float_rn(x);
            r1 += __int2float_ru(x);
            r1 += __int2float_rz(x);

            r1 += __uint2float_rd(x);
            r1 += __uint2float_rn(x);
            r1 += __uint2float_ru(x);
            r1 += __uint2float_rz(x);

            r1 += __int_as_float(x);
            r1 += __uint_as_float(x);

            r1 += __ll2float_rd(x);
            r1 += __ll2float_rn(x);
            r1 += __ll2float_ru(x);
            r1 += __ll2float_rz(x);

            r1 += __ull2float_rd(x);
            r1 += __ull2float_rn(x);
            r1 += __ull2float_ru(x);
            r1 += __ull2float_rz(x);

            r2 += __int2double_rn(x);
            r2 += __uint2double_rn(x);

            r2 += __ll2double_rd(x);
            r2 += __ll2double_rn(x);
            r2 += __ll2double_ru(x);
            r2 += __ll2double_rz(x);

            r2 += __ull2double_rd(x);
            r2 += __ull2double_rn(x);
            r2 += __ull2double_ru(x);
            r2 += __ull2double_rz(x);

            r2 += __longlong_as_double(x);

            output[i] = __double_as_longlong(r1+r2);

        }
    }
}
