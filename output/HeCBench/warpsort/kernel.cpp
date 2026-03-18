#include "kernel.h"

// --- from warpsort.cu ---
void warpMergeMN(const T a[M], const T b[N], T dst[M + N]) {
  const int laneId = getLaneId();

  // It is presumed that `a` and `b` are sorted lists of the form:
  // A: [0:lane 0-31], [1: lane 0-31], ..., [N-1: lane 0-31]
  // B: [0:lane 0-31], [1: lane 0-31], ..., [N-1: lane 0-31]
  // We are merging `a` and `b` into dst

  // `val` is the working array that we are merging into and sorting
  // Populate `val` with initial values:
  // lanes 0-15 take a[0:lane 0-15], lanes 16-31 take b[0:lane 0-15]
  T val = shfl_up(b[0], HALF_WARP_SIZE);
  val = (laneId < HALF_WARP_SIZE) ? a[0] : val;

  int aIndex = HALF_WARP_SIZE;
  int bIndex = HALF_WARP_SIZE;
  int dstIndex = 0;

  // Each time through we use a sort of 32 elements as our merge
  // primitive, and output 16 elements. For the following 16 elements,
  // we take from either `a` or `b` depending on which list is
  // guaranteed to have the larger or equivalent elements to the
  // following list.
  for ( ; ; ) {
    // Sort entries in `val`
    val = warpBitonicSort<T, Comparator>(val);

    if (dstIndex < ((M + N) * WARP_SIZE - WARP_SIZE)) {
      // Values val[lane 0-15] are sorted, output them
      scatterHalfWarp<M + N>(dst, dstIndex, val);
      dstIndex += HALF_WARP_SIZE;
    } else {
      // We've exhausted `a` and `b`. Everything left in `val` across
      // all lanes are the final values
      assert(aIndex == WARP_SIZE * M);
      assert(bIndex == WARP_SIZE * N);
      dst[M + N - 1] = val;
      break;
    }

    // It is possible that we've exhausted one of the branches (A or
    // B).
    if (aIndex == WARP_SIZE * M) {
      // We have to load from `b`; `a` has no more elements
      val = getMulti<N>(b, bIndex, val);
      bIndex += HALF_WARP_SIZE;
    } else if (bIndex == WARP_SIZE * N) {
      // We have to load from `a`; `b` has no more elements
      val = getMulti<M>(a, aIndex, val);
      aIndex += HALF_WARP_SIZE;
    } else {
      // Should we take from `a` or `b` next?
      const T compA = WarpRegisterUtils<T, M>::broadcast(a, aIndex - 1);
      const T compB = WarpRegisterUtils<T, N>::broadcast(b, bIndex - 1);

      if (Comparator::compare(compA, compB)) {
        // Load from `a` next
        val = getMulti<M>(a, aIndex, val);
        aIndex += HALF_WARP_SIZE;
      } else {
        // Load from `b` next
        val = getMulti<N>(b, bIndex, val);
        bIndex += HALF_WARP_SIZE;
      }
    }
  }
}

  static void splitAndMerge(const T in[N], T out[N]) {
    // Split the input into two sub-lists `a` and `b` as best as
    // possible
    T a[STATIC_FLOOR(N, 2)];
    T b[STATIC_CEIL(N, 2)];

    for (int i = 0; i < STATIC_FLOOR(N, 2); ++i) {
      a[i] = in[i];
    }

    for (int i = STATIC_FLOOR(N, 2); i < N; ++i) {
      b[i - STATIC_FLOOR(N, 2)] = in[i];
    }

    // Recursively split `a` and merge to a sorted list `aOut`
    T aOut[STATIC_FLOOR(N, 2)];
    Merge<T, Comparator, STATIC_FLOOR(N, 2)>::splitAndMerge(a, aOut);

    // Recursively split `b` and merge to a sorted list `bOut`
    T bOut[STATIC_CEIL(N, 2)];
    Merge<T, Comparator, STATIC_CEIL(N, 2)>::splitAndMerge(b, bOut);

    // Merge `aOut` with `bOut` to produce the final sorted list `out`
    warpMergeMN<T, Comparator, STATIC_FLOOR(N, 2), STATIC_CEIL(N, 2)>(
      aOut, bOut, out);
  }

  static void splitAndMerge(const T in[1], T out[1]) {
    out[0] = in[0];
  }

void warpSortRegisters(T a[N], T dst[N]) {
  // Sort all sub-lists of 32. We could do this in Merge's base case
  // instead, but that increases register usage, since it is at the
  // leaf of the recursion.
  for (int i = 0; i < N; ++i) {
    a[i] = warpBitonicSort<T, Comparator>(a[i]);
  }

  // Recursive subdivision to sort all a[i] together
  Merge<T, Comparator, N>::splitAndMerge(a, dst);
}

void warpSortRegisters(const DeviceTensor<T, 1>& key,
                  DeviceTensor<T, 1>& sortedKey) {

  // Load the elements we have available
  T val[N];
  WarpRegisterLoaderUtils<T, N>::load(
    val, key, NumericLimits<T>::minPossible());

  // Recursively split, shuffle sort and merge sort back
  T sortedVal[N];
  warpSortRegisters<T, Comparator, N>(val, sortedVal);

  // Write the warp's registers back out
  WarpRegisterLoaderUtils<T, N>::save(
    sortedKey, sortedVal, key.getSize(0));
}

