#include "kernel.h"

// --- from main.cu ---
ulong mulfp( ulong weight, uint freq )
{
  uint part1 = weight & 0xFFFFF;         // lower 24-bits of weight
  uint part2 = (weight >> 24) & 0xFFFF;  // next 16-bits

  uint res1 = part1 * freq;
  uint res2 = part2 * freq;

  return (ulong)res1 + (((ulong)res2) << 24);
}
extern "C"

void compute ( const uint*  docWordFrequencies_dimm1, 
          const uint*  docWordFrequencies_dimm2,
          const ulong* profileWeights_dimm1,
          const ulong* profileWeights_dimm2,
          const uint*  isWordInProfileHash,
                uint*  profileScorePerGroup_highbits_dimm1,
                uint*  profileScorePerGroup_lowbits_dimm2 )
{
    #pragma HLS INTERFACE m_axi port=docWordFrequencies_dimm1 offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=docWordFrequencies_dimm2 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=profileWeights_dimm1 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=profileWeights_dimm2 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=isWordInProfileHash offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=profileScorePerGroup_highbits_dimm1 offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=profileScorePerGroup_lowbits_dimm2 offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=partial complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            uint curr_entry[MANUAL_VECTOR];
            uint word_id[MANUAL_VECTOR];
            uint freq[MANUAL_VECTOR];
            uint hash1[MANUAL_VECTOR];
            uint hash2[MANUAL_VECTOR];
            bool is_end[MANUAL_VECTOR];
            bool make_access[MANUAL_VECTOR];

            ulong partial[NUM_THREADS_PER_WG/MANUAL_VECTOR];

            int gid = _bid_x * BLOCK_DIM_X + _tid_x;

            ulong sum = 0;
            //#pragma unroll
            for (uint i=0; i<MANUAL_VECTOR; i++) {
            curr_entry[i] = docWordFrequencies_dimm1[gid*MANUAL_VECTOR + i];
            freq[i] = curr_entry[i] & 0xff;
            word_id[i] = curr_entry[i] >> 8;
            is_end[i] = curr_entry[i] == docEndingTag;
            hash1[i] = word_id[i] >> BLOOM_1;
            hash2[i] = word_id[i] & BLOOM_2;
            make_access[i] = !is_end[i] && ((isWordInProfileHash[ hash1[i] >> 5 ] >> (hash1[i] & 0x1f)) & 0x1)
            && ((isWordInProfileHash[ hash2[i] >> 5 ] >> (hash2[i] & 0x1f)) & 0x1);
            if (make_access[i]) {
            sum += mulfp(profileWeights_dimm1[word_id[i]],freq[i]);
            }
            }

            //#pragma unroll
            for (uint i=0; i<MANUAL_VECTOR; i++) {
            curr_entry[i] = docWordFrequencies_dimm2[gid*MANUAL_VECTOR + i];
            freq[i] = curr_entry[i] & 0xff;
            word_id[i] = curr_entry[i] >> 8;
            is_end[i] = curr_entry[i] == docEndingTag;
            hash1[i] = word_id[i] >> BLOOM_1;
            hash2[i] = word_id[i] & BLOOM_2;
            make_access[i] = !is_end[i] && ((isWordInProfileHash[ hash1[i] >> 5 ] >> (hash1[i] & 0x1f)) & 0x1)
            && ((isWordInProfileHash[ hash2[i] >> 5 ] >> (hash2[i] & 0x1f)) & 0x1);
            if (make_access[i]) {
            sum += mulfp(profileWeights_dimm2[word_id[i]],freq[i]);
            }
            }

            partial[_tid_x] = sum;

            if (_tid_x == 0) {
            ulong4 res= *(ulong4*)&partial[0];
            ulong4 res2= *(ulong4*)&partial[4];
            ulong final_result = res.x + res.y + res.z + res.w + res2.x + res2.y + res2.z + res2.w;
            profileScorePerGroup_highbits_dimm1[_bid_x] = (uint) (final_result >> 32);
            profileScorePerGroup_lowbits_dimm2[_bid_x] = (uint) (final_result & 0xFFFFFFFF);
            }

        }
    }
}
extern "C"

void reduction( const ulong*  docInfo,
           const uint*   partial_highbits_dimm1,
           const uint*   partial_lowbits_dimm2,
                 ulong*  result)
{
    #pragma HLS INTERFACE m_axi port=docInfo offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=partial_highbits_dimm1 offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=partial_lowbits_dimm2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=result offset=slave bundle=gmem3
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=partial complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            int gid = _bid_x * BLOCK_DIM_X + _tid_x;
            ulong info = docInfo[gid];
            unsigned start = info >> 32;
            unsigned end = info & 0xFFFFFFFF;

            ulong total = 0;
            #pragma unroll 2
            for (unsigned i=start; i<=end; i++) {
            ulong upper = partial_highbits_dimm1[i];
            ulong lower = partial_lowbits_dimm2[i];
            ulong sum = (upper << 32) | lower;
            total += sum;
            }
            result[gid] = total;

        }
    }
}
