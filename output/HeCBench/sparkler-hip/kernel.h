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
//=============================================================================

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <sys/time.h>
#include <errno.h>

#include <mpi.h>
#include <hip/hip_runtime.h>
#include <hipblas/hipblas.h>

//=============================================================================

#define ASSERT(condition) \
  (void)((condition) || (assert_(#condition, __FILE__, __LINE__), 0))


#define SAFE_CALL_MPI(call) \
 {int errcode = call; \

  struct timeval tv;
  gettimeofday(&tv, NULL);
  double result = ((double)tv.tv_sec + (double)tv.tv_usec * 1.e-6);

  return result;
}

//-----------------------------------------------------------------------------

/// Choices for tensor core GEMM method.

enum {
  TC_METHOD_NONE = 0,
  TC_METHOD_FLOAT16 = 1,
  TC_METHOD_INT8 = 2,
  TC_METHOD_FLOAT32 = 3,
  NUM_TC_METHOD = 4
};

//-----------------------------------------------------------------------------

template<typename GemmIn_t> struct TCBufTypes;

template<> struct TCBufTypes<float> {
};

//-----------------------------------------------------------------------------

template<int TC_METHOD> struct TCSelector;

template<> struct TCSelector<TC_METHOD_FLOAT32> {
  enum {TC_METHOD = TC_METHOD_FLOAT32};
  // types.
  typedef float GemmIn_t;
  typedef float GemmOut_t;
};

//-----------------------------------------------------------------------------

/// Matrix class, templated on scalar data type.

template<typename P_>
class Matrix {

  enum {ROUNDUP = 8};

  public:

    typedef P_ P;

    //----------


    //----------

    ~Matrix() {
      SAFE_CALL_HIP(hipHostFree(h_));



    //----------


    //----------


    //----------


    //----------


    //----------


    //----------

  private:

    size_t num_row_;
    size_t num_col_;
    size_t num_row_up_;
    size_t num_col_up_;
    size_t num_elt_up_;
    size_t sizeP;

    P* h_;
    P* d_;

    // Disallowed methods.
    Matrix(const Matrix&);
    void operator=(const Matrix&);
};

//=============================================================================

/// Greatest common divisor.


//-----------------------------------------------------------------------------

/// Least common multiple.


//-----------------------------------------------------------------------------

/// Distance between nonzero elements along a column of the matrix.


//-----------------------------------------------------------------------------

/// HIP kernel for set_input_matrix.

template<class Matrix_t>

//-----------------------------------------------------------------------------

/// Set a sparse subset of the entries of a matrix.
///
/// All entries of the matrix A are zero, except for a small number of entries
/// along each column set to 1 according to a stride.  The number of
/// interactions of elements between two columns is based on the least common
/// multiple of their respective stride values.

template<class Matrix_t>

//-----------------------------------------------------------------------------

/// A very simplistic hash for a reult matrix element, used for validation.


//-----------------------------------------------------------------------------

template<typename TCS, typename GemmIn_t, typename GemmOut_t>

//-----------------------------------------------------------------------------

template<int TC_METHOD>

//-----------------------------------------------------------------------------

