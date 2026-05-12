// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.

// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#ifndef __AP_FLOAT_H__
#define __AP_FLOAT_H__

#include <etc/ap_common.h>

#include <ap_int.h>
#include <ap_fixed.h>
#include <hls_math.h>

#ifndef __SYNTHESIS__
#include <floating_point_v7_1_bitacc_cmodel.h>
#endif

#include <cstring>
#include <limits>
#include <type_traits>
#ifndef __SYNTHESIS__
#include <iostream>
#endif

// Forward declaration(s)
template<int _W, int _E>
class ap_float;

// Shorthand for native type equivalents
using ap_float_half = ap_float<16, 5>;
using ap_float_single = ap_float<32, 8>;
using ap_float_double = ap_float<64, 11>;

// Shorthand for AI related types
using ap_float_bf16 = ap_float<16, 8>;
using ap_float_tf32 = ap_float<19, 8>;

namespace {
// Log base 2 (ceiling) helper function
static constexpr int log2_ceil(int val) {
  if (val <= 0)
    return std::numeric_limits<int>::lowest();
  if (val == 1)
    return 0;
  //return 32 - __builtin_clz(val - 1);
  return log2_ceil((val + 1) / 2) + 1;
}
} // anonymous namespace

/** Arbitrary Precision Floating-Point Class
 *   - _W: Total bitwidth (half = 16, float = 32, double = 64)
 *   - _E: Exponent's bitwidth (half = 5, float = 8, double = 11)
 */
template<int _W, int _E>
class ap_float {
public:
  // Raw bits representation
  using bits_t = ap_int<_W>;

private:
  // Internal representation stored into a single ap_int<_W>
  // Note the ap_int<_W> alignment requirement. (aligned to next power-of-two)
  bits_t bits;

  static constexpr int _M = _W - _E; // note that this includes the "hidden" bit
  static_assert(_E >= 4, "ap_float exponent must be at least 4 bits");
  static_assert(_E <= 16, "ap_float exponent must be at most 16 bits");
  static_assert(_M >= 4, "ap_float significand must be at least 4 bits");
  static_assert(_M <= 64, "ap_float significand must be at most 64 bits");
  static_assert(_E >= log2_ceil(_M+3)+1, "ap_float exponent must be at least "
                                         "ceil(log2(significand+3))+1 bits");

#ifndef __SYNTHESIS__
  // Raw conversion from double (rounding not supported)
  void from_double(const double &x) {
    static_assert(_W <= 64, "Raw conversion from double supported up to 64 total bits");
    static_assert(_M <= 53, "Raw conversion from double supported up to 53 mantissa bits");
    static_assert(_E <= 11, "Raw conversion from double supported up to 11 exponent bits");
    ap_float<64, 11> val;
    std::memcpy(&val, &x, sizeof(val));
    sign_ref() = val.sign_ref();

    if (val.exponent_ref() == val.exponent_denorm()) {
      // Sub-normals rounded to 0
      exponent_ref() = exponent_denorm();
      mantissa_ref() = 0;
    } else if (val.exponent_ref() == val.exponent_infinity()) {
      if (val.mantissa_ref() == 0) {
        // Infinite stay infinite
        exponent_ref() = exponent_infinity();
        mantissa_ref() = 0;
      } else {
        // NaN become Canonical QNaN
        exponent_ref() = exponent_infinity();
        mantissa_ref() = mantissa_quiet_mask();
      }
    } else {
      // Normals are assumed to fit in the range (rounding must be done before)
      set_unbiased_exponent(val.get_unbiased_exponent());
      mantissa_ref() = ap_int<_M-1>(val.mantissa_ref() >> (53-_M));
    }
  }

  // Raw conversion to double (rounding not supported)
  double to_double() const {
    static_assert(_W <= 64, "Raw conversion to double supported up to 64 total bits");
    static_assert(_M <= 53, "Raw conversion to double supported up to 53 mantissa bits");
    static_assert(_E <= 11, "Raw conversion to double supported up to 11 exponent bits");
    ap_float<64, 11> val;
    val.sign_ref() = sign_ref();

    if (exponent_ref() == exponent_denorm()) {
      // Sub-normals rounded to 0
      val.exponent_ref() = val.exponent_denorm();
      val.mantissa_ref() = 0;
    } else if (exponent_ref() == exponent_infinity()) {
      if (mantissa_ref() == 0) {
        // Infinite stay infinite
        val.exponent_ref() = val.exponent_infinity();
        val.mantissa_ref() = 0;
      } else {
        // NaN become Canonical QNaN
        val.exponent_ref() = val.exponent_infinity();
        val.mantissa_ref() = -1;
      }
    } else {
      // Normals are assumed to fit in the range (rounding must be done before)
      val.set_unbiased_exponent(get_unbiased_exponent());
      val.mantissa_ref() = ap_int<64>(mantissa_ref()) << (53-_M);
    }
    double res;
    std::memcpy(&res, &val, sizeof(val));
    return res;
  }

  // Initialize the xip_fpo_t to match precision/bitwidth
  void init_xip_fpo_t(xip_fpo_t &res) const {
    xip_fpo_init2(res, _E, _M);
  }

  // Conversion to xip_fpo_t
  void to_xip_fpo_t(xip_fpo_t &res) const {
    // FIXME Raw double conversion may cause weird rounding
    xip_fpo_set_d(res, to_double());
  }

  // Conversion from xip_fpo_t
  void from_xip_fpo_t(const xip_fpo_t &f) {
    // FIXME Raw double conversion may cause weird rounding
    from_double(xip_fpo_get_d(f));
  }

  // Clear the xip_fpo_t to free memory
  void fini_xip_fpo_t(xip_fpo_t &res) const {
    xip_fpo_clear(res);
  }
#endif

private:
  // Internal reference types
  using bits_bit_ref_t = ap_bit_ref<bits_t::width, bits_t::sign_flag>;
  using bits_range_ref_t = ap_range_ref<bits_t::width, bits_t::sign_flag>;

public:
  // Sign, Exponent and Mantissa (value types)
  using sign_t = ap_int<1>;
  using exponent_t = ap_uint<_E>;
  using mantissa_t = ap_uint<_M-1>;

