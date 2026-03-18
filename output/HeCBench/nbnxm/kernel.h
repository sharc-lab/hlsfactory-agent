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
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef BLOCK_DIM_Z
#define BLOCK_DIM_Z 1
#endif
#ifndef GRID_DIM_Z
#define GRID_DIM_Z 1
#endif

// --- from main.cu ---
#include <chrono>
#include <iostream>

typedef float2 Float2;
typedef gmx::BasicVector<float> Float3;
typedef float4 Float4;


#if (CUDART_VERSION >= 9000)
#define __shfl_up(v, d) __shfl_up_sync(0xffffffff, v, d)
#define __shfl_down(v, d) 0
#endif

inline void operator+=(float4 &a, float4 b)
{
  a.x += b.x;
  a.y += b.y;
  a.z += b.z;
  a.w += b.w;
}












// --- from constants.h ---

template<std::uint64_t n>
struct StaticLog2
{
  //!< Variable value used for recursive static calculation of Log2(int)
  static const int value = StaticLog2<n / 2>::value + 1;
};
template<>
struct StaticLog2<1>
{
  //!< Base value for recursive static calculation of Log2(int)
  static const int value = 0;
};
template<>
struct StaticLog2<0>
{
  //!< Base value for recursive static calculation of Log2(int)
  static const int value = -1;
};

constexpr unsigned int NBNXN_INTERACTION_MASK_ALL = 0xffffffffU;

static constexpr int c_nbnxnGpuClusterpairSplit = 2;
static constexpr int c_nbnxnGpuNumClusterPerSupercluster = 8;
static constexpr int c_nbnxnGpuClusterSize = 8;
static constexpr int c_clSize = c_nbnxnGpuClusterSize;
constexpr int c_nbnxnGpuJgroupSize = (32 / c_nbnxnGpuNumClusterPerSupercluster);
static constexpr int c_nbnxnGpuExclSize = c_nbnxnGpuClusterSize * c_nbnxnGpuClusterSize 
                                          / c_nbnxnGpuClusterpairSplit;
static constexpr int c_splitClSize = c_clSize / c_nbnxnGpuClusterpairSplit;

constexpr int c_dBoxZ = 1;
constexpr int c_dBoxY = 1;
constexpr int c_dBoxX = 2;
constexpr int c_nBoxZ = 2 * c_dBoxZ + 1;
constexpr int c_nBoxY = 2 * c_dBoxY + 1;
constexpr int c_nBoxX = 2 * c_dBoxX + 1;
constexpr int c_numIvecs = c_nBoxZ * c_nBoxY * c_nBoxX;

constexpr int c_centralShiftIndex = c_numIvecs / 2;
static constexpr unsigned superClInteractionMask = ((1U << c_nbnxnGpuNumClusterPerSupercluster) - 1U);
constexpr float c_nbnxnMinDistanceSquared = 3.82e-07F; // r > 6.2e-4

// atom index is computed as shown in the following code snippet
//  for (int i = 0; i < c_nbnxnGpuNumClusterPerSupercluster; i += c_clSize)
//  {
//     const int ci = sci * c_nbnxnGpuNumClusterPerSupercluster + tidxj + i;
//     const int ai = ci * c_clSize + tidxi;
//
// As c_nbnxnGpuNumClusterPerSupercluster equals c_nbnxnGpuClusterSize, and c_clSize
// equals c_nbnxnGpuClusterSize, i is always 0 for the specific case
//
// grid size in the z dim
constexpr int grid_z = 3199;
// thread block size in the x and y dims
constexpr int block_x = 8;
constexpr int block_y = 8;
constexpr int NUM_ATOMS = (grid_z * c_nbnxnGpuNumClusterPerSupercluster + block_x) * c_clSize + block_y;

struct nbnxn_im_ei_t
{
  //! The i-cluster interactions mask for 1 warp
  unsigned int imask = 0U;
  //! Index into the exclusion array for 1 warp, default index 0 which means no exclusions
  int excl_ind = 0;
};

