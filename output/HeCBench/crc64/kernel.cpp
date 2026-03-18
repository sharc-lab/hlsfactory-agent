#include "kernel.h"

// --- from CRC64.cu ---
static inline uint32_t crc64_load_le32_(const uint32_t *p) {
  uint32_t w = *p;
  return  ((((w) & 0xff000000) >> 24)
         | (((w) & 0x00ff0000) >>  8)
         | (((w) & 0x0000ff00) <<  8)
         | (((w) & 0x000000ff) << 24));
}

uint64_t crc64_device(const unsigned char *input, size_t nbytes, 
		const uint64_t *d_crc64_table, 
		const uint64_t *d_crc64_interleaved_table) {
  const unsigned char *data = input;
  const unsigned char *end = data + nbytes;
  uint64_t cs[5] = { UINT64_C(0xffffffffffffffff), 0, 0, 0, 0 };

  // Process byte-by-byte until proper alignment is attained.
  // In the inner loop, we process 5 4-byte words (20 bytes in total)
  // per iteration. If the amount of data remaining is small,
  // then we also use the slow algorithm.
  while (data < end && ((((size_t) data) & 3) || (end - data < 20))) {
    uint32_t idx = ((uint32_t) (cs[0] ^ *data++)) & 0xff;
    cs[0] = d_crc64_table[3*256+idx] ^ (cs[0] >> 8);
  }

  if (data == end)
    return cs[0] ^ UINT64_C(0xffffffffffffffff);

  const uint32_t one = 1;
  bool big_endian = !(*((char *)(&one)));

  uint64_t cry = 0;
  uint32_t in[5];

  if (!big_endian) {
    for (unsigned i = 0; i < 5; ++i)
      in[i] = ((const uint32_t*) data)[i];
    data += 20;

    for (; end - data >= 20; data += 20) {
      cs[0] ^= cry;

      in[0] ^= (uint32_t) cs[0];
      cs[1] ^= cs[0] >> 32;
      cs[0] = d_crc64_interleaved_table[in[0] & 0xff];
      in[0] >>= 8;

      in[1] ^= (uint32_t) cs[1];
      cs[2] ^= cs[1] >> 32;
      cs[1] = d_crc64_interleaved_table[in[1] & 0xff];
      in[1] >>= 8;

      in[2] ^= (uint32_t) cs[2];
      cs[3] ^= cs[2] >> 32;
      cs[2] = d_crc64_interleaved_table[in[2] & 0xff];
      in[2] >>= 8;

      in[3] ^= (uint32_t) cs[3];
      cs[4] ^= cs[3] >> 32;
      cs[3] = d_crc64_interleaved_table[in[3] & 0xff];
      in[3] >>= 8;

      in[4] ^= (uint32_t) cs[4];
      cry = cs[4] >> 32;
      cs[4] = d_crc64_interleaved_table[in[4] & 0xff];
      in[4] >>= 8;

      for (unsigned b = 1; b < 3; ++b) {
        cs[0] ^= d_crc64_interleaved_table[b*256+(in[0] & 0xff)];
        in[0] >>= 8;

        cs[1] ^= d_crc64_interleaved_table[b*256+(in[1] & 0xff)];
        in[1] >>= 8;

        cs[2] ^= d_crc64_interleaved_table[b*256+(in[2] & 0xff)];
        in[2] >>= 8;

        cs[3] ^= d_crc64_interleaved_table[b*256+(in[3] & 0xff)];
        in[3] >>= 8;

        cs[4] ^= d_crc64_interleaved_table[b*256+(in[4] & 0xff)];
        in[4] >>= 8;
      }

      cs[0] ^= d_crc64_interleaved_table[3*256+(in[0] & 0xff)];
      in[0] = ((const uint32_t*) data)[0];

      cs[1] ^= d_crc64_interleaved_table[3*256+(in[1] & 0xff)];
      in[1] = ((const uint32_t*) data)[1];

      cs[2] ^= d_crc64_interleaved_table[3*256+(in[2] & 0xff)];
      in[2] = ((const uint32_t*) data)[2];

      cs[3] ^= d_crc64_interleaved_table[3*256+(in[3] & 0xff)];
      in[3] = ((const uint32_t*) data)[3];

      cs[4] ^= d_crc64_interleaved_table[3*256+(in[4] & 0xff)];
      in[4] = ((const uint32_t*) data)[4];
    }
  } else {
    for (unsigned i = 0; i < 5; ++i) {
      in[i] = crc64_load_le32_(&((const uint32_t*) data)[i]);
    }
    data += 20;

    for (; end - data >= 20; data += 20) {
      cs[0] ^= cry;

      in[0] ^= (uint32_t) cs[0];
      cs[1] ^= cs[0] >> 32;
      cs[0] = d_crc64_interleaved_table[in[0] & 0xff];
      in[0] >>= 8;

      in[1] ^= (uint32_t) cs[1];
      cs[2] ^= cs[1] >> 32;
      cs[1] = d_crc64_interleaved_table[in[1] & 0xff];
      in[1] >>= 8;

      in[2] ^= (uint32_t) cs[2];
      cs[3] ^= cs[2] >> 32;
      cs[2] = d_crc64_interleaved_table[in[2] & 0xff];
      in[2] >>= 8;

      in[3] ^= (uint32_t) cs[3];
      cs[4] ^= cs[3] >> 32;
      cs[3] = d_crc64_interleaved_table[in[3] & 0xff];
      in[3] >>= 8;

      in[4] ^= (uint32_t) cs[4];
      cry = cs[4] >> 32;
      cs[4] = d_crc64_interleaved_table[in[4] & 0xff];
      in[4] >>= 8;

      for (unsigned b = 1; b < 3; ++b) {
        cs[0] ^= d_crc64_interleaved_table[b*256+(in[0] & 0xff)];
        in[0] >>= 8;

        cs[1] ^= d_crc64_interleaved_table[b*256+(in[1] & 0xff)];
        in[1] >>= 8;

        cs[2] ^= d_crc64_interleaved_table[b*256+(in[2] & 0xff)];
        in[2] >>= 8;

        cs[3] ^= d_crc64_interleaved_table[b*256+(in[3] & 0xff)];
        in[3] >>= 8;

        cs[4] ^= d_crc64_interleaved_table[b*256+(in[4] & 0xff)];
        in[4] >>= 8;
      }

      cs[0] ^= d_crc64_interleaved_table[3*256+(in[0] & 0xff)];
      in[0] = crc64_load_le32_(&((const uint32_t*) data)[0]);

      cs[1] ^= d_crc64_interleaved_table[3*256+(in[1] & 0xff)];
      in[1] = crc64_load_le32_(&((const uint32_t*) data)[1]);

      cs[2] ^= d_crc64_interleaved_table[3*256+(in[2] & 0xff)];
      in[2] = crc64_load_le32_(&((const uint32_t*) data)[2]);

      cs[3] ^= d_crc64_interleaved_table[3*256+(in[3] & 0xff)];
      in[3] = crc64_load_le32_(&((const uint32_t*) data)[3]);

      cs[4] ^= d_crc64_interleaved_table[3*256+(in[4] & 0xff)];
      in[4] = crc64_load_le32_(&((const uint32_t*) data)[4]);
    }
  }

  cs[0] ^= cry;

  for (unsigned i = 0; i < 5; ++i) {
    if (i > 0)
      cs[0] ^= cs[i];
    in[i] ^= (uint32_t) cs[0];
    cs[0] = cs[0] >> 32;

    for (unsigned b = 0; b < 3; ++b) {
      cs[0] ^= d_crc64_table[b*256+(in[i] & 0xff)];
      in[i] >>= 8;
    }

    cs[0] ^= d_crc64_table[3*256+(in[i] & 0xff)];
  }

  while (data < end) {
    uint32_t idx = ((uint32_t) (cs[0] ^ *data++)) & 0xff;
    cs[0] = d_crc64_table[3*256+idx] ^ (cs[0] >> 8);
  }

  return cs[0] ^ UINT64_C(0xffffffffffffffff);
}
extern "C"

void crc64_kernel(
  size_t * d_thread_sz, 
  uint64_t * d_thread_cs, 
  const unsigned char*  d_data, 
  const uint64_t * d_crc64_table, 
  const uint64_t * d_crc64_interleaved_table, 
  size_t nbytes, int nthreads) 
{
    #pragma HLS INTERFACE m_axi port=d_thread_sz offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=d_thread_cs offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=d_data offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=d_crc64_table offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=d_crc64_interleaved_table offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=nbytes
    #pragma HLS INTERFACE s_axilite port=nthreads
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            size_t bpt = nbytes/nthreads;
            const unsigned char *start = d_data + bpt*tid;
            const unsigned char *end;
            if (tid != nthreads - 1)
            end = start + bpt;
            else
            end = d_data + nbytes;

            size_t sz = end - start;
            d_thread_sz[tid] = sz;
            d_thread_cs[tid] = crc64_device(start, sz, d_crc64_table, d_crc64_interleaved_table);

        }
    }
}