  // Sign, Exponent and Mantissa (reference types) (to avoid unnecessary copies)
  using sign_ref_t = bits_bit_ref_t;
  using exponent_ref_t = bits_range_ref_t;
  using mantissa_ref_t = bits_range_ref_t;

  // Clean names for bitwidth values
  static constexpr int width = _W;
  static constexpr int exponent_width = _E;
  static constexpr int mantissa_width = _M;

  // All constructor, copy-constructor, copy-assignment are default
  ap_float() = default;
  ~ap_float() = default;
  ap_float(ap_float &&) = default;
  ap_float(const ap_float &) = default;
  ap_float& operator=(ap_float &&) = default;
  ap_float& operator=(const ap_float &) = default;

  // Returns a reference to the raw bits
  AP_INLINE AP_NODEBUG bits_t& bits_ref() noexcept { return bits; }
  AP_INLINE AP_NODEBUG const bits_t& bits_ref() const noexcept { return bits; }

  // Returns a reference to the sign bit
  AP_INLINE AP_NODEBUG sign_ref_t sign_ref() noexcept { return bits[_W-1]; }
  AP_INLINE AP_NODEBUG bool sign_ref() const noexcept { return bits[_W-1]; }

  // Returns a reference to the exponent bits range (biased value)
  AP_INLINE AP_NODEBUG exponent_ref_t exponent_ref() noexcept { return bits.range(_W-2, _M-1); }
  AP_INLINE AP_NODEBUG const exponent_ref_t exponent_ref() const noexcept { return bits.range(_W-2, _M-1); }

  // Returns a reference to the mantissa bits range (doesn't include sign)
  AP_INLINE AP_NODEBUG mantissa_ref_t mantissa_ref() noexcept { return bits.range(_M-2, 0); }
  AP_INLINE AP_NODEBUG const mantissa_ref_t mantissa_ref() const noexcept { return bits.range(_M-2, 0); }

  // Bits and pieces constructor (biased exponent)
  AP_INLINE AP_NODEBUG
  ap_float(sign_t s, exponent_t e, mantissa_t m) noexcept {
    sign_ref() = s;
    exponent_ref() = e;
    mantissa_ref() = m;
  }

  AP_INLINE AP_NODEBUG
  ap_float(sign_ref_t s, exponent_ref_t e, mantissa_ref_t m) noexcept {
    sign_ref() = s;
    exponent_ref() = e;
    mantissa_ref() = m;
  }

  // Exponent special values
  AP_INLINE AP_NODEBUG static constexpr exponent_t exponent_denorm() noexcept { return 0; }
  AP_INLINE AP_NODEBUG static constexpr exponent_t exponent_min() noexcept { return 1; }
  AP_INLINE AP_NODEBUG static constexpr exponent_t exponent_max() noexcept { return -2; }
  AP_INLINE AP_NODEBUG static constexpr exponent_t exponent_infinity() noexcept { return -1; }

  // Mantissa special values
  AP_INLINE AP_NODEBUG
  static constexpr mantissa_t mantissa_quiet_mask() noexcept {
    return ((mantissa_t)-1) << (_M-2);
  }

  // Exponent bias
  AP_INLINE AP_NODEBUG
  static constexpr int exponent_bias() noexcept {
    return (exponent_t(1) << (_E-1)) - 1;
  }

  // Special values
  AP_INLINE AP_NODEBUG
  static constexpr ap_float lowest() noexcept {
    return ap_float(true, exponent_max(), -1);
  }
  AP_INLINE AP_NODEBUG
  static constexpr ap_float zero() noexcept {
    return ap_float(false, exponent_denorm(), 0);
  }
  AP_INLINE AP_NODEBUG
  static constexpr ap_float min_denorm() noexcept {
    return ap_float(false, exponent_denorm(), 1);
  }
  AP_INLINE AP_NODEBUG
  static constexpr ap_float min() noexcept {
    return ap_float(false, exponent_min(), 0);
  }
  AP_INLINE AP_NODEBUG
  static constexpr ap_float epsilon() noexcept {
    return ap_float(false, exponent_bias() - (_M-1), 0);
  }
  AP_INLINE AP_NODEBUG
  static constexpr ap_float one() noexcept {
    return ap_float(false, exponent_bias(), 0);
  }
  AP_INLINE AP_NODEBUG
  static constexpr ap_float one_half() noexcept {
    return ap_float(false, exponent_bias() - 1, 0);
  }
  AP_INLINE AP_NODEBUG
  static constexpr ap_float max() noexcept {
    return ap_float(false, exponent_max(), -1);
  }
  AP_INLINE AP_NODEBUG
  static constexpr ap_float infinity() noexcept {
    return ap_float(false, exponent_infinity(), 0);
  }
  AP_INLINE AP_NODEBUG
  static constexpr ap_float quiet_NaN() noexcept {
    return ap_float(false, exponent_infinity(), mantissa_quiet_mask());
  }
  AP_INLINE AP_NODEBUG
  static constexpr ap_float signaling_NaN() noexcept {
    return ap_float(false, exponent_infinity(), mantissa_quiet_mask() >> 1);
  }

  // Friend with other template specialization of itself (to allow conversions)
  template<int _W2, int _E2>
  friend class ap_float;

  // Get exponent value (unbiased value, only valid for normals)
  AP_INLINE AP_NODEBUG
  int get_unbiased_exponent() const noexcept {
    int biased = exponent_ref();
    assert(biased > exponent_denorm() && "Unbiased exponent on denormals is unsupported");
    assert(biased < exponent_infinity() && "Unbiased exponent on infinite/NaN is unsupported");
    return biased - exponent_bias();
  }

  // Set exponent value (unbiased value, only valid for normals)
  AP_INLINE AP_NODEBUG
  void set_unbiased_exponent(int e) noexcept {
    int biased = e + exponent_bias();
    assert(biased > exponent_denorm() && "Unbiased exponent on denormals is unsupported");
    assert(biased < exponent_infinity() && "Unbiased exponent on infinite/NaN is unsupported");
    exponent_ref() = biased;
  }

  // Test if conversion to/from each native types are free
  static constexpr bool is_half = _W == 16 && _E == 5;
  static constexpr bool is_single = _W == 32 && _E == 8;
  static constexpr bool is_double = _W == 64 && _E == 11;

