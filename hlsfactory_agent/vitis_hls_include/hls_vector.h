// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.

// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#ifndef __HLS_VECTOR_H__
#define __HLS_VECTOR_H__

#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <type_traits>

namespace hls {

#ifdef __SYNTHESIS__
#define SYN_PRAGMA(PRAG) _Pragma(#PRAG)
#ifndef AP_NODEBUG
#define AP_NODEBUG __attribute__((nodebug))
#endif // AP_NODEBUG
#else
#define SYN_PRAGMA(PRAG)
#ifndef AP_NODEBUG
#define AP_NODEBUG
#endif // AP_NODEBUG
#endif // __SYNTHESIS__

#ifdef __SYNTHESIS__
#ifndef AP_INLINE
#define AP_INLINE inline __attribute__((always_inline))
#endif
#else
#ifndef AP_INLINE
#define AP_INLINE inline
#endif
#endif // __SYNTHESIS__

// type_traits std::is_invocable_r is only available in C++17 and later,
// so we provide an equivalent hls::is_invocable_r based on std::result_of
#if __cplusplus < 201703L
template <typename _R, typename _Fn, typename... _Args>
struct is_invocable_r {
private:
  template <
    typename _U,
    typename = typename std::result_of<_U&&(_Args&&...)>::type,
    typename = typename std::enable_if<
      std::is_convertible<
        typename std::result_of<_U&&(_Args&&...)>::type, _R
      >::value
    >::type
  >
  static std::true_type test(int);

  template <typename>
  static std::false_type test(...);

public:
  static constexpr bool value = decltype(test<_Fn>(0))::value;
};

template <typename _R, typename _Fn, typename... _Args>
constexpr bool is_invocable_r_v = is_invocable_r<_R, _Fn, _Args...>::value;

template <typename _From, typename _To>
constexpr bool is_convertible_v = std::is_convertible<_From, _To>::value;
#else
using std::is_invocable_r_v;
using std::is_convertible_v;
#endif

namespace details {

/// Returns the greatest power of two that divides n
constexpr size_t gp2(size_t n) {
  if (n == 0)
    return 0;
  if (n % 2 != 0)
    return 1;
  return 2 * gp2(n / 2);
}

} // namespace details

/// SIMD Vector of `N` elements of type `T`
template <typename _T, size_t _N>
class alignas(details::gp2(sizeof(_T) * _N)) vector {
  static_assert(_N > 0, "vector must have at least one element");

  using data_t = std::array<_T, _N>;
  data_t data;

public:
#define USING_TYPE(NAME) using NAME = typename data_t::NAME
  /// Member types
  USING_TYPE(value_type);
  USING_TYPE(size_type);
  USING_TYPE(difference_type);
  USING_TYPE(reference);
  USING_TYPE(const_reference);
  USING_TYPE(pointer);
  USING_TYPE(const_pointer);
  USING_TYPE(iterator);
  USING_TYPE(const_iterator);
  USING_TYPE(reverse_iterator);
  USING_TYPE(const_reverse_iterator);
#undef USING_TYPE

#define MUTABLE_MEMBER(RET, NAME)                                              \
  AP_INLINE AP_NODEBUG RET NAME() {                                            \
    pragma();                                                                  \
    return data.NAME();                                                        \
  }

#define CONST_MEMBER(RET, NAME)                                                \
  AP_INLINE AP_NODEBUG RET NAME() const {                                      \
    pragma();                                                                  \
    return data.NAME();                                                        \
  }

  MUTABLE_MEMBER(iterator, begin)
  MUTABLE_MEMBER(iterator, end)
  MUTABLE_MEMBER(reverse_iterator, rbegin)
  MUTABLE_MEMBER(reverse_iterator, rend)
  CONST_MEMBER(const_iterator, begin)
  CONST_MEMBER(const_iterator, end)
  CONST_MEMBER(const_iterator, cbegin)
  CONST_MEMBER(const_iterator, cend)
  CONST_MEMBER(const_reverse_iterator, rbegin)
  CONST_MEMBER(const_reverse_iterator, rend)
  CONST_MEMBER(const_reverse_iterator, crbegin)
  CONST_MEMBER(const_reverse_iterator, crend)

