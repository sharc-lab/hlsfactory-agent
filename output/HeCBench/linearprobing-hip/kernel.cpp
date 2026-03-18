#include "kernel.h"

// --- from linearprobing.cu ---
uint32_t hash(uint32_t k)
{
  k ^= k >> 16;
  k *= 0x85ebca6b;
  k ^= k >> 13;
  k *= 0xc2b2ae35;
  k ^= k >> 16;
  return k & (kHashTableCapacity-1);
}
extern "C"

void k_hashtable_insert(KeyValue* hashtable,
                   const KeyValue* kvs,
                   unsigned int numkvs)
{
    #pragma HLS INTERFACE m_axi port=hashtable offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=kvs offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=numkvs
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if (tid < numkvs)
            {
            uint32_t key = kvs[tid].key;
            uint32_t value = kvs[tid].value;
            uint32_t slot = hash(key);

            while (true)
            {
            uint32_t prev = atomicCAS(&hashtable[slot].key, kEmpty, key);
            if (prev == kEmpty || prev == key)
            {
            hashtable[slot].value = value;
            return;
            }

            slot = (slot + 1) & (kHashTableCapacity-1);
            }
            }

        }
    }
}
extern "C"

void k_hashtable_delete(KeyValue* hashtable, 
                   const KeyValue* kvs,
                   unsigned int numkvs)
{
    #pragma HLS INTERFACE m_axi port=hashtable offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=kvs offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=numkvs
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if (tid < numkvs)
            {
            uint32_t key = kvs[tid].key;
            uint32_t slot = hash(key);

            while (true)
            {
            if (hashtable[slot].key == key)
            {
            hashtable[slot].value = kEmpty;
            return;
            }
            if (hashtable[slot].key == kEmpty)
            {
            return;
            }
            slot = (slot + 1) & (kHashTableCapacity - 1);
            }
            }

        }
    }
}
extern "C"

void k_iterate_hashtable(KeyValue* pHashTable,
                    KeyValue* kvs,
                    uint32_t* kvs_size)
{
    #pragma HLS INTERFACE m_axi port=pHashTable offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=kvs offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=kvs_size offset=slave bundle=gmem2
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            unsigned int tid = _bid_x * BLOCK_DIM_X + _tid_x;
            if (tid < kHashTableCapacity)
            {
            if (pHashTable[tid].key != kEmpty)
            {
            uint32_t value = pHashTable[tid].value;
            if (value != kEmpty)
            {
            uint32_t size = (*kvs_size += 1);
            kvs[size] = pHashTable[tid];
            }
            }
            }

        }
    }
}
