#include "kernel.h"

// --- from main.cu ---
unsigned my_abs ( int x )
{
  unsigned t = x >> 31;
  return (x ^ t) - t;
}

unsigned f1(ulong value, bool* mask) {
  if (value == 0) {
    *mask = 1;
  } 
  return 1;
}

unsigned f2(ulong value, bool* mask) {
  if (my_abs((int)(value)) <= 0xFF) *mask = 1;
  return 1;
}

unsigned f3(ulong value, bool* mask) {
  if (my_abs((int)(value)) <= 0xFFFF) *mask = 1;
  return 2;
}

unsigned f4(ulong value, bool* mask) {
  if (((value) & 0xFFFF) == 0 ) *mask = 1;
  return 2;
}

unsigned f5(ulong value, bool* mask) {
  if ((my_abs((int)((value) & 0xFFFF))) <= 0xFF && 
      my_abs((int)((value >> 16) & 0xFFFF)) <= 0xFF) 
    *mask = 1;
  return 2;
}

unsigned f6(ulong value, bool* mask) {
  unsigned byte0 = (value) & 0xFF;
  unsigned byte1 = (value >> 8) & 0xFF;
  unsigned byte2 = (value >> 16) & 0xFF;
  unsigned byte3 = (value >> 24) & 0xFF;
  if (byte0 == byte1 && byte0 == byte2 && byte0 == byte3) 
    *mask = 1;
  return 1;
}

unsigned f7(ulong value, bool* mask) {
  *mask = 1;
  return 4;
}
extern "C"

void fpc_kernel (const ulong* values, unsigned *cmp_size)
{
    #pragma HLS INTERFACE m_axi port=values offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=cmp_size offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned compressable;
            int lid = _tid_x;
            int WGS = BLOCK_DIM_X;
            int gid = _bid_x*WGS+lid;

            ulong value = values[gid];
            unsigned inc;

            // 000
            if (value == 0){
            inc = 1;
            }
            // 001 010
            else if ((my_abs((int)(value)) <= 0xFF)) {
            inc = 1;
            }
            // 011
            else if ((my_abs((int)(value)) <= 0xFFFF)) {
            inc = 2;
            }
            //100
            else if ((((value) & 0xFFFF) == 0 )) {
            inc = 2;
            }
            //101
            else if ((my_abs((int)((value) & 0xFFFF))) <= 0xFF
            && my_abs((int)((value >> 16) & 0xFFFF)) <= 0xFF ) {
            inc = 2;
            }
            //110
            else if( (((value) & 0xFF) == ((value >> 8) & 0xFF)) &&
            (((value) & 0xFF) == ((value >> 16) & 0xFF)) &&
            (((value) & 0xFF) == ((value >> 24) & 0xFF)) ) {
            inc = 1;
            } else {
            inc = 4;
            }

            if (lid == 0) compressable = 0;

            (compressable += inc);
            if (lid == WGS-1) {
            (*cmp_size += compressable);
            }

        }
    }
}
extern "C"

void fpc2_kernel (const ulong* values, unsigned *cmp_size)
{
    #pragma HLS INTERFACE m_axi port=values offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=cmp_size offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned compressable;
            int lid = _tid_x;
            int WGS = BLOCK_DIM_X;
            int gid = _bid_x*WGS+lid;

            unsigned inc;

            bool m1 = 0;
            bool m2 = 0;
            bool m3 = 0;
            bool m4 = 0;
            bool m5 = 0;
            bool m6 = 0;
            bool m7 = 0;

            ulong value = values[gid];
            unsigned inc1 = f1(value, &m1);
            unsigned inc2 = f2(value, &m2);
            unsigned inc3 = f3(value, &m3);
            unsigned inc4 = f4(value, &m4);
            unsigned inc5 = f5(value, &m5);
            unsigned inc6 = f6(value, &m6);
            unsigned inc7 = f7(value, &m7);

            if (m1)
            inc = inc1;
            else if (m2)
            inc = inc2;
            else if (m3)
            inc = inc3;
            else if (m4)
            inc = inc4;
            else if (m5)
            inc = inc5;
            else if (m6)
            inc = inc6;
            else
            inc = inc7;

            if (lid == 0) compressable = 0;

            (compressable += inc);
            if (lid == WGS-1) {
            (*cmp_size += compressable);
            }

        }
    }
}
