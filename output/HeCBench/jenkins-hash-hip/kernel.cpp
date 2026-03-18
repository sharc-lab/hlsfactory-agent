#include "kernel.h"

// --- from main.cu ---
unsigned int mixRemainder(unsigned int a,
    unsigned int b,
    unsigned int c,
    unsigned int k0,
    unsigned int k1,
    unsigned int k2,
    unsigned int length )
{
  switch(length)
  {
    case 12: c+=k2; b+=k1; a+=k0; break;
    case 11: c+=k2&0xffffff; b+=k1; a+=k0; break;
    case 10: c+=k2&0xffff; b+=k1; a+=k0; break;
    case 9 : c+=k2&0xff; b+=k1; a+=k0; break;
    case 8 : b+=k1; a+=k0; break;
    case 7 : b+=k1&0xffffff; a+=k0; break;
    case 6 : b+=k1&0xffff; a+=k0; break;
    case 5 : b+=k1&0xff; a+=k0; break;
    case 4 : a+=k0; break;
    case 3 : a+=k0&0xffffff; break;
    case 2 : a+=k0&0xffff; break;
    case 1 : a+=k0&0xff; break;
    case 0 : return c;              /* zero length strings require no mixing */
  }

  final(a,b,c);
  return c;
}
extern "C"

void jk3_hash_kernel (
    const unsigned int * lengths,
    const unsigned int * initvals,
    const unsigned int * keys,
    unsigned int * out,
    const int N )
{
    #pragma HLS INTERFACE m_axi port=lengths offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=initvals offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=keys offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=out offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=N
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int id = BLOCK_DIM_X*_bid_x+_tid_x;
            if (id >= N) return;
            unsigned int length = lengths[id];
            const unsigned int initval = initvals[id];
            auto k = (const uint3*) (keys+id*16);  // each key has at most 15 words (60 bytes)

            /* Set up the internal state */
            unsigned int a,b,c;
            unsigned int r0,r1,r2;
            a = b = c = 0xdeadbeef + length + initval;

            /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
            uint3 v;
            while (length > 12) {
            v = *k++;
            a += v.x;
            b += v.y;
            c += v.z;
            mix(a,b,c);
            length -= 12;
            }
            v = *k;

            /*----------------------------- handle the last (probably partial) block */
            /*
            * "k[2]&0xffffff" actually reads beyond the end of the string, but
            * then masks off the part it's not allowed to read.  Because the
            * string is aligned, the masked-off tail is in the same word as the
            * rest of the string.  Every machine with memory protection I've seen
            * does it on word boundaries, so is OK with this.  But VALGRIND will
            * still catch it and complain.  The masking trick does make the hash
            * noticably faster for short strings (like English words).
            */
            out[id] = mixRemainder(a, b, c, v.x, v.y, v.z, length);

        }
    }
}
