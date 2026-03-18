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

// --- from main.cu ---
#include <stdio.h>
#include <stdlib.h>
#include <chrono>

#define THREADS 256
#define BLOCKS(N) (N + THREADS - 1) / THREADS

#define CHECK_CUDA(func)                                                       \
{                                                                              \
    cudaError_t status = func;                                                 \
}

template <typename scalar_t, ReductionType REDUCE>

template<typename scalar_t, ReductionType REDUCE>



// --- from TensorInfo.h ---
#pragma once

#include <cassert>

#define MAX_TENSORINFO_DIMS 25

// kernel argument that defines tensor layout
template <typename T, typename IndexType>
struct TensorInfo {
  TensorInfo();
  TensorInfo(T* p,
             int dim,
             IndexType sz[MAX_TENSORINFO_DIMS],
             IndexType st[MAX_TENSORINFO_DIMS]);

  // Set the size of the given dimension to 1, as if it were a
  // reduction dim (allows you to calculate offsets of the reduction
  // slice)
  void reduceDim(int dim);

  T* data;
  IndexType sizes[MAX_TENSORINFO_DIMS];
  IndexType strides[MAX_TENSORINFO_DIMS];
  int dims;
};

template <typename T, typename IndexType>
TensorInfo<T, IndexType>::TensorInfo() {
  data = nullptr;
  dims = 0;
}

template <typename T, typename IndexType>
TensorInfo<T, IndexType>::TensorInfo(T* p,
                                     int dim,
                                     IndexType sz[MAX_TENSORINFO_DIMS],
                                     IndexType st[MAX_TENSORINFO_DIMS]) {
  data = p;
  dims = dim;
  assert(dims < MAX_TENSORINFO_DIMS);

  for (int i = 0; i < dim; ++i) {
    sizes[i] = sz[i];
    strides[i] = st[i];
  }
}

template <typename T, typename IndexType>
void
TensorInfo<T, IndexType>::reduceDim(int dim) {
  // expected dim between 0 and dims - 1");
  assert(dim < dims && dim >= 0);
  sizes[dim] = 1;
}

// Translate a linear index for the apply to a T* offset;
// specialized on `Dims` to reduce compilation time
template <typename T, typename IndexType, int Dims>
struct IndexToOffset {
  static IndexType get(
    IndexType linearId,
    const TensorInfo<T, IndexType>& info) {

    IndexType offset = 0;

    // Uses static dims
    for (int i = Dims - 1; i > 0; --i) {
      IndexType curDimIndex = linearId % info.sizes[i];
      IndexType curDimOffset = curDimIndex * info.strides[i];
      offset += curDimOffset;
      linearId /= info.sizes[i];
    }

    return offset + linearId * info.strides[0];
  }
};

// Uses dynamic (runtime) instead of static (compiletime) dims
template <typename T, typename IndexType>
struct IndexToOffset<T, IndexType, -1> {
  static inline IndexType get(
    IndexType linearId,
    const TensorInfo<T, IndexType>& info) {

      IndexType offset = 0;

      for (int i = info.dims - 1; i > 0; --i) {
        IndexType curDimIndex = linearId % info.sizes[i];
        IndexType curDimOffset = curDimIndex * info.strides[i];
        offset += curDimOffset;
        linearId /= info.sizes[i];
      }

      return offset + linearId * info.strides[0];
  }
};


// --- from atomics.h ---
#pragma once

#define ATOMIC(NAME)                                                           \
  template <typename scalar, size_t size> struct Atomic##NAME##IntegerImpl;    \
                                                                               \
  template <typename scalar> struct Atomic##NAME##IntegerImpl<scalar, 4> {     \
    inline void operator()(scalar *address, scalar val) {           \
      uint32_t *address_as_ui = (uint32_t *)address;                           \
      uint32_t old = *address_as_ui;                                           \
      uint32_t assumed;                                                        \
                                                                               \
      do {                                                                     \
        assumed = old;                                                         \
        old = atomicCAS(address_as_ui, assumed, OP(val, (scalar)old));         \
      } while (assumed != old);                                                \
    }                                                                          \
  };                                                                           \
                                                                               \
  template <typename scalar> struct Atomic##NAME##IntegerImpl<scalar, 8> {     \
    inline void operator()(scalar *address, scalar val) {           \
      unsigned long long *address_as_ull = (unsigned long long *)address;      \
      unsigned long long old = *address_as_ull;                                \
      unsigned long long assumed;                                              \
                                                                               \
      do {                                                                     \
        assumed = old;                                                         \
        old = atomicCAS(address_as_ull, assumed, OP(val, (scalar)old));        \
      } while (assumed != old);                                                \
    }                                                                          \
  };                                                                           \
                                                                               \
  template <typename scalar, size_t size> struct Atomic##NAME##DecimalImpl;    \
                                                                               \
                                                                               \
  template <typename scalar> struct Atomic##NAME##DecimalImpl<scalar, 4> {     \
    inline void operator()(scalar *address, scalar val) {           \
      int *address_as_i = (int *)address;                                      \
      int old = *address_as_i;                                                 \
      int assumed;                                                             \
                                                                               \
      do {                                                                     \
        assumed = old;                                                         \
        old = atomicCAS(address_as_i, assumed,                                 \
                        __float_as_int(OP(val, __int_as_float(assumed))));     \
      } while (assumed != old);                                                \
    }                                                                          \
  };                                                                           \
                                                                               \
  template <typename scalar> struct Atomic##NAME##DecimalImpl<scalar, 8> {     \
    inline void operator()(scalar *address, scalar val) {           \
      unsigned long long int *address_as_ull =                                 \
          (unsigned long long int *)address;                                   \
      unsigned long long int old = *address_as_ull;                            \
      unsigned long long int assumed;                                          \
                                                                               \
      do {                                                                     \
        assumed = old;                                                         \
        old = atomicCAS(                                                       \
            address_as_ull, assumed,                                           \
            __double_as_longlong(OP(val, __longlong_as_double(assumed))));     \
      } while (assumed != old);                                                \
    }                                                                          \
  };