  // Conversion from native floating point 
  template<typename Enabled = void>
  AP_INLINE AP_NODEBUG
  ap_float(const half &h, std::enable_if_t<is_half, Enabled>* = 0) noexcept {
#ifndef __SYNTHESIS__
    xip_fpo_t val;
    this->init_xip_fpo_t(val);
    xip_fpo_set_flt(val, (float)h);
    this->from_xip_fpo_t(val);
    this->fini_xip_fpo_t(val);
#else
    std::memcpy(&bits, &h, sizeof(bits));
#endif
  }

  template<typename Enabled = void>
  AP_INLINE AP_NODEBUG
  ap_float(const half &h, std::enable_if_t<!is_half, Enabled>* = 0) noexcept
      : ap_float(ap_float_half(h)) {}

  template<typename Enabled = void>
  AP_INLINE AP_NODEBUG
  ap_float(const float &f, std::enable_if_t<is_single, Enabled>* = 0) noexcept {
#ifndef __SYNTHESIS__
    xip_fpo_t val;
    this->init_xip_fpo_t(val);
    xip_fpo_set_flt(val, f);
    this->from_xip_fpo_t(val);
    this->fini_xip_fpo_t(val);
#else
    std::memcpy(&bits, &f, sizeof(bits));
#endif
  }
  
  template<typename Enabled = void>
  AP_INLINE AP_NODEBUG
  ap_float(const float &f, std::enable_if_t<!is_single, Enabled>* = 0) noexcept
      : ap_float(ap_float_single(f)) {}

  template<typename Enabled = void>
  AP_INLINE AP_NODEBUG
  ap_float(const double &d, std::enable_if_t<is_double, Enabled>* = 0) noexcept {
#ifndef __SYNTHESIS__
    xip_fpo_t val;
    this->init_xip_fpo_t(val);
    xip_fpo_set_d(val, d);
    this->from_xip_fpo_t(val);
    this->fini_xip_fpo_t(val);
#else
    std::memcpy(&bits, &d, sizeof(bits));
#endif
  }
  
  template<typename Enabled = void>
  AP_INLINE AP_NODEBUG
  ap_float(const double &d, std::enable_if_t<!is_double, Enabled>* = 0) noexcept :
      ap_float(ap_float_double(d)) {}

  // Conversion to native floating point types
  template<typename Enabled = half>
  AP_INLINE AP_NODEBUG
  std::enable_if_t<is_half, Enabled> to_half() const noexcept {
    half res;
    std::memcpy(&res, &bits, sizeof(bits));
    return res;
  }

  template<typename Enabled = half>
  AP_INLINE AP_NODEBUG
  std::enable_if_t<!is_half, Enabled> to_half() const noexcept {
    return ap_float_half(*this).to_half();
  }

  template<typename Enabled = float>
  AP_INLINE AP_NODEBUG
  std::enable_if_t<is_single, Enabled> to_float() const noexcept {
    float res;
    std::memcpy(&res, &bits, sizeof(bits));
    return res;
  }

  template<typename Enabled = float>
  AP_INLINE AP_NODEBUG
  std::enable_if_t<!is_single, Enabled> to_float() const noexcept {
    return ap_float_single(*this).to_float();
  }

  template<typename Enabled = double>
  AP_INLINE AP_NODEBUG
  std::enable_if_t<is_double, Enabled> to_double() const noexcept {
    double res;
    std::memcpy(&res, &bits, sizeof(bits));
    return res;
  }

  template<typename Enabled = double>
  AP_INLINE AP_NODEBUG
  std::enable_if_t<!is_double, Enabled> to_double() const noexcept {
    return ap_float_double(*this).to_double();
  }

  AP_INLINE AP_NODEBUG
  explicit operator half() const noexcept {
    return to_half();
  }

  AP_INLINE AP_NODEBUG
  explicit operator float() const noexcept {
    return to_float();
  }

  AP_INLINE AP_NODEBUG
  explicit operator double() const noexcept {
    return to_double();
  }

  // Conversion from ap_fixed
  template<int _W2, int _I2, ap_q_mode _Q2, ap_o_mode _O2, int _N2>
  AP_INLINE AP_NODEBUG
  explicit ap_float(const ap_fixed<_W2, _I2, _Q2, _O2, _N2> &other) noexcept {
    static_assert(_E >= log2_ceil(_W2+3)+1,
                  "conversion from ap_fixed only supported on ap_float with"
                  " sufficient exponent bitwidth");
#ifdef __SYNTHESIS__
    __fpga_float_from_fixed(&bits, &other, _W2, _I2, _W, _E);
#else
    xip_fpo_t out;
    int base = 2;
    std::string str;

    this->init_xip_fpo_t(out);

    str = other.to_string(base , true); // example : -3.75 -> -11.11
    const char* ch = str.c_str();
    xip_fpo_set_str(out, ch, base);
    this->from_xip_fpo_t(out);
  
    this->fini_xip_fpo_t(out);
#endif
  }

  // Conversion to ap_fixed
  template<int _W2, int _I2, ap_q_mode _Q2, ap_o_mode _O2, int _N2>
  AP_INLINE AP_NODEBUG
  explicit operator ap_fixed<_W2, _I2, _Q2, _O2, _N2>() const noexcept {
    static_assert(_E >= log2_ceil(_W2+3)+1,
                  "conversion from ap_fixed only supported on ap_float with"
                  " sufficient exponent bitwidth");
#ifdef __SYNTHESIS__
    ap_fixed<_W2, _I2, _Q2, _O2, _N2> res;
    __fpga_float_to_fixed(&res, this, _W, _E, _W2, _I2);
    return res;
#else
    ap_fixed<_W2, _I2, _Q2, _O2, _N2> res;
    xip_fpo_t in;
    xip_fpo_fix_t out;
    xip_fpo_prec_t i = _I2;
    xip_fpo_prec_t frac = _W2 - _I2;
    int base = 2;
    char* ch = nullptr;

    this->init_xip_fpo_t(in);
    this->to_xip_fpo_t(in);
    xip_fpo_fix_init2(out, i, frac);
    
    xip_fpo_flttofix(out,in);
    ch = xip_fpo_fix_get_str(ch, base , out); // example : -3.75 -> -11.11
    res = ap_fixed<_W2, _I2, _Q2, _O2, _N2>(ch, base); // if base = 10 this line is: res = ch
    
    this->fini_xip_fpo_t(in);
    xip_fpo_fix_clear(out);
    xip_fpo_free_str(ch);
    
    return res;
  #endif
  }