  CONST_MEMBER(bool, empty)
  CONST_MEMBER(size_type, size)
  CONST_MEMBER(size_type, max_size)

#undef MUTABLE_MEMBER
#undef CONST_MEMBER

protected:
  /// Pragma setter (hack until we support pragma on types)
  /// Note: must be used on all functions if possible
  AP_INLINE AP_NODEBUG void pragma() const {
    SYN_PRAGMA(HLS AGGREGATE variable=this)
  }

public:
  /// Default constructor (trivial)
  vector() = default;
  /// Copy-constructor (trivial)
  vector(const vector &other) = default;
  /// Move-constructor (trivial)
  vector(vector &&other) = default;
  /// Copy-assignment operator (trivial)
  vector &operator=(const vector &other) = default;
  /// Move-assignment operator (trivial)
  vector &operator=(vector &&other) = default;
  /// Destructor (trivial)
  ~vector() = default;

  /// Note: all the above special member functions must be trivial,
  ///       as we want this class to be usable in union (POD requirement).

  /// Conversion to T (scalar)
  /// Note: only enabled for N=1
  template <size_t _N2 = _N, typename = typename std::enable_if_t<_N2 == 1>>
  AP_INLINE AP_NODEBUG operator _T() const {
    pragma();
    return data[0];
  }

  /// Construct from T (scalar)
  AP_INLINE AP_NODEBUG vector(const _T &val) {
    pragma();
    for (size_t i = 0; i < _N; ++i) {
      SYN_PRAGMA(HLS UNROLL)
      data[i] = val;
    }
  }

  /// Construct from std::array<T, N>
  AP_INLINE AP_NODEBUG vector(const std::array<_T, _N> &data) : data{data} {
    pragma();
  }

  /// Construct from std::initializer_list<T>
  AP_INLINE AP_NODEBUG vector(std::initializer_list<_T> l) {
    pragma();
    assert(l.size() == _N &&
           "Initializer list must be the same size as the vector");
    for (size_t i = 0; i < _N; ++i) {
      SYN_PRAGMA(HLS UNROLL)
      data[i] = l.begin()[i];
    }
  }

  /// Construct from an initializing lambda
  // NOTE: the lambda must be of the form T(size_t)
  //       we cannot use std::function here as it is not synthesizable
  template<typename _L,
           typename = typename std::enable_if_t<hls::is_invocable_r_v<_T, _L, size_t> &&
                                               !hls::is_convertible_v<_L, _T>>>
  AP_INLINE AP_NODEBUG vector(_L init) {
    pragma();
    for (size_t i = 0; i < _N; ++i) {
      SYN_PRAGMA(HLS UNROLL)
      data[i] = init(i);
    }
  }

  /// Array-like operator[]
  AP_INLINE AP_NODEBUG _T &operator[](size_t idx) {
    pragma();
    return data[idx];
  }
  AP_INLINE AP_NODEBUG const _T &operator[](size_t idx) const {
    pragma();
    return data[idx];
  }

#define INPLACE_PREUNOP(OP)                                                    \
  AP_INLINE AP_NODEBUG vector &operator OP() {                                 \
    pragma();                                                                  \
    for (size_t i = 0; i < _N; ++i) {                                          \
      SYN_PRAGMA(HLS UNROLL)                                                   \
      OP data[i];                                                              \
    }                                                                          \
    return *this;                                                              \
  }

  INPLACE_PREUNOP(++)
  INPLACE_PREUNOP(--)

#undef INPLACE_PREUNOP

#define INPLACE_POSTUNOP(OP)                                                   \
 AP_INLINE AP_NODEBUG vector operator OP(int) {                                \
    pragma();                                                                  \
    vector orig = *this;                                                       \
    OP *this;                                                                  \
    return orig;                                                               \
  }