typedef struct
{
  //! The 4 j-clusters
  int cj[c_nbnxnGpuJgroupSize];
  //! The i-cluster mask data for 2 warps
  nbnxn_im_ei_t imei[c_nbnxnGpuClusterpairSplit];
} nbnxn_cj4_t;

typedef struct nbnxn_sci
{
  //! Returns the number of j-cluster groups in this entry
  int numJClusterGroups() const { return cj4_ind_end - cj4_ind_start; }

  //! i-super-cluster
  int sci;
  //! Shift vector index plus possible flags
  int shift;
  //! Start index into cj4
  int cj4_ind_start;
  //! End index into cj4
  int cj4_ind_end;
} nbnxn_sci_t;

struct nbnxn_excl_t
{
  //! Constructor, sets no exclusions, so all atom pairs interacting
  nbnxn_excl_t()
  {
    for (unsigned int& pairEntry : pair)
    {
      pairEntry = NBNXN_INTERACTION_MASK_ALL;
    }
  }

  //! Topology exclusion interaction bits per warp
  unsigned int pair[c_nbnxnGpuExclSize];
};


// --- from vectypes.h ---
#ifndef GMX_MATH_VECTYPES_H
#define GMX_MATH_VECTYPES_H

#include <cassert>
#include <cmath>
#include <algorithm>
#include <type_traits>

#define XX 0 /* Defines for indexing in */
#define YY 1 /* vectors                 */
#define ZZ 2
#define DIM 3 /* Dimension of vectors    */

typedef float real;

typedef real rvec[DIM];

typedef double dvec[DIM];

typedef real matrix[DIM][DIM];

typedef real tensor[DIM][DIM];

typedef int ivec[DIM];

namespace gmx
{

/*! \brief
 * C++ class for 3D vectors.
 *
 * \tparam ValueType  Type
 *
 * This class provides a C++ version of rvec/dvec/ivec that can be put into STL
 * containers etc.  It is more or less a drop-in replacement for `rvec` and
 * friends: it can be used in most contexts that accept the equivalent C type.
 * However, there is one case where explicit conversion is necessary:
 *  - An array of these objects needs to be converted with as_vec_array() (or
 *    convenience methods like as_rvec_array()).
 *
 * For the array conversion to work, the compiler should not add any extra
 * alignment/padding in the layout of this class;  that this actually works as
 * intended is tested in the unit tests.
 *
 * \inpublicapi
 */
template<typename ValueType>
class BasicVector
{
public:
    //! Underlying raw C array type (rvec/dvec/ivec).
    using RawArray = ValueType[DIM];

    // The code here assumes ValueType has been deduced as a data type like int
    // and not a pointer like int*. If there is a use case for a 3-element array
    // of pointers, the implementation will be different enough that the whole
    // template class should have a separate partial specialization. We try to avoid
    // accidental matching to pointers, but this assertion is a no-cost extra check.
    //
    // TODO: Use std::is_pointer_v when CUDA 11 is a requirement.
    static_assert(!std::is_pointer<std::remove_cv_t<ValueType>>::value,
                  "BasicVector value type must not be a pointer.");

