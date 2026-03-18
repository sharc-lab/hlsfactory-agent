#include "kernel.h"

// --- from kernels.cu ---
extern "C"
void gcrs_m_1_w_4_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 4;
            int i,j;
            long result = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result = result ^ ( (((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            ++index;
            }

            }

            out[idx] = result;

        }
    }
}
extern "C"

void gcrs_m_1_w_5_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 5;
            int i,j;
            long result = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result = result ^ ( (((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            ++index;
            }

            }

            out[idx] = result;

        }
    }
}
extern "C"

void gcrs_m_1_w_6_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 6;
            int i,j;
            long result = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result = result ^ ( (((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            ++index;
            }

            }

            out[idx] = result;

        }
    }
}
extern "C"

void gcrs_m_1_w_7_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 7;
            int i,j;
            long result = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result = result ^ ( (((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            ++index;
            }

            }

            out[idx] = result;

        }
    }
}
extern "C"

void gcrs_m_1_w_8_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 8;
            int i,j;
            long result = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result = result ^ ( (((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            ++index;
            }

            }

            out[idx] = result;

        }
    }
}
extern "C"

void gcrs_m_2_w_4_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 4;
            int i,j;
            long result[2];

            result[0] = 0;
            result[1] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];

        }
    }
}
extern "C"

void gcrs_m_2_w_5_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 5;
            int i,j;
            long result[2];

            result[0] = 0;
            result[1] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];

        }
    }
}
extern "C"

void gcrs_m_2_w_6_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 6;
            int i,j;
            long result[2];

            result[0] = 0;
            result[1] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];

        }
    }
}
extern "C"

void gcrs_m_2_w_7_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 7;
            int i,j;
            long result[2];

            result[0] = 0;
            result[1] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];

        }
    }
}
extern "C"

void gcrs_m_2_w_8_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 8;
            int i,j;
            long result[2];

            result[0] = 0;
            result[1] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];

        }
    }
}
extern "C"

void gcrs_m_3_w_4_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 4;

            int i,j;
            long result[3];

            result[0] = 0;
            result[1] = 0;
            result[2] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);
            result[2] = result[2] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 2*w))) >> (group_inner_offset + 2*w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];
            out[idx + 2 * size] = result[2];

        }
    }
}
extern "C"

void gcrs_m_3_w_5_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 5;

            int i,j;
            long result[3];

            result[0] = 0;
            result[1] = 0;
            result[2] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);
            result[2] = result[2] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 2*w))) >> (group_inner_offset + 2*w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];
            out[idx + 2 * size] = result[2];

        }
    }
}
extern "C"

void gcrs_m_3_w_6_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 6;

            int i,j;
            long result[3];

            result[0] = 0;
            result[1] = 0;
            result[2] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);
            result[2] = result[2] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 2*w))) >> (group_inner_offset + 2*w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];
            out[idx + 2 * size] = result[2];

        }
    }
}
extern "C"

void gcrs_m_3_w_7_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 7;

            int i,j;
            long result[3];

            result[0] = 0;
            result[1] = 0;
            result[2] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);
            result[2] = result[2] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 2*w))) >> (group_inner_offset + 2*w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];
            out[idx + 2 * size] = result[2];

        }
    }
}
extern "C"

void gcrs_m_3_w_8_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 8;

            int i,j;
            long result[3];

            result[0] = 0;
            result[1] = 0;
            result[2] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);
            result[2] = result[2] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 2*w))) >> (group_inner_offset + 2*w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];
            out[idx + 2 * size] = result[2];

        }
    }
}
extern "C"

void gcrs_m_4_w_4_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 4;
            int i,j;
            long result[4];

            result[0] = 0;
            result[1] = 0;
            result[2] = 0;
            result[3] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);
            result[2] = result[2] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 2*w))) >> (group_inner_offset + 2*w)) * fullOneBit) & shared_data[group_offset + j]);
            result[3] = result[3] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 3*w))) >> (group_inner_offset + 3*w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];
            out[idx + 2 * size] = result[2];
            out[idx + 3 * size] = result[3];

        }
    }
}
extern "C"

