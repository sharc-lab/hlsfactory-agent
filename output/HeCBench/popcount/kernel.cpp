#include "kernel.h"

// --- from main.cu ---
extern "C"
void pc1 (const unsigned long*  data,int*  r, const int length)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= length) return;
            unsigned long x = data[i];
            x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
            x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits
            x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
            x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
            x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
            x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
            r[i] = x & 0x7f;

        }
    }
}
extern "C"

void pc2 (const unsigned long*  data, int*  r, const int length)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= length) return;
            unsigned long x = data[i];
            x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
            x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits
            x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
            r[i] = (x * h01) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...

        }
    }
}
extern "C"

void pc3 (const unsigned long*  data, int*  r, const int length)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= length) return;
            char count;
            unsigned long x = data[i];
            for (count=0; x; count++) x &= x - 1;
            r[i] = count;

        }
    }
}
extern "C"

void pc4 (const unsigned long*  data, int*  r, const int length)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= length) return;
            unsigned long x = data[i];
            char cnt = 0;
            for (char i = 0; i < 64; i++)
            {
            cnt = cnt + (x & 0x1);
            x = x >> 1;
            }
            r[i] = cnt;

        }
    }
}
extern "C"

void pc5 (const unsigned long*  data, int*  r, const int length)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= length) return;
            unsigned long x = data[i];
            unsigned char i1 = lut[(x & 0xFF)];
            unsigned char i2 = lut[(x >> 8) & 0xFF];
            unsigned char i3 = lut[(x >> 16) & 0xFF];
            unsigned char i4 = lut[(x >> 24) & 0xFF];
            unsigned char i5 = lut[(x >> 32) & 0xFF];
            unsigned char i6 = lut[(x >> 40) & 0xFF];
            unsigned char i7 = lut[(x >> 48) & 0xFF];
            unsigned char i8 = lut[(x >> 56) & 0xFF];
            r[i] = (i1+i2)+(i3+i4)+(i5+i6)+(i7+i8);

        }
    }
}
extern "C"

void pc6 (const unsigned long*  data, int*  r, const int length)
{
    #pragma HLS INTERFACE m_axi port=data offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=r offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=length
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int i = _bid_x*BLOCK_DIM_X+_tid_x;
            if (i >= length) return;
            r[i] = __popcll(data[i]);

        }
    }
}
