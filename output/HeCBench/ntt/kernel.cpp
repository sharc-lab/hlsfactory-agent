#include "kernel.h"

// --- from main.cu ---
extern "C"
void intt_3_64k_modcrt(
        uint32 * dst,
  const uint64 * src)
{
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buffer complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uint64 buffer[512];
            register uint64 samples[8], s8[8];
            register uint32 fmem, tmem, fbuf, tbuf;
            fmem = (bidx<<9)|((tidx&0x3E)<<3)|(tidx&0x1);
            tbuf = tidx<<3;
            fbuf = ((tidx&0x38)<<3) | (tidx&0x7);
            tmem = (bidx<<9)|((tidx&0x38)<<3) | (tidx&0x7);
            #pragma unroll
            for (int i=0; i<8; i++)
            samples[i] = src[fmem|(i<<1)];
            ntt8(samples);

            #pragma unroll
            for (int i=0; i<8; i++)
            buffer[tbuf|i] = _ls_modP(samples[i], ((tidx&0x1)<<2)*i*3);

            #pragma unroll
            for (int i=0; i<8; i++)
            samples[i] = buffer[fbuf|(i<<3)];

            #pragma unroll
            for (int i=0; i<4; i++) {
            s8[2*i] = _add_modP(samples[2*i], samples[2*i+1]);
            s8[2*i+1] = _sub_modP(samples[2*i], samples[2*i+1]);
            }

            #pragma unroll
            for (int i=0; i<8; i++) {
            dst[(((tmem|(i<<3))&0xf)<<12)|((tmem|(i<<3))>>4)] =
            (uint32)(_mul_modP(s8[i], 18446462594437939201UL, valP));
            }

        }
    }
}