  // Conversions from/to other bitwidth
  template<int _W2, int _E2>
  AP_INLINE AP_NODEBUG
  explicit ap_float(const ap_float<_W2, _E2> &other) noexcept {
#ifdef __SYNTHESIS__
    __fpga_float_to_float(this, &other, _W2, _E2, _W, _E);
#else
    xip_fpo_t in;
    xip_fpo_t out;
    other.init_xip_fpo_t(in);
    this->init_xip_fpo_t(out);

    other.to_xip_fpo_t(in);
    xip_fpo_flttoflt(out, in);
    this->from_xip_fpo_t(out);

    other.fini_xip_fpo_t(in);
    this->fini_xip_fpo_t(out);
#endif
  }

  // Conversion from integer types
#define AP_FLOAT_FROM_INT(_T)                                                  \
  AP_INLINE AP_NODEBUG                                                         \
  ap_float(signed _T i) noexcept : ap_float(ap_fixed<_W,_W>(i)) {}             \
  AP_INLINE AP_NODEBUG                                                         \
  ap_float(unsigned _T i) noexcept : ap_float(ap_fixed<_W,_W>(i)) {}
  AP_FLOAT_FROM_INT(char)
  AP_FLOAT_FROM_INT(short)
  AP_FLOAT_FROM_INT(int)
  AP_FLOAT_FROM_INT(long)
  AP_FLOAT_FROM_INT(long long)
#undef AP_FLOAT_FROM_INT

  // Conversion to integer types
#define AP_FLOAT_TO_INT(_T)                                                    \
  AP_INLINE AP_NODEBUG                                                         \
  explicit operator signed _T() const noexcept {                               \
    return (_T) (ap_fixed<_W,_W>) *this;                                       \
  }                                                                            \
  AP_INLINE AP_NODEBUG                                                         \
  explicit operator unsigned _T() const noexcept {                             \
    return (_T) (ap_fixed<_W,_W>) *this;                                       \
  }
  AP_FLOAT_TO_INT(char)
  AP_FLOAT_TO_INT(short)
  AP_FLOAT_TO_INT(int)
  AP_FLOAT_TO_INT(long)
  AP_FLOAT_TO_INT(long long)
#undef AP_FLOAT_TO_INT

  // Inplace addition
  AP_INLINE AP_NODEBUG
  ap_float<_W, _E>& operator+=(const ap_float<_W, _E> &other) noexcept {
#ifdef __SYNTHESIS__
    __fpga_float_add(this, this, &other, _W, _E);
#else
    xip_fpo_t lhs;
    xip_fpo_t rhs;
    xip_fpo_t out;
    this->init_xip_fpo_t(lhs);
    other.init_xip_fpo_t(rhs);
    this->init_xip_fpo_t(out);

    this->to_xip_fpo_t(lhs);
    other.to_xip_fpo_t(rhs);
    xip_fpo_add(out, lhs, rhs);
    this->from_xip_fpo_t(out);

    this->fini_xip_fpo_t(lhs);
    other.fini_xip_fpo_t(rhs);
    this->fini_xip_fpo_t(out);
#endif
    return *this;
  }

  // Inplace subtraction
  AP_INLINE AP_NODEBUG
  ap_float<_W, _E>& operator-=(const ap_float<_W, _E> &other) noexcept {
#ifdef __SYNTHESIS__
    __fpga_float_sub(this, this, &other, _W, _E);
#else
    xip_fpo_t lhs;
    xip_fpo_t rhs;
    xip_fpo_t out;
    this->init_xip_fpo_t(lhs);
    other.init_xip_fpo_t(rhs);
    this->init_xip_fpo_t(out);

    this->to_xip_fpo_t(lhs);
    other.to_xip_fpo_t(rhs);
    xip_fpo_sub(out, lhs, rhs);
    this->from_xip_fpo_t(out);

    this->fini_xip_fpo_t(lhs);
    other.fini_xip_fpo_t(rhs);
    this->fini_xip_fpo_t(out);
#endif
    return *this;
  }

  // Inplace multiplication
  AP_INLINE AP_NODEBUG
  ap_float<_W, _E>& operator*=(const ap_float<_W, _E> &other) noexcept {
#ifdef __SYNTHESIS__
    __fpga_float_mul(this, this, &other, _W, _E);
#else
    xip_fpo_t lhs;
    xip_fpo_t rhs;
    xip_fpo_t out;
    this->init_xip_fpo_t(lhs);
    other.init_xip_fpo_t(rhs);
    this->init_xip_fpo_t(out);

    this->to_xip_fpo_t(lhs);
    other.to_xip_fpo_t(rhs);
    xip_fpo_mul(out, lhs, rhs);
    this->from_xip_fpo_t(out);

    this->fini_xip_fpo_t(lhs);
    other.fini_xip_fpo_t(rhs);
    this->fini_xip_fpo_t(out);
#endif
    return *this;
  }

  // Inplace division
  AP_INLINE AP_NODEBUG
  ap_float<_W, _E>& operator/=(const ap_float<_W, _E> &other) noexcept {
#ifdef __SYNTHESIS__
    __fpga_float_div(this, this, &other, _W, _E);
#else
    xip_fpo_t lhs;
    xip_fpo_t rhs;
    xip_fpo_t out;
    this->init_xip_fpo_t(lhs);
    other.init_xip_fpo_t(rhs);
    this->init_xip_fpo_t(out);

    this->to_xip_fpo_t(lhs);
    other.to_xip_fpo_t(rhs);
    xip_fpo_div(out, lhs, rhs);
    this->from_xip_fpo_t(out);

    this->fini_xip_fpo_t(lhs);
    other.fini_xip_fpo_t(rhs);
    this->fini_xip_fpo_t(out);
#endif
    return *this;
  }