    //! Constructs default (uninitialized) vector.
    BasicVector() {}
    //! Constructs a vector from given values.
    BasicVector(ValueType x, ValueType y, ValueType z) : x_{ x, y, z } {}
    /*! \brief
     * Constructs a vector from given values.
     *
     * This constructor is not explicit to support implicit conversions
     * that allow, e.g., calling `std::vector<RVec>:``:push_back()` directly
     * with an `rvec` parameter.
     */
    BasicVector(const RawArray x) : x_{ x[XX], x[YY], x[ZZ] } {}
    //! Default copy constructor.
    BasicVector(const BasicVector& src) = default;
    //! Default copy assignment operator.
    BasicVector& operator=(const BasicVector& v) = default;
    //! Default move constructor.
    BasicVector(BasicVector&& src) noexcept = default;
    //! Default move assignment operator.
    BasicVector& operator=(BasicVector&& v) noexcept = default;
    //! Indexing operator to make the class work as the raw array.
    ValueType& operator[](int i) { return x_[i]; }
    //! Indexing operator to make the class work as the raw array.
    ValueType operator[](int i) const { return x_[i]; }
    //! Return whether all elements compare equal
    bool operator==(const BasicVector<ValueType>& right)
    {
        return x_[0] == right[0] && x_[1] == right[1] && x_[2] == right[2];
    }
    //! Return whether any elements compare unequal
    bool operator!=(const BasicVector<ValueType>& right)
    {
        return x_[0] != right[0] || x_[1] != right[1] || x_[2] != right[2];
    }
    //! Allow inplace addition for BasicVector
    inline BasicVector<ValueType>& operator+=(const BasicVector<ValueType>& right)
    {
        return *this = *this + right;
    }
    //! Allow inplace subtraction for BasicVector
    BasicVector<ValueType>& operator-=(const BasicVector<ValueType>& right)
    {
        return *this = *this - right;
    }
    //! Allow vector addition
    BasicVector<ValueType> operator+(const BasicVector<ValueType>& right) const
    {
        return { x_[0] + right[0], x_[1] + right[1], x_[2] + right[2] };
    }
    //! Allow vector subtraction
    inline BasicVector<ValueType> operator-(const BasicVector<ValueType>& right) const
    {
        return { x_[0] - right[0], x_[1] - right[1], x_[2] - right[2] };
    }
    //! Allow vector scalar division
    BasicVector<ValueType> operator/(const ValueType& right) const
    {
        assert((right != 0 && "Cannot divide by zero"));

        return *this * (1 / right);
    }
    //! Scale vector by a scalar
    BasicVector<ValueType>& operator*=(const ValueType& right)
    {
        x_[0] *= right;
        x_[1] *= right;
        x_[2] *= right;

        return *this;
    }
    //! Divide vector by a scalar
    BasicVector<ValueType>& operator/=(const ValueType& right)
    {
        assert((right != 0 && "Cannot divide by zero"));

        return *this *= 1 / right;
    }
    //! Return dot product
    inline ValueType dot(const BasicVector<ValueType>& right) const
    {
        return x_[0] * right[0] + x_[1] * right[1] + x_[2] * right[2];
    }

    //! Allow vector vector multiplication (cross product)
    BasicVector<ValueType> cross(const BasicVector<ValueType>& right) const
    {
        return { x_[YY] * right.x_[ZZ] - x_[ZZ] * right.x_[YY],
                 x_[ZZ] * right.x_[XX] - x_[XX] * right.x_[ZZ],
                 x_[XX] * right.x_[YY] - x_[YY] * right.x_[XX] };
    }

    //! Return normalized to unit vector
    BasicVector<ValueType> unitVector() const
    {
        const ValueType vectorNorm = norm();
        assert((vectorNorm != 0 && "unitVector() should not be called with a zero vector"));

        return *this / vectorNorm;
    }

    //! Length^2 of vector
    inline ValueType norm2() const { return dot(*this); }

    //! Norm or length of vector
    ValueType norm() const { return std::sqrt(norm2()); }

    //! cast to RVec
    BasicVector<real> toRVec() const { return { real(x_[0]), real(x_[1]), real(x_[2]) }; }

    //! cast to IVec
    BasicVector<int> toIVec() const
    {
        return { static_cast<int>(x_[0]), static_cast<int>(x_[1]), static_cast<int>(x_[2]) };
    }

    //! cast to DVec
    BasicVector<double> toDVec() const { return { double(x_[0]), double(x_[1]), double(x_[2]) }; }