#define OP(X, Y) Y + X
ATOMIC(Add)
#undef OP
static inline void atomAdd(int32_t *address, int32_t val) {
  (*address += val);
}
static inline void atomAdd(int64_t *address, int64_t val) {
  AtomicAddIntegerImpl<int64_t, sizeof(int64_t)>()(address, val);
}
static inline void atomAdd(float *address, float val) {
  (*address += val);
}
#if defined(__CUDA_ARCH__) && (__CUDA_ARCH__ < 600 || CUDA_VERSION < 8000)
static inline void atomAdd(double *address, double val) {
  AtomicAddDecimalImpl<double, sizeof(double)>()(address, val);
}
#else
static inline void atomAdd(double *address, double val) {
  (*address += val);
}
#endif

#define OP(X, Y) Y *X
ATOMIC(Mul)
#undef OP
static inline void atomMul(int32_t *address, int32_t val) {
  AtomicMulIntegerImpl<int32_t, sizeof(int32_t)>()(address, val);
}
static inline void atomMul(int64_t *address, int64_t val) {
  AtomicMulIntegerImpl<int64_t, sizeof(int64_t)>()(address, val);
}
static inline void atomMul(float *address, float val) {
  AtomicMulDecimalImpl<float, sizeof(float)>()(address, val);
}
static inline void atomMul(double *address, double val) {
  AtomicMulDecimalImpl<double, sizeof(double)>()(address, val);
}

#define OP(X, Y) Y / X
ATOMIC(Div)
#undef OP
static inline void atomDiv(int32_t *address, int32_t val) {
  AtomicDivIntegerImpl<int32_t, sizeof(int32_t)>()(address, val);
}
static inline void atomDiv(int64_t *address, int64_t val) {
  AtomicDivIntegerImpl<int64_t, sizeof(int64_t)>()(address, val);
}
static inline void atomDiv(float *address, float val) {
  AtomicDivDecimalImpl<float, sizeof(float)>()(address, val);
}
static inline void atomDiv(double *address, double val) {
  AtomicDivDecimalImpl<double, sizeof(double)>()(address, val);
}

#define OP(X, Y) max(Y, X)
ATOMIC(Max)
#undef OP
static inline void atomMax(int32_t *address, int32_t val) {
  (*address = max(*address, val));
}
static inline void atomMax(int64_t *address, int64_t val) {
  AtomicMaxIntegerImpl<int64_t, sizeof(int64_t)>()(address, val);
}
static inline void atomMax(float *address, float val) {
  AtomicMaxDecimalImpl<float, sizeof(float)>()(address, val);
}
static inline void atomMax(double *address, double val) {
  AtomicMaxDecimalImpl<double, sizeof(double)>()(address, val);
}

#define OP(X, Y) min(Y, X)
ATOMIC(Min)
#undef OP
static inline void atomMin(int32_t *address, int32_t val) {
  (*address = min(*address, val));
}
static inline void atomMin(int64_t *address, int64_t val) {
  AtomicMinIntegerImpl<int64_t, sizeof(int64_t)>()(address, val);
}
static inline void atomMin(float *address, float val) {
  AtomicMinDecimalImpl<float, sizeof(float)>()(address, val);
}
static inline void atomMin(double *address, double val) {
  AtomicMinDecimalImpl<double, sizeof(double)>()(address, val);
}


// --- from reducer.h ---
#pragma once

#include <limits>
#include <map>
#include <string>

enum ReductionType { SUM, MUL, DIV, MIN, MAX };

const std::map<std::string, ReductionType> reduce2REDUCE = {
    {"sum", SUM}, {"mul", MUL}, {"div", DIV}, {"min", MIN}, {"max", MAX}
};

template <typename scalar_t, ReductionType REDUCE> struct Reducer {
  static inline scalar_t init() {
    if (REDUCE == MUL || REDUCE == DIV)
      return (scalar_t)1;
    else if (REDUCE == MIN)
      return std::numeric_limits<scalar_t>::max();
    else if (REDUCE == MAX)
      return std::numeric_limits<scalar_t>::lowest();
    else
      return (scalar_t)0;
  }

  static inline void update(scalar_t &val,
                                                scalar_t new_val) {
    if (REDUCE == SUM)
      val += new_val;
    else if (REDUCE == MUL)
      val *= new_val;
    else if (REDUCE == DIV)
      val /= new_val;
    else if ((REDUCE == MIN && new_val < val) ||
             (REDUCE == MAX && new_val > val)) {
      val = new_val;
    }
  }

  static inline void atomic_write(scalar_t *address, scalar_t val) {
    if (REDUCE == SUM)
      atomAdd(address, val);
    else if (REDUCE == MUL)
      atomMul(address, val);
    else if (REDUCE == DIV)
      atomDiv(address, val);
    else if (REDUCE == MIN)
      atomMin(address, val);
    else if (REDUCE == MAX)
      atomMax(address, val);
  }
};