  // Inplace pre-increment
  AP_INLINE AP_NODEBUG
  ap_float& operator++() noexcept {
    *this += one();
    return *this;
  }

  // Inplace post-increment
  AP_INLINE AP_NODEBUG
  ap_float operator++(int) noexcept {
    ap_float res = *this;
    ++*this;
    return res;
  }

  // Inplace pre-decrement
  AP_INLINE AP_NODEBUG
  ap_float& operator--() noexcept {
    *this -= one();
    return *this;
  }

  // Inplace post-decrement
  AP_INLINE AP_NODEBUG
  ap_float operator--(int) noexcept {
    ap_float res = *this;
    --*this;
    return res;
  }

  // Comparisons
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend bool operator==(const ap_float<_W0, _E0> &lhs,
                         const ap_float<_W0, _E0> &rhs) noexcept;
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend bool operator!=(const ap_float<_W0, _E0> &lhs,
                         const ap_float<_W0, _E0> &rhs) noexcept;
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend bool operator<(const ap_float<_W0, _E0> &lhs,
                        const ap_float<_W0, _E0> &rhs) noexcept;
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend bool operator>(const ap_float<_W0, _E0> &lhs,
                        const ap_float<_W0, _E0> &rhs) noexcept;
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend bool operator<=(const ap_float<_W0, _E0> &lhs,
                         const ap_float<_W0, _E0> &rhs) noexcept;
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend bool operator>=(const ap_float<_W0, _E0> &lhs,
                         const ap_float<_W0, _E0> &rhs) noexcept;

  // Unary Expression Operator Overloading
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend ap_float<_W0, _E0> operator+(ap_float<_W0, _E0> val) noexcept;
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend ap_float<_W0, _E0> operator-(ap_float<_W0, _E0> val) noexcept;

  // Binary Expression Operator Overloading
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend ap_float<_W0, _E0> operator+(ap_float<_W0, _E0> lhs,
                                      const ap_float<_W0, _E0> &rhs) noexcept;
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend ap_float<_W0, _E0> operator-(ap_float<_W0, _E0> lhs,
                                      const ap_float<_W0, _E0> &rhs) noexcept;
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend ap_float<_W0, _E0> operator*(ap_float<_W0, _E0> lhs,
                                      const ap_float<_W0, _E0> &rhs) noexcept;
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend ap_float<_W0, _E0> operator/(ap_float<_W0, _E0> lhs,
                                      const ap_float<_W0, _E0> &rhs) noexcept;

  // Math library
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend ap_float<_W0, _E0> hls::fma(const ap_float<_W0, _E0> &lhs,
                                     const ap_float<_W0, _E0> &rhs,
                                     const ap_float<_W0, _E0> &add) noexcept;
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend ap_float<_W0, _E0> hls::sqrt(const ap_float<_W0, _E0> &val) noexcept;

  // Accumulator
  template<typename _T, typename _O, typename _I>
  friend class ap_float_acc;

#ifndef __SYNTHESIS__
  // Output stream printing
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend std::ostream &operator<<(std::ostream &os,
                                  const ap_float<_W0, _E0> &val) noexcept;
  // Input stream parsing
  template<int _W0, int _E0>
  AP_INLINE AP_NODEBUG
  friend std::istream &operator>>(std::istream &os,
                                  ap_float<_W0, _E0> &val) noexcept;
#endif
};

template<int _W, int _E>
AP_INLINE AP_NODEBUG
bool operator==(const ap_float<_W, _E> &lhs, const ap_float<_W, _E> &rhs) noexcept {
#ifdef __SYNTHESIS__
  return __fpga_float_compare_eq(&lhs, &rhs, _W, _E);
#else
  xip_fpo_t xlhs;
  xip_fpo_t xrhs;
  int res;
  lhs.init_xip_fpo_t(xlhs);
  rhs.init_xip_fpo_t(xrhs);

  lhs.to_xip_fpo_t(xlhs);
  rhs.to_xip_fpo_t(xrhs);
  xip_fpo_equal(&res, xlhs, xrhs);

  lhs.fini_xip_fpo_t(xlhs);
  rhs.fini_xip_fpo_t(xrhs);
  return res;
#endif
}

template<int _W, int _E>
AP_INLINE AP_NODEBUG
bool operator!=(const ap_float<_W, _E> &lhs, const ap_float<_W, _E> &rhs) noexcept {
#ifdef __SYNTHESIS__
  return __fpga_float_compare_ne(&lhs, &rhs, _W, _E);
#else
  xip_fpo_t xlhs;
  xip_fpo_t xrhs;
  int res;
  lhs.init_xip_fpo_t(xlhs);
  rhs.init_xip_fpo_t(xrhs);

  lhs.to_xip_fpo_t(xlhs);
  rhs.to_xip_fpo_t(xrhs);
  xip_fpo_notequal(&res, xlhs, xrhs);

  lhs.fini_xip_fpo_t(xlhs);
  rhs.fini_xip_fpo_t(xrhs);
  return res;
#endif
}

template<int _W, int _E>
AP_INLINE AP_NODEBUG
bool operator<(const ap_float<_W, _E> &lhs, const ap_float<_W, _E> &rhs) noexcept {
#ifdef __SYNTHESIS__
  return __fpga_float_compare_lt(&lhs, &rhs, _W, _E);
#else
  xip_fpo_t xlhs;
  xip_fpo_t xrhs;
  int res;
  lhs.init_xip_fpo_t(xlhs);
  rhs.init_xip_fpo_t(xrhs);

  lhs.to_xip_fpo_t(xlhs);
  rhs.to_xip_fpo_t(xrhs);
  xip_fpo_less(&res, xlhs, xrhs);

  lhs.fini_xip_fpo_t(xlhs);
  rhs.fini_xip_fpo_t(xrhs);
  return res;
#endif
}

template<int _W, int _E>
AP_INLINE AP_NODEBUG
bool operator>(const ap_float<_W, _E> &lhs, const ap_float<_W, _E> &rhs) noexcept {
  return rhs < lhs;
}