    //! Converts to a raw C array where implicit conversion does not work.
    RawArray& as_vec() { return x_; }
    //! Converts to a raw C array where implicit conversion does not work.
    const RawArray& as_vec() const { return x_; }
    //! Makes BasicVector usable in contexts where a raw C array is expected.
    operator RawArray&() { return x_; }
    //! Makes BasicVector usable in contexts where a raw C array is expected.
    operator const RawArray&() const { return x_; }

private:
    RawArray x_;
};

//! Allow vector scalar multiplication
template<typename ValueType>
inline BasicVector<ValueType> operator*(const BasicVector<ValueType>& basicVector, const ValueType& scalar)
{
    return { basicVector[0] * scalar, basicVector[1] * scalar, basicVector[2] * scalar };
}

//! Allow scalar vector multiplication
template<typename ValueType>
inline BasicVector<ValueType> operator*(const ValueType& scalar, const BasicVector<ValueType>& basicVector)
{
    return { scalar * basicVector[0], scalar * basicVector[1], scalar * basicVector[2] };
}

/*! \brief
 * unitv for gmx::BasicVector
 */
template<typename VectorType>
static inline VectorType unitVector(const VectorType& v)
{
    return v.unitVector();
}

/*! \brief
 * norm for gmx::BasicVector
 */
template<typename ValueType>
static inline ValueType norm(BasicVector<ValueType> v)
{
    return v.norm();
}

/*! \brief
 * Square of the vector norm for gmx::BasicVector
 */
template<typename ValueType>
static inline ValueType norm2(BasicVector<ValueType> v)
{
    return v.norm2();
}

/*! \brief
 * cross product for gmx::BasicVector
 */
template<typename VectorType>
static inline VectorType cross(const VectorType& a, const VectorType& b)
{
    return a.cross(b);
}

/*! \brief
 * dot product for gmx::BasicVector
 */
template<typename ValueType>
static inline ValueType dot(BasicVector<ValueType> a, BasicVector<ValueType> b)
{
    return a.dot(b);
}

/*! \brief
 * Multiply two vectors element by element and return the result.
 */
template<typename VectorType>
static inline VectorType scaleByVector(const VectorType& a, const VectorType& b)
{
    return { a[0] * b[0], a[1] * b[1], a[2] * b[2] };
}

/*! \brief
 * Return the element-wise minimum of two vectors.
 */
template<typename VectorType>
static inline VectorType elementWiseMin(const VectorType& a, const VectorType& b)
{
    return { std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]) };
}

/*! \brief
 * Return the element-wise maximum of two vectors.
 */
template<typename VectorType>
static inline VectorType elementWiseMax(const VectorType& a, const VectorType& b)
{
    return { std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]) };
}

/*! \brief
 * Casts a gmx::BasicVector array into an equivalent raw C array.
 */
template<typename ValueType>
static inline typename BasicVector<ValueType>::RawArray* as_vec_array(BasicVector<ValueType>* x)
{
    return reinterpret_cast<typename BasicVector<ValueType>::RawArray*>(x);
}

/*! \brief
 * Casts a gmx::BasicVector array into an equivalent raw C array.
 */
template<typename ValueType>
static inline const typename BasicVector<ValueType>::RawArray* as_vec_array(const BasicVector<ValueType>* x)
{
    return reinterpret_cast<const typename BasicVector<ValueType>::RawArray*>(x);
}

//! Shorthand for C++ `rvec`-equivalent type.
typedef BasicVector<real> RVec;
//! Shorthand for C++ `dvec`-equivalent type.
typedef BasicVector<double> DVec;
//! Shorthand for C++ `ivec`-equivalent type.
typedef BasicVector<int> IVec;
//! Casts a gmx::RVec array into an `rvec` array.
static inline rvec* as_rvec_array(RVec* x)
{
    return as_vec_array(x);
}
//! Casts a gmx::RVec array into an `rvec` array.
static inline const rvec* as_rvec_array(const RVec* x)
{
    return as_vec_array(x);
}
//! Casts a gmx::DVec array into an `Dvec` array.
static inline dvec* as_dvec_array(DVec* x)
{
    return as_vec_array(x);
}
//! Casts a gmx::IVec array into an `ivec` array.
static inline ivec* as_ivec_array(IVec* x)
{
    return as_vec_array(x);
}

//! Casts a gmx::DVec array into an `dvec` array.
static inline const dvec* as_dvec_array(const DVec* x)
{
    return as_vec_array(x);
}
//! Casts a gmx::IVec array into an `ivec` array.
static inline const ivec* as_ivec_array(const IVec* x)
{
    return as_vec_array(x);
}

//! Shorthand for C++ `ivec`-equivalent type.
typedef BasicVector<int> IVec;

} // namespace gmx

#endif // include guard