void warpSortRegisters(const DeviceTensor<T, 1>& key,
                  DeviceTensor<T, 1>& sortedKey,
                  DeviceTensor<IndexType, 1>& sortedKeyIndices) {

  // Load the elements we have available
  Pair<T, int> pairs[N];
  WarpRegisterPairLoaderUtils<T, IndexType, N>::load(
    pairs, key,
    NumericLimits<T>::minPossible(),
    NumericLimits<IndexType>::minPossible());

  // Recursively split, shuffle sort and merge sort back
  Pair<T, IndexType> sortedPairs[N];
  warpSortRegisters<Pair<T, IndexType>, Comparator, N>(pairs, sortedPairs);

  // Write the warp's registers back out
  WarpRegisterPairLoaderUtils<T, IndexType, N>::save(
    sortedKey, sortedKeyIndices, sortedPairs, key.getSize(0));
}

void warpSortRegisters(const DeviceTensor<K, 1>& key,
                  const DeviceTensor<V, 1>& value,
                  DeviceTensor<K, 1>& sortedKey,
                  DeviceTensor<V, 1>& sortedValue) {

  // Load the elements we have available
  Pair<K, V> pairs[N];
  WarpRegisterPairLoaderUtils<K, V, N>::load(
    pairs, key, value,
    NumericLimits<K>::minPossible(),
    NumericLimits<V>::minPossible());

  // Recursively split, shuffle sort and merge sort back
  Pair<K, V> sortedPairs[N];
  warpSortRegisters<Pair<K, V>, Comparator, N>(pairs, sortedPairs);

  // Write the warp's registers back out
  WarpRegisterPairLoaderUtils<K, V, N>::save(
    sortedKey, sortedValue, sortedPairs, key.getSize(0));
}

bool warpSort(const DeviceTensor<T, 1>& key,
                         DeviceTensor<T, 1>& sortedKey) {
  assert(key.getSize(0) <= sortedKey.getSize(0));

  if (key.getSize(0) <= WARP_SIZE) {
    warpSortRegisters<float, Comparator, 1>(key, sortedKey);
    return true;
  } else if (key.getSize(0) <= 2 * WARP_SIZE) {
    warpSortRegisters<float, Comparator, 2>(key, sortedKey);
    return true;
  } else if (key.getSize(0) <= 3 * WARP_SIZE) {
    warpSortRegisters<float, Comparator, 3>(key, sortedKey);
    return true;
  } else if (key.getSize(0) <= 4 * WARP_SIZE) {
    warpSortRegisters<float, Comparator, 4>(key, sortedKey);
    return true;
  }

  // size too large
  return false;
}

bool warpSort(const DeviceTensor<T, 1>& key,
                         DeviceTensor<T, 1>& sortedKey,
                         DeviceTensor<IndexType, 1>& sortedKeyIndices) {
  assert(key.getSize(0) <= sortedKey.getSize(0) &&
         key.getSize(0) <= sortedKeyIndices.getSize(0));

  if (key.getSize(0) <= WARP_SIZE) {
    warpSortRegisters<float, IndexType, Comparator, 1>(
      key, sortedKey, sortedKeyIndices);
    return true;
  } else if (key.getSize(0) <= 2 * WARP_SIZE) {
    warpSortRegisters<float, IndexType, Comparator, 2>(
      key, sortedKey, sortedKeyIndices);
    return true;
  } else if (key.getSize(0) <= 3 * WARP_SIZE) {
    warpSortRegisters<float, IndexType, Comparator, 3>(
      key, sortedKey, sortedKeyIndices);
    return true;
  } else if (key.getSize(0) <= 4 * WARP_SIZE) {
    warpSortRegisters<float, IndexType, Comparator, 4>(
      key, sortedKey, sortedKeyIndices);
    return true;
  }

  // size too large
  return false;
}

bool warpSort(const DeviceTensor<K, 1>& key,
                         const DeviceTensor<V, 1>& value,
                         DeviceTensor<K, 1>& sortedKey,
                         DeviceTensor<V, 1>& sortedValue) {
  assert(key.getSize(0) <= sortedKey.getSize(0) &&
         value.getSize(0) <= sortedValue.getSize(0) &&
         key.getSize(0) == value.getSize(0));

  if (key.getSize(0) <= WARP_SIZE) {
    warpSortRegisters<K, V, Comparator, 1>(
      key, value, sortedKey, sortedValue);
    return true;
  } else if (key.getSize(0) <= 2 * WARP_SIZE) {
    warpSortRegisters<K, V, Comparator, 2>(
      key, value, sortedKey, sortedValue);
    return true;
  } else if (key.getSize(0) <= 3 * WARP_SIZE) {
    warpSortRegisters<K, V, Comparator, 3>(
      key, value, sortedKey, sortedValue);
    return true;
  } else if (key.getSize(0) <= 4 * WARP_SIZE) {
    warpSortRegisters<K, V, Comparator, 4>(
      key, value, sortedKey, sortedValue);
    return true;
  }

  // size too large
  return false;
}
extern "C"

void sortDevice(DeviceTensor<float, 1> data, DeviceTensor<float, 1> out) {
    #pragma HLS INTERFACE s_axilite port=DeviceTensor<float
    #pragma HLS INTERFACE s_axilite port=data
    #pragma HLS INTERFACE s_axilite port=DeviceTensor<float
    #pragma HLS INTERFACE s_axilite port=out
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS INTERFACE s_axilite port=DeviceTensor<float
    #pragma HLS INTERFACE s_axilite port=data
    #pragma HLS INTERFACE s_axilite port=DeviceTensor<float
    #pragma HLS INTERFACE s_axilite port=out
    #pragma HLS INTERFACE s_axilite port=return

  warpSort<float, GreaterThan<float> >(data, out);
}

void sortDevice(DeviceTensor<float, 1> data,
           DeviceTensor<float, 1> out,
           DeviceTensor<int, 1> indices) {
  warpSort<float, int, GreaterThan<Pair<float, int> > >(data, out, indices);
}