template<int _W, int _E>
AP_INLINE AP_NODEBUG
bool operator<=(const ap_float<_W, _E> &lhs, const ap_float<_W, _E> &rhs) noexcept {
#ifdef __SYNTHESIS__
  return __fpga_float_compare_le(&lhs, &rhs, _W, _E);
#else
  xip_fpo_t xlhs;
  xip_fpo_t xrhs;
  int res;
  lhs.init_xip_fpo_t(xlhs);
  rhs.init_xip_fpo_t(xrhs);

  lhs.to_xip_fpo_t(xlhs);
  rhs.to_xip_fpo_t(xrhs);
  xip_fpo_lessequal(&res, xlhs, xrhs);

  lhs.fini_xip_fpo_t(xlhs);
  rhs.fini_xip_fpo_t(xrhs);
  return res;
#endif
}

template<int _W, int _E>
AP_INLINE AP_NODEBUG
bool operator>=(const ap_float<_W, _E> &lhs, const ap_float<_W, _E> &rhs) noexcept {
  return rhs <= lhs;
}

// Expression unary plus
template<int _W, int _E>
AP_INLINE AP_NODEBUG
ap_float<_W, _E> operator+(ap_float<_W, _E> val) noexcept {
  return val;
}

// Expression unary minus
template<int _W, int _E>
AP_INLINE AP_NODEBUG
ap_float<_W, _E> operator-(ap_float<_W, _E> val) noexcept {
  val.sign_ref() = ~val.sign_ref();
  return val;
}

// Expression addition
template<int _W, int _E>
AP_INLINE AP_NODEBUG
ap_float<_W, _E> operator+(ap_float<_W, _E> lhs,
                           const ap_float<_W, _E> &rhs) noexcept {
  return lhs += rhs;
}

// Expression subtraction
template<int _W, int _E>
AP_INLINE AP_NODEBUG
ap_float<_W, _E> operator-(ap_float<_W, _E> lhs,
                           const ap_float<_W, _E> &rhs) noexcept {
  return lhs -= rhs;
}

// Expression multiplication
template<int _W, int _E>
AP_INLINE AP_NODEBUG
ap_float<_W, _E> operator*(ap_float<_W, _E> lhs,
                           const ap_float<_W, _E> &rhs) noexcept {
  return lhs *= rhs;
}

// Expression division
template<int _W, int _E>
AP_INLINE AP_NODEBUG
ap_float<_W, _E> operator/(ap_float<_W, _E> lhs,
                           const ap_float<_W, _E> &rhs) noexcept {
  return lhs /= rhs;
}

// Implicit conversions with native types (when at least one input is ap_float)
#define AP_FLOAT_COMPARE_WITH_NATIVE(_O, _T)                                   \
  template<int _W, int _E>                                                     \
  AP_INLINE AP_NODEBUG                                                         \
  bool operator _O(const ap_float<_W, _E> &lhs, const _T &rhs) noexcept {      \
    return operator _O(lhs, ap_float<_W, _E>(rhs));                            \
  }                                                                            \
  template<int _W, int _E>                                                     \
  AP_INLINE AP_NODEBUG                                                         \
  bool operator _O(const _T &lhs, const ap_float<_W, _E> &rhs) noexcept {      \
    return operator _O(ap_float<_W, _E>(lhs), rhs);                            \
  }
#define AP_FLOAT_BIN_OP_WITH_NATIVE(_O, _T)                                    \
  template<int _W, int _E>                                                     \
  AP_INLINE AP_NODEBUG                                                         \
  ap_float<_W, _E> operator _O(const ap_float<_W, _E> &lhs,                    \
                               const _T &rhs) noexcept {                       \
    return operator _O(lhs, ap_float<_W, _E>(rhs));                            \
  }                                                                            \
  template<int _W, int _E>                                                     \
  AP_INLINE AP_NODEBUG                                                         \
  ap_float<_W, _E> operator _O(const _T &lhs,                                  \
                               const ap_float<_W, _E> &rhs) noexcept {         \
    return operator _O(ap_float<_W, _E>(lhs), rhs);                            \
  }
#define AP_FLOAT_WITH_NATIVE(_T)                                               \
  AP_FLOAT_COMPARE_WITH_NATIVE(==, _T)                                         \
  AP_FLOAT_COMPARE_WITH_NATIVE(!=, _T)                                         \
  AP_FLOAT_COMPARE_WITH_NATIVE(<, _T)                                          \
  AP_FLOAT_COMPARE_WITH_NATIVE(>, _T)                                          \
  AP_FLOAT_COMPARE_WITH_NATIVE(<=, _T)                                         \
  AP_FLOAT_COMPARE_WITH_NATIVE(>=, _T)                                         \
  AP_FLOAT_BIN_OP_WITH_NATIVE(+, _T)                                           \
  AP_FLOAT_BIN_OP_WITH_NATIVE(-, _T)                                           \
  AP_FLOAT_BIN_OP_WITH_NATIVE(*, _T)                                           \
  AP_FLOAT_BIN_OP_WITH_NATIVE(/, _T)
AP_FLOAT_WITH_NATIVE(half)
AP_FLOAT_WITH_NATIVE(float)
AP_FLOAT_WITH_NATIVE(double)
AP_FLOAT_WITH_NATIVE(signed char)
AP_FLOAT_WITH_NATIVE(signed short)
AP_FLOAT_WITH_NATIVE(signed int)
AP_FLOAT_WITH_NATIVE(signed long)
AP_FLOAT_WITH_NATIVE(signed long long)
AP_FLOAT_WITH_NATIVE(unsigned char)
AP_FLOAT_WITH_NATIVE(unsigned short)
AP_FLOAT_WITH_NATIVE(unsigned int)
AP_FLOAT_WITH_NATIVE(unsigned long)
AP_FLOAT_WITH_NATIVE(unsigned long long)
#undef AP_FLOAT_COMPARE_WITH_NATIVE
#undef AP_FLOAT_BIN_OP_WITH_NATIVE
#undef AP_FLOAT_WITH_NATIVE

#ifndef __SYNTHESIS__
// Output stream printing
template<int _W, int _E>
AP_INLINE AP_NODEBUG
std::ostream &operator<<(std::ostream &os, const ap_float<_W, _E> &val) noexcept {
  double wr;
  wr = val.to_double(); // FIXME Raw double conversion display too many digits
  os << wr;
  return os;
}