  INPLACE_POSTUNOP(++)
  INPLACE_POSTUNOP(--)

#undef INPLACE_POSTUNOP

#define INPLACE_BINOP(OP)                                                      \
 AP_INLINE AP_NODEBUG vector &operator OP(const vector &rhs) {                 \
    pragma();                                                                  \
    rhs.pragma();                                                              \
    for (size_t i = 0; i < _N; ++i) {                                          \
      SYN_PRAGMA(HLS UNROLL)                                                   \
      data[i] OP rhs[i];                                                       \
    }                                                                          \
    return *this;                                                              \
  }

  INPLACE_BINOP(+=)
  INPLACE_BINOP(-=)
  INPLACE_BINOP(*=)
  INPLACE_BINOP(/=)
  INPLACE_BINOP(%=)
  INPLACE_BINOP(&=)
  INPLACE_BINOP(|=)
  INPLACE_BINOP(^=)
  INPLACE_BINOP(<<=)
  INPLACE_BINOP(>>=)

#undef INPLACE_BINOP

#define REDUCE_OP(NAME, OP)                                                    \
  AP_INLINE AP_NODEBUG _T reduce_##NAME() const {                              \
    pragma();                                                                  \
    _T res = data[0];                                                          \
    for (size_t i = 1; i < _N; ++i) {                                          \
      SYN_PRAGMA(HLS UNROLL)                                                   \
      res OP data[i];                                                          \
    }                                                                          \
    return res;                                                                \
  }

  REDUCE_OP(add,  +=)
  REDUCE_OP(mult, *=)
  REDUCE_OP(and,  &=)
  REDUCE_OP(or,   |=)
  REDUCE_OP(xor,  ^=)

#undef REDUCE_OP

#define LEXICO_OP(OP)                                                          \
  AP_INLINE AP_NODEBUG                                                         \
  friend bool operator OP(const vector &lhs, const vector &rhs) {              \
    lhs.pragma();                                                              \
    rhs.pragma();                                                              \
    for (size_t i = 0; i < _N; ++i) {                                          \
      SYN_PRAGMA(HLS UNROLL)                                                   \
      if (lhs[i] == rhs[i])                                                    \
        continue;                                                              \
      return lhs[i] OP rhs[i];                                                 \
    }                                                                          \
    return _T{} OP _T{};                                                       \
  }

#define COMPARE_OP(OP)                                                         \
  AP_INLINE AP_NODEBUG                                                         \
  friend vector<bool, N> operator OP(const vector &lhs, const vector &rhs) {   \
    lhs.pragma();                                                              \
    rhs.pragma();                                                              \
    vector<bool, _N> res;                                                      \
    for (size_t i = 0; i < N; ++i) {                                           \
      SYN_PRAGMA(HLS UNROLL)                                                   \
      res[i] = lhs[i] OP rhs[i];                                               \
    }                                                                          \
    return res;                                                                \
  }

  LEXICO_OP(<)
  LEXICO_OP(<=)
  LEXICO_OP(==)
  LEXICO_OP(!=)
  LEXICO_OP(>=)
  LEXICO_OP(>)

#undef LEXICO_OP
#undef COMPARE_OP

#define BINARY_OP(OP, INPLACE_OP)                                              \
  AP_INLINE AP_NODEBUG                                                         \
  friend vector operator OP(vector lhs, const vector &rhs) {                   \
    lhs.pragma();                                                              \
    rhs.pragma();                                                              \
    return lhs INPLACE_OP rhs;                                                 \
  }

  BINARY_OP(+, +=)
  BINARY_OP(-, -=)
  BINARY_OP(*, *=)
  BINARY_OP(/, /=)
  BINARY_OP(%, %=)
  BINARY_OP(&, &=)
  BINARY_OP(|, |=)
  BINARY_OP(^, ^=)
  BINARY_OP(<<, <<=)
  BINARY_OP(>>, >>=)

#undef BINARY_OP

  /// Iota initialize the vector with increasing values starting from `start`
  static AP_INLINE AP_NODEBUG vector iota(_T start = {}) {
    return vector([start](size_t i) { return start + i; });
  }
};

} // namespace hls

#endif
