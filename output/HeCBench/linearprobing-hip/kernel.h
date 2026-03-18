#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// CUDA thread/block dimension constants (adjust for your workload)
#ifndef BLOCK_DIM_X
#define BLOCK_DIM_X 256
#endif
#ifndef GRID_DIM_X
#define GRID_DIM_X 1
#endif

// --- from linearprobing.cu ---
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <chrono>

// 32 bit Murmur3 hash

// Insert the key/values in kvs into the hashtable


// Delete each key in kvs from the hash table, if the key exists
// A deleted key is left in the hash table, but its value is set to kEmpty
// Deleted keys are not reused; once a key is assigned a slot, it never moves


// Iterate over every item in the hashtable; return non-empty key/values

std::vector<KeyValue> iterate_hashtable(KeyValue* pHashTable)
{
  uint32_t* device_num_kvs;
  hipMalloc((void**) &device_num_kvs, sizeof(uint32_t));
  hipMemset(device_num_kvs, 0, sizeof(uint32_t));

  KeyValue* device_kvs;
  hipMalloc((void**) &device_kvs, sizeof(KeyValue) * kNumKeyValues);

  const int threadblocksize = 256;
  int gridsize = (kHashTableCapacity + threadblocksize - 1) / threadblocksize;

  hipDeviceSynchronize();
  auto start = std::chrono::steady_clock::now();

  hipLaunchKernelGGL(k_iterate_hashtable, gridsize, threadblocksize, 0, 0, pHashTable, device_kvs, device_num_kvs);

  hipDeviceSynchronize();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Kernel execution time (iterate): %f (s)\n", time * 1e-9f);

  uint32_t num_kvs;
  hipMemcpy(&num_kvs, device_num_kvs, sizeof(uint32_t), hipMemcpyDeviceToHost);

  std::vector<KeyValue> kvs;
  kvs.resize(num_kvs);

  hipMemcpy(kvs.data(), device_kvs, sizeof(KeyValue) * num_kvs, hipMemcpyDeviceToHost);

  hipFree(device_kvs);
  hipFree(device_num_kvs);

  return kvs;
}


// --- from main.cu ---
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>

// Create random keys/values in the range [0, kEmpty)
// kEmpty is used to indicate an empty slot
std::vector<KeyValue> generate_random_keyvalues(std::mt19937& rnd, uint32_t numkvs)
{
  std::uniform_int_distribution<uint32_t> dis(0, kEmpty - 1);

  std::vector<KeyValue> kvs;
  kvs.reserve(numkvs);


  return kvs;
}

// return numshuffledkvs random items from kvs
std::vector<KeyValue> shuffle_keyvalues(std::mt19937& rnd, std::vector<KeyValue> kvs, uint32_t numshuffledkvs)
{
  std::shuffle(kvs.begin(), kvs.end(), rnd);

  std::vector<KeyValue> shuffled_kvs;
  shuffled_kvs.resize(numshuffledkvs);

  std::copy(kvs.begin(), kvs.begin() + numshuffledkvs, shuffled_kvs.begin());

  return shuffled_kvs;
}

using Time = std::chrono::time_point<std::chrono::high_resolution_clock>;






// --- from test.cu ---
#include <stdio.h>
#include <stdint.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <random>



// --- from linearprobing.h ---
#pragma once

#include <hip/hip_runtime.h>

struct KeyValue
{
    uint32_t key;
    uint32_t value;
};

const uint32_t kHashTableCapacity = 64*1024*1024; //128 * 1024 * 1024;

const uint32_t kNumKeyValues = kHashTableCapacity / 2;

const uint32_t kEmpty = 0xffffffff;

double insert_hashtable(KeyValue* hashtable, const KeyValue* kvs, uint32_t num_kvs);

double delete_hashtable(KeyValue* hashtable, const KeyValue* kvs, uint32_t num_kvs);

std::vector<KeyValue> iterate_hashtable(KeyValue* hashtable);