// Input stream parsing
template<int _W, int _E>
AP_INLINE AP_NODEBUG
std::istream &operator>>(std::istream &os, ap_float<_W, _E> &val) noexcept {
  double rd;
  xip_fpo_t xval;
  
  os >> rd;

  val.init_xip_fpo_t(xval); // inizialize the xip_fpo_t
  xip_fpo_set_d(xval,rd);   // set xval to the double value from rd (rounding performed)
  val.from_xip_fpo_t(xval); // get double from xip -> double to ap_float (no rounding required)
  
  val.fini_xip_fpo_t(xval);
  return os;
}
#endif

namespace hls {
// Fused Multiply and Add
template<int _W, int _E>
AP_INLINE AP_NODEBUG
ap_float<_W, _E> fma(const ap_float<_W, _E> &lhs, const ap_float<_W, _E> &rhs,
                     const ap_float<_W, _E> &add) noexcept {
  ap_float<_W, _E> res;
#ifdef __SYNTHESIS__
  __fpga_float_fma(&res, &lhs, &rhs, &add, _W, _E);
#else
  xip_fpo_t xlhs;
  xip_fpo_t xrhs;
  xip_fpo_t xadd;
  xip_fpo_t xres;
  lhs.init_xip_fpo_t(xlhs);
  rhs.init_xip_fpo_t(xrhs);
  add.init_xip_fpo_t(xadd);
  res.init_xip_fpo_t(xres);

  lhs.to_xip_fpo_t(xlhs);
  rhs.to_xip_fpo_t(xrhs);
  add.to_xip_fpo_t(xadd);
  xip_fpo_fma(xres, xlhs, xrhs, xadd);
  res.from_xip_fpo_t(xres);

  lhs.fini_xip_fpo_t(xlhs);
  rhs.fini_xip_fpo_t(xrhs);
  add.fini_xip_fpo_t(xadd);
  res.fini_xip_fpo_t(xres);
#endif
  return res;
}

// Implicit conversions with native types (when at least one input is ap_float)
#define AP_FLOAT_FMA_WITH_NATIVE(_T)                                           \
  template<int _W, int _E>                                                     \
  AP_INLINE AP_NODEBUG                                                         \
  ap_float<_W, _E> fma(const _T &lhs, const ap_float<_W, _E> &rhs,             \
                       const ap_float<_W, _E> &add) noexcept {                 \
    return fma(ap_float<_W, _E>(lhs), rhs, add);                               \
  }                                                                            \
  template<int _W, int _E>                                                     \
  AP_INLINE AP_NODEBUG                                                         \
  ap_float<_W, _E> fma(const ap_float<_W, _E> &lhs, const _T &rhs,             \
                       const ap_float<_W, _E> &add) noexcept {                 \
    return fma(lhs, ap_float<_W, _E>(rhs), add);                               \
  }                                                                            \
  template<int _W, int _E>                                                     \
  AP_INLINE AP_NODEBUG                                                         \
  ap_float<_W, _E> fma(const ap_float<_W, _E> &lhs,                            \
                       const ap_float<_W, _E> &rhs, const _T &add) noexcept {  \
    return fma(lhs, rhs, ap_float<_W, _E>(add));                               \
  }                                                                            \
  template<int _W, int _E>                                                     \
  AP_INLINE AP_NODEBUG                                                         \
  ap_float<_W, _E> fma(const _T &lhs, const _T &rhs,                           \
                       const ap_float<_W, _E> &add) noexcept {                 \
    return fma(ap_float<_W, _E>(lhs), ap_float<_W, _E>(rhs), add);             \
  }                                                                            \
  template<int _W, int _E>                                                     \
  AP_INLINE AP_NODEBUG                                                         \
  ap_float<_W, _E> fma(const _T &lhs, const ap_float<_W, _E> &rhs,             \
                       const _T &add) noexcept {                               \
    return fma(ap_float<_W, _E>(lhs), rhs, ap_float<_W, _E>(add));             \
  }                                                                            \
  template<int _W, int _E>                                                     \
  AP_INLINE AP_NODEBUG                                                         \
  ap_float<_W, _E> fma(const ap_float<_W, _E> &lhs, const _T &rhs,             \
                       const _T &add) noexcept {                               \
    return fma(lhs, ap_float<_W, _E>(rhs), ap_float<_W, _E>(add));             \
  }
AP_FLOAT_FMA_WITH_NATIVE(half)
AP_FLOAT_FMA_WITH_NATIVE(float)
AP_FLOAT_FMA_WITH_NATIVE(double)
AP_FLOAT_FMA_WITH_NATIVE(signed char)
AP_FLOAT_FMA_WITH_NATIVE(signed short)
AP_FLOAT_FMA_WITH_NATIVE(signed int)
AP_FLOAT_FMA_WITH_NATIVE(signed long)
AP_FLOAT_FMA_WITH_NATIVE(signed long long)
AP_FLOAT_FMA_WITH_NATIVE(unsigned char)
AP_FLOAT_FMA_WITH_NATIVE(unsigned short)
AP_FLOAT_FMA_WITH_NATIVE(unsigned int)
AP_FLOAT_FMA_WITH_NATIVE(unsigned long)
AP_FLOAT_FMA_WITH_NATIVE(unsigned long long)
#undef AP_FLOAT_FMA_WITH_NATIVE

// Square Root
template<int _W, int _E>
AP_INLINE AP_NODEBUG
ap_float<_W, _E> sqrt(const ap_float<_W, _E> &val) noexcept {
  ap_float<_W, _E> res;
#ifdef __SYNTHESIS__
  __fpga_float_sqrt(&res, &val, _W, _E);
#else
  xip_fpo_t xval;
  xip_fpo_t xres;
  val.init_xip_fpo_t(xval);
  res.init_xip_fpo_t(xres);

  val.to_xip_fpo_t(xval);
  xip_fpo_sqrt(xres, xval);
  res.from_xip_fpo_t(xres);

  val.fini_xip_fpo_t(xval);
  res.fini_xip_fpo_t(xres);
#endif
  return res;
}

// Absolute value
template<int _W, int _E>
AP_INLINE AP_NODEBUG
ap_float<_W, _E> abs(ap_float<_W, _E> val) noexcept {
  val.sign_ref() = 0;
  return val;
}
} // namespace hls

