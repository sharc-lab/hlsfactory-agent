#include "kernel.h"

// --- from main.cu ---
  inline Pair() {}

  inline Pair(K key, V value)
      : k(key), v(value) {}

inline int getBit(int val, int pos) {
  return (val >> pos) & 0x1;
}

inline int getLaneId() {
  return _tid_x % WARP_SIZE;
}

  static inline bool compare(const T lhs, const T rhs) {
    return (lhs > rhs);
  }

  static inline bool compare(const T lhs, const T rhs) {
    return (lhs < rhs);
  }

inline T shflSwap(const T x, int mask, int dir) {
  T y = shfl_xor(x, mask);
  return Comparator::compare(x, y) == dir ? y : x;
}

T warpBitonicSort(T val) {
  const int laneId = getLaneId();
  // 2
  val = shflSwap<T, Comparator>(val, 0x01, getBit(laneId, 1) ^ getBit(laneId, 0));

  // 4
  val = shflSwap<T, Comparator>(val, 0x02, getBit(laneId, 2) ^ getBit(laneId, 1));
  val = shflSwap<T, Comparator>(val, 0x01, getBit(laneId, 2) ^ getBit(laneId, 0));

  // 8
  val = shflSwap<T, Comparator>(val, 0x04, getBit(laneId, 3) ^ getBit(laneId, 2));
  val = shflSwap<T, Comparator>(val, 0x02, getBit(laneId, 3) ^ getBit(laneId, 1));
  val = shflSwap<T, Comparator>(val, 0x01, getBit(laneId, 3) ^ getBit(laneId, 0));

  // 16
  val = shflSwap<T, Comparator>(val, 0x08, getBit(laneId, 4) ^ getBit(laneId, 3));
  val = shflSwap<T, Comparator>(val, 0x04, getBit(laneId, 4) ^ getBit(laneId, 2));
  val = shflSwap<T, Comparator>(val, 0x02, getBit(laneId, 4) ^ getBit(laneId, 1));
  val = shflSwap<T, Comparator>(val, 0x01, getBit(laneId, 4) ^ getBit(laneId, 0));

  // 32
  val = shflSwap<T, Comparator>(val, 0x10, getBit(laneId, 4));
  val = shflSwap<T, Comparator>(val, 0x08, getBit(laneId, 3));
  val = shflSwap<T, Comparator>(val, 0x04, getBit(laneId, 2));
  val = shflSwap<T, Comparator>(val, 0x02, getBit(laneId, 1));
  val = shflSwap<T, Comparator>(val, 0x01, getBit(laneId, 0));

  return val;
}

inline bool warpHasCollision(T val) {
  // -sort all values
  // -compare our lower neighbor's value against ourselves (excepting
  //  the first lane)
  // -if any lane as a difference of 0, there is a duplicate
  //  (excepting the first lane)
  val = warpBitonicSort<T, LessThan<T>>(val);
  const T lower = __shfl_up_sync(MASK, val, 1);

  // Shuffle for lane 0 will present its same value, so only
  // subsequent lanes will detect duplicates
  const bool dup = (lower == val) && (getLaneId() != 0);
  return (__any_sync(MASK, dup) != 0);
}

inline unsigned int warpCollisionMask(T val) {
  // -sort all (lane, value) pairs on value
  // -compare our lower neighbor's value against ourselves (excepting
  //  the first lane)
  // -if any lane as a difference of 0, there is a duplicate
  //  (excepting the first lane)
  // -shuffle sort (originating lane, dup) pairs back to the original
  //  lane and report
  Pair<T, int> pVal(val, getLaneId());

  pVal = warpBitonicSort<Pair<T, int>, LessThan<Pair<T, int> > >(pVal);

  // If our neighbor is the same as us, we know our thread's value is
  // duplicated. All except for lane 0, since shfl will present its
  // own value (and if lane 0's value is duplicated, lane 1 will pick
  // that up)
  const unsigned long lower = __shfl_up_sync(MASK, pVal.k, 1);
  Pair<int, bool> dup(pVal.v, (lower == pVal.k) && (getLaneId() != 0));

  // Sort back based on lane ID so each thread originally knows
  // whether or not it duplicated
  dup = warpBitonicSort<Pair<int, bool>,
                        LessThan<Pair<int, bool> > >(dup);
  return 0;
}
extern "C"

void checkDuplicates(int num, const int* v) {
    #pragma HLS INTERFACE s_axilite port=num
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        hasDuplicate[_tid_x] = (int) warpHasCollision(v[_tid_x]);

    }
}
extern "C"

void checkDuplicateMask(int num, const int* v) {
    #pragma HLS INTERFACE s_axilite port=num
    #pragma HLS INTERFACE m_axi port=v offset=slave bundle=gmem0
    #pragma HLS INTERFACE s_axilite port=return

    for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
    #pragma HLS PIPELINE II=1

        unsigned int mask = warpCollisionMask(v[_tid_x]);
        if (_tid_x == 0) {
        duplicateMask = mask;
        }

    }
}

unsigned int checkDuplicateMask(const vector<int>& v) {
  int* devSet = NULL;

             cudaMemcpyHostToDevice);

  unsigned int mask = 0;

  cudaMemcpyFromSymbol(&mask,
                       duplicateMask, sizeof(unsigned int), 0,
                       cudaMemcpyDeviceToHost);

  return mask;
}