void gcrs_m_4_w_5_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 5;
            int i,j;
            long result[4];

            result[0] = 0;
            result[1] = 0;
            result[2] = 0;
            result[3] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);
            result[2] = result[2] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 2*w))) >> (group_inner_offset + 2*w)) * fullOneBit) & shared_data[group_offset + j]);
            result[3] = result[3] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 3*w))) >> (group_inner_offset + 3*w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];
            out[idx + 2 * size] = result[2];
            out[idx + 3 * size] = result[3];

        }
    }
}
extern "C"

void gcrs_m_4_w_6_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 6;
            int i,j;
            long result[4];

            result[0] = 0;
            result[1] = 0;
            result[2] = 0;
            result[3] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);
            result[2] = result[2] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 2*w))) >> (group_inner_offset + 2*w)) * fullOneBit) & shared_data[group_offset + j]);
            result[3] = result[3] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 3*w))) >> (group_inner_offset + 3*w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];
            out[idx + 2 * size] = result[2];
            out[idx + 3 * size] = result[3];

        }
    }
}
extern "C"

void gcrs_m_4_w_7_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 7;
            int i,j;
            long result[4];

            result[0] = 0;
            result[1] = 0;
            result[2] = 0;
            result[3] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);
            result[2] = result[2] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 2*w))) >> (group_inner_offset + 2*w)) * fullOneBit) & shared_data[group_offset + j]);
            result[3] = result[3] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 3*w))) >> (group_inner_offset + 3*w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];
            out[idx + 2 * size] = result[2];
            out[idx + 3 * size] = result[3];

        }
    }
}
extern "C"

void gcrs_m_4_w_8_coding_dotprod(
  int k, int index, 
  const long *__restrict in, 
  long *__restrict out, 
  const unsigned int *__restrict bm, 
  int size)
{
    #pragma HLS INTERFACE s_axilite port=k
    #pragma HLS INTERFACE s_axilite port=index
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=bm offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=size
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1
    #pragma HLS ARRAY_PARTITION variable=shared_data complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            long shared_data[4096];

            int w = 8;
            int i,j;
            long result[4];

            result[0] = 0;
            result[1] = 0;
            result[2] = 0;
            result[3] = 0;

            const unsigned long fullOneBit = 0xFFFFFFFFFFFFFFFF;

            int worksize_perblock = BLOCK_DIM_X / w * w;
            const unsigned int idx = worksize_perblock * _bid_x + _tid_x;

            if (_tid_x >= worksize_perblock) {
            return;
            }

            if (idx >= size) {
            return;
            }

            int group_offset = (_tid_x / w) * w;
            int group_inner_offset = _tid_x % w;
            // row for each thread in the bitmatrix * row size which is k * w

            unsigned int bitInt = 0x01;
            unsigned int matrixInt;

            for ( i = 0; i < k; i++ ) {

            shared_data[_tid_x] = *(in + i*size + idx);

            #pragma unroll
            for ( j = 0; j < w; j++ ) {
            matrixInt = bm[index];
            result[0] = result[0] ^ ((((matrixInt & (bitInt<< group_inner_offset)) >> group_inner_offset) * fullOneBit) & shared_data[group_offset + j]);
            result[1] = result[1] ^ ((((matrixInt & (bitInt<< (group_inner_offset+w))) >> (group_inner_offset+w)) * fullOneBit) & shared_data[group_offset + j]);
            result[2] = result[2] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 2*w))) >> (group_inner_offset + 2*w)) * fullOneBit) & shared_data[group_offset + j]);
            result[3] = result[3] ^ ((((matrixInt & (bitInt<< (group_inner_offset + 3*w))) >> (group_inner_offset + 3*w)) * fullOneBit) & shared_data[group_offset + j]);

            ++index;
            }

            }

            out[idx] = result[0];
            out[idx + size] = result[1];
            out[idx + 2 * size] = result[2];
            out[idx + 3 * size] = result[3];

        }
    }
}