namespace std {
template<int _W, int _E>
struct numeric_limits<ap_float<_W, _E>> {
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = true;
  static constexpr bool is_integer = false;
  static constexpr bool is_exact = false;

  static constexpr bool has_infinity = true;
  static constexpr bool has_quiet_NaN = true;
  static constexpr bool has_signaling_NaN = false;

  static constexpr float_denorm_style has_denorm = denorm_present;
  static constexpr bool has_denorm_loss = true;
  static constexpr float_round_style round_style = round_to_nearest;

  static constexpr bool is_iec559 = has_infinity && has_quiet_NaN &&
                                    has_signaling_NaN;

  static constexpr bool is_bounded = true;
  static constexpr bool is_modulo = false;

  static constexpr int digits = _W - _E;
  // log10(2) = 0.301029995663981  (D10317)
  static constexpr int digits10 = static_cast<int>((digits - 1) * 0.301029995663981);
  static constexpr int max_digits10 = static_cast<int>(digits * 0.301029995663981 + 2);
  
  static constexpr int radix = 2;
  static constexpr int min_exponent =  3 - (1 << (_E - 1));
  static constexpr int min_exponent10 = 0; // FIXME
  static constexpr int max_exponent = (1 << (_E - 1));
  static constexpr int max_exponent10 = 0; // FIXME

  static constexpr bool traps = false;
  static constexpr bool tinyness_before = false;

  AP_INLINE AP_NODEBUG
  static constexpr ap_float<_W, _E> lowest() noexcept {
    return ap_float<_W, _E>::lowest();
  }

  AP_INLINE AP_NODEBUG
  static constexpr ap_float<_W, _E> denorm_min() noexcept {
    return ap_float<_W, _E>::min_denorm();
  }

  AP_INLINE AP_NODEBUG
  static constexpr ap_float<_W, _E> min() noexcept {
    return ap_float<_W, _E>::min();
  }

  AP_INLINE AP_NODEBUG
  static constexpr ap_float<_W, _E> epsilon() noexcept {
    return ap_float<_W, _E>::epsilon();
  }

  AP_INLINE AP_NODEBUG
  static constexpr ap_float<_W, _E> round_error() noexcept {
    return ap_float<_W, _E>::one_half();
  }

  AP_INLINE AP_NODEBUG
  static constexpr ap_float<_W, _E> max() noexcept {
    return ap_float<_W, _E>::max();
  }

  AP_INLINE AP_NODEBUG
  static constexpr ap_float<_W, _E> infinity() noexcept {
    return ap_float<_W, _E>::infinity();
  }

  AP_INLINE AP_NODEBUG
  static constexpr ap_float<_W, _E> quiet_NaN() noexcept {
    return ap_float<_W, _E>::quiet_NaN();
  }

  AP_INLINE AP_NODEBUG
  static constexpr ap_float<_W, _E> signaling_NaN() noexcept {
    return ap_float<_W, _E>::signaling_NaN();
  }
};
} // namespace std

/** Arbitrary Precision Floating-Point Class
 *   - _T: ap_float<_W, _E> used for input values as well as output/return value
 *   - _O: ap_fixed<_OW, _OI> used for internal accumulator as well as output for fixed-to-float conversion
 *   - _I: ap_fixed<_IW, _II> used for input float-to-fixed conversion
 */
template<typename _T, typename _O, typename _I = _O>
class ap_float_acc {
  _T accumulate(const _T &v, bool last);
};

template<int _W, int _E, int _OW, int _OI, int _IW, int _II>
class ap_float_acc<ap_float<_W, _E>, ap_fixed<_OW, _OI>, ap_fixed<_IW, _II>> {
  static constexpr int _M = _W - _E; // note that this includes the "hidden" bit
  static_assert(_IW - _II == _OW - _OI, "ap_float_acc output LSB must be aligned "
                                        "with the input LSB");
  static_assert(_OI >= _II, "ap_float_acc output MSB must be greater or equal to "
                            "the input MSB");
  static_assert(_OI <= _II + 54, "ap_float_acc output MSB must be no greater "
                                 "than the input MSB + 54");

#ifdef __SYNTHESIS__
  ap_fixed<_OW, _OI> sum;
#else
  xil_fpo_accum_state *state = nullptr;
  bool last;
#endif

public:
#ifdef __SYNTHESIS__
  ap_float_acc() = default;
  ~ap_float_acc() = default;
#else
  ap_float_acc() {
    state = xip_fpo_accum_create_state(_E, _M, _OI-1, _II-1, _OI-_OW);

    last = true;
    xip_fpo_accum_reset_state(state);
  }

  ~ap_float_acc() {
    assert(last && "ap_float_acc destroyed before last accumulation");

    xip_fpo_accum_destroy_state(state);
    state = nullptr;
  }
#endif

  // No copy/assignment allowed
  ap_float_acc(ap_float_acc &&) = delete;
  ap_float_acc(const ap_float_acc &) = delete;
  ap_float_acc& operator=(ap_float_acc &&) = delete;
  ap_float_acc& operator=(const ap_float_acc &) = delete;

  // Public API
  AP_INLINE AP_NODEBUG
  ap_float<_W, _E> accumulate(const ap_float<_W, _E> &val, bool last) noexcept {
    ap_float<_W, _E> res;
#ifdef __SYNTHESIS__
    __fpga_float_accumulate(&res, &sum, &val, last, _W, _E, _OW, _OI, _II);
#else
    xip_fpo_t xval;
    xip_fpo_t xres;
    val.init_xip_fpo_t(xval);
    res.init_xip_fpo_t(xres);

    val.to_xip_fpo_t(xval);
    xip_fpo_accum_sample(xres, xval, /*subtract=*/false, state);
    res.from_xip_fpo_t(xres);

    val.fini_xip_fpo_t(xval);
    res.fini_xip_fpo_t(xres);

    this->last = last;
    if (last)
      xip_fpo_accum_reset_state(state);
#endif
    return res;
  }
};

#ifndef AP_FLOAT_SLOW_BF16
#include <etc/ap_float_bf16.h>
#endif

#endif
