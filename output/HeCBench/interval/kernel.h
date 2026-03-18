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
/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Example of program using the interval_gpu<T> template class and operators:
 * Search for roots of a function using an interval Newton method.
  *
 * 0: the first implementation
 * 1: the second implementation
 *
 */

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <chrono>



// --- from cpu_interval.h ---
/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Simple CPU implementation
*  Depends on Boost.Interval
*/

#ifndef CPU_INTERVAL_H
#define CPU_INTERVAL_H

#ifndef __USE_ISOC99
#define __USE_ISOC99
#endif

#include <iostream>
#include <vector>
#include <boost/numeric/interval.hpp>
//#include <iomanip>

#define UNPROTECTED 0
#define USE_RECURSION_CPU 1

using boost::numeric::interval;
using namespace boost::numeric;

template <class T, int N, int THREADS>
class global_stack_cpu {
 private:
  T *buf;
  int free_index;

 public:
  // buf should point to an allocated global buffer of size N * THREADS *
  // sizeof(T)
  global_stack_cpu(T *buf, int thread_id) : buf(buf), free_index(thread_id) {}

  void push(T const &v) {
    buf[free_index] = v;
    free_index += THREADS;
  }
  T pop() {
    free_index -= THREADS;
    return buf[free_index];
  }
  bool full() { return free_index >= N * THREADS; }
  bool empty() { return free_index < THREADS; }
  int size() { return free_index / THREADS; }
};

// The function F of which we want to find roots, defined on intervals
// Should typically depend on thread_id (indexing an array of coefficients...)
template <class I>
I f_cpu(I const &x, int thread_id) {
  typedef typename I::base_type T;
  T alpha = -T(thread_id) / T(THREADS);
  return square(x - I(1)) + I(alpha) * x;
}

// First derivative of F, also defined on intervals
template <class I>
I fd_cpu(I const &x, int thread_id) {
  typedef typename I::base_type T;
  T alpha = -T(thread_id) / T(THREADS);
  return I(2) * x + I(alpha - 2);
}

// Is this interval small enough to stop iterating?
template <class I>
bool is_minimal_cpu(I const &x, int thread_id) {
  typedef typename I::base_type T;
  T const epsilon_x = 1e-6f;
  T const epsilon_y = 1e-6f;
  return !empty(x) && (width(x) <= epsilon_x * abs(median(x)) ||
                       width(f_cpu(x, thread_id)) <= epsilon_y);
}

// In some cases, Newton iterations converge slowly.
// Bisecting the interval accelerates convergence.
template <class I>
bool should_bisect_cpu(I const &x, I const &x1, I const &x2,
                       typename I::base_type alpha) {
  typedef typename I::base_type T;
  T wmax = alpha * width(x);
  return width(x1) > wmax || width(x2) > wmax;
}

int const DEPTH_WORK = 128;

// Main interval Newton loop.
// Keep refining a list of intervals stored in a stack.
// Always keep the next interval to work on in registers (avoids excessive
// spilling to local mem)
template <class I, int THREADS, int DEPTH_RESULT>
void newton_interval_cpu(global_stack_cpu<I, DEPTH_RESULT, THREADS> &result,
                         I const &ix0, int thread_id) {
  typedef typename I::base_type T;

  T const alpha = .99f;  // Threshold before switching to bisection

  // Intervals to be processed
  I local_buffer[DEPTH_WORK];
  global_stack_cpu<I, DEPTH_WORK, 1> work(local_buffer, 0);

  // We start with the whole domain
  I ix = ix0;

  while (true) {
    // Compute (x - F({x})/F'(ix)) inter ix
    // -> may yield 0, 1 or 2 intervals
    T x = median(ix);
    I iq = f_cpu(I(x), thread_id);
    I id = fd_cpu(ix, thread_id);

    bool has_part2;
    I part1, part2;
    part1 = division_part1(iq, id, has_part2);
    part1 = intersect(I(x) - part1, ix);

    if (has_part2) {
      part2 = division_part2(iq, id);
      part2 = intersect(I(x) - part2, ix);
    }

    // Do we have small-enough intervals?
    if (is_minimal_cpu(part1, thread_id)) {
      result.push(part1);
      part1 = I::empty();
    }

    if (has_part2 && is_minimal_cpu(part2, thread_id)) {
      result.push(part2);
      part2 = I::empty();
    }

    if (should_bisect_cpu(ix, part1, part2, alpha)) {
      // Not so good improvement
      // Switch to bisection method for this step
      part1 = I(ix.lower(), x);
      part2 = I(x, ix.upper());
      has_part2 = true;
    }

    if ((part1.lower() <= part1.upper()) && !empty(part1)) {
      // At least 1 solution
      // We will compute part1 next
      ix = part1;

      if (has_part2 && !empty(part2)) {
        // 2 solutions
        // Save the second solution for later
        work.push(part2);
      }
    } else if (has_part2 && !empty(part2)) {
      // 1 solution
      // Work on that next
      ix = part2;
    } else {
      // No solution
      // Do we still have work to do in the stack?
      if (work.empty())  // If not, we are done
        break;
      else
        ix = work.pop();  // Otherwise, pick an interval to work on
    }
  }
}

template <class I, int THREADS, int DEPTH_RESULT>
void newton_interval_rec_cpu(global_stack_cpu<I, DEPTH_RESULT, THREADS> &result,
                             I const &ix, int thread_id) {
  typedef typename I::base_type T;
  T const alpha = .99f;  // Threshold before switching to bisection

  if (is_minimal_cpu(ix, thread_id)) {
    result.push(ix);
    return;
  }

  // Compute (x - F({x})/F'(ix)) inter ix
  // -> may yield 0, 1 or 2 intervals
  T x = median(ix);
  I iq = f_cpu(I(x), thread_id);
  I id = fd_cpu(ix, thread_id);

  bool has_part2;
  I part1, part2;
  part1 = division_part1(iq, id, has_part2);
  part1 = intersect(I(x) - part1, ix);

  if (has_part2) {
    part2 = division_part2(iq, id);
    part2 = intersect(I(x) - part2, ix);
  }

  if (should_bisect_cpu(ix, part1, part2, alpha)) {
    // Not so good improvement
    // Switch to bisection method for this step
    part1 = I(ix.lower(), x);
    part2 = I(x, ix.upper());
    has_part2 = true;
  }

  if ((part1.lower() <= part1.upper()) && (!empty(part1))) {
    newton_interval_rec_cpu<I, THREADS, DEPTH_RESULT>(result, part1, thread_id);
  }

  if (has_part2 && !empty(part2)) {
    newton_interval_rec_cpu<I, THREADS, DEPTH_RESULT>(result, part2, thread_id);
  }
}

template <class I>
void test_interval_newton_cpu(I *buffer, int *nresults, I i) {
  typedef typename I::base_type T;

  // Intervals to return
  // std::vector<I> local_buffer(BLOCK_SIZE * GRID_SIZE * DEPTH_WORK);
  for (int thread_id = 0; thread_id != BLOCK_SIZE * GRID_SIZE; ++thread_id) {
    global_stack_cpu<I, DEPTH_RESULT, THREADS> result(buffer, thread_id);

#if USE_RECURSION_CPU
    newton_interval_rec_cpu<I, THREADS>(result, i, thread_id);
#else
    newton_interval_cpu<I, THREADS>(result, i, thread_id);
#endif
    nresults[thread_id] = result.size();
  }
}

typedef interval<T, interval_lib::policies<interval_lib::rounded_math<T>,
                                           interval_lib::checking_base<T> > >
    Ibase;

#if UNPROTECTED
typedef interval_lib::unprotect<Ibase>::type I_CPU;
Ibase::traits_type::rounding rnd;
#else
typedef Ibase I_CPU;
#endif

bool checkAgainstHost(int *h_nresults, int *h_nresults_cpu, I_CPU *h_result,
                      I_CPU *h_result_cpu) {
  std::cout << "\nCheck against Host computation...\n\n";
  int success = 1;
  int success1 = 1;
  int success2 = 1;

  if (h_nresults_cpu[0] == h_nresults[0]) {
    for (int i = 0; i != h_nresults[0]; ++i) {
      TYPE diff1 = abs(h_result[THREADS * i + 0].lower() -
                       h_result_cpu[THREADS * i + 0].lower());
      TYPE diff2 = abs(h_result[THREADS * i + 0].upper() -
                       h_result_cpu[THREADS * i + 0].upper());

      if ((diff1 > 1.0e-6f) || (diff2 > 1.0e-6f)) {
        success1 = 0;
        break;
      }
    }

    // in case the two intervals are reversed
    for (int i = 0; i != h_nresults[0]; ++i) {
      TYPE diff1 =
          abs(h_result[THREADS * i + 0].lower() -
              h_result_cpu[THREADS * (h_nresults[0] - i - 1) + 0].lower());
      TYPE diff2 =
          abs(h_result[THREADS * i + 0].upper() -
              h_result_cpu[THREADS * (h_nresults[0] - i - 1) + 0].upper());

      if ((diff1 > 1.0e-6f) || (diff2 > 1.0e-6f)) {
        success2 = 0;
        break;
      }
    }

    success = success1 || success2;
  } else
    success = 0;

  return (bool)success;
}

#endif


// --- from cuda_interval_lib.h ---
/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CUDA_INTERVAL_LIB_H
#define CUDA_INTERVAL_LIB_H

// Interval template class and basic operations
// Interface inspired from the Boost Interval library (www.boost.org)

template <class T>
class interval_gpu {
 public:
  interval_gpu();
  interval_gpu(T const &v);
  interval_gpu(T const &l, T const &u);

  T const &lower() const;
  T const &upper() const;

  static interval_gpu empty();

 private:
  T low;
  T up;
};

// Constructors
template <class T>
inline interval_gpu<T>::interval_gpu() {}

template <class T>
inline interval_gpu<T>::interval_gpu(T const &l, T const &u)
    : low(l), up(u) {}

template <class T>
inline interval_gpu<T>::interval_gpu(T const &v)
    : low(v), up(v) {}

template <class T>
inline T const &interval_gpu<T>::lower() const {
  return low;
}

template <class T>
inline T const &interval_gpu<T>::upper() const {
  return up;
}

template <class T>
inline interval_gpu<T> interval_gpu<T>::empty() {
  rounded_arith<T> rnd;
  return interval_gpu<T>(rnd.nan(), rnd.nan());
}

template <class T>
inline bool empty(interval_gpu<T> x) {
  T hash = x.lower() + x.upper();
  return (hash != hash);
}

template <class T>
inline T width(interval_gpu<T> x) {
  if (empty(x)) return 0;

  rounded_arith<T> rnd;
  return rnd.sub_up(x.upper(), x.lower());
}

// Arithmetic operations

// Unary operators
template <class T>
inline interval_gpu<T> const &operator+(interval_gpu<T> const &x) {
  return x;
}

template <class T>
inline interval_gpu<T> operator-(interval_gpu<T> const &x) {
  return interval_gpu<T>(-x.upper(), -x.lower());
}

// Binary operators
template <class T>
inline interval_gpu<T> operator+(interval_gpu<T> const &x,
                                            interval_gpu<T> const &y) {
  rounded_arith<T> rnd;
  return interval_gpu<T>(rnd.add_down(x.lower(), y.lower()),
                         rnd.add_up(x.upper(), y.upper()));
}

template <class T>
inline interval_gpu<T> operator-(interval_gpu<T> const &x,
                                            interval_gpu<T> const &y) {
  rounded_arith<T> rnd;
  return interval_gpu<T>(rnd.sub_down(x.lower(), y.upper()),
                         rnd.sub_up(x.upper(), y.lower()));
}

inline float min4(float a, float b, float c, float d) {
  return fminf(fminf(a, b), fminf(c, d));
}

inline float max4(float a, float b, float c, float d) {
  return fmaxf(fmaxf(a, b), fmaxf(c, d));
}

inline double min4(double a, double b, double c, double d) {
  return fmin(fmin(a, b), fmin(c, d));
}

inline double max4(double a, double b, double c, double d) {
  return fmax(fmax(a, b), fmax(c, d));
}

template <class T>
inline interval_gpu<T> operator*(interval_gpu<T> const &x,
                                            interval_gpu<T> const &y) {
  // Textbook implementation: 14 flops, but no branch.
  rounded_arith<T> rnd;
  return interval_gpu<T>(
      min4(rnd.mul_down(x.lower(), y.lower()),
           rnd.mul_down(x.lower(), y.upper()),
           rnd.mul_down(x.upper(), y.lower()),
           rnd.mul_down(x.upper(), y.upper())),
      max4(rnd.mul_up(x.lower(), y.lower()), rnd.mul_up(x.lower(), y.upper()),
           rnd.mul_up(x.upper(), y.lower()), rnd.mul_up(x.upper(), y.upper())));
}

// Center of an interval
// Typically used for bisection
template <class T>
inline T median(interval_gpu<T> const &x) {
  rounded_arith<T> rnd;
  return rnd.median(x.lower(), x.upper());
}

// Intersection between two intervals (can be empty)
template <class T>
inline interval_gpu<T> intersect(interval_gpu<T> const &x,
                                            interval_gpu<T> const &y) {
  rounded_arith<T> rnd;
  T const &l = rnd.max(x.lower(), y.lower());
  T const &u = rnd.min(x.upper(), y.upper());

  if (l <= u)
    return interval_gpu<T>(l, u);
  else
    return interval_gpu<T>::empty();
}

// Division by an interval which does not contain 0.
// GPU-optimized implementation assuming division is expensive
template <class T>
inline interval_gpu<T> div_non_zero(interval_gpu<T> const &x,
                                               interval_gpu<T> const &y) {
  rounded_arith<T> rnd;
  typedef interval_gpu<T> I;
  T xl, yl, xu, yu;

  if (y.upper() < 0) {
    xl = x.upper();
    xu = x.lower();
  } else {
    xl = x.lower();
    xu = x.upper();
  }

  if (x.upper() < 0) {
    yl = y.lower();
    yu = y.upper();
  } else if (x.lower() < 0) {
    if (y.upper() < 0) {
      yl = y.upper();
      yu = y.upper();
    } else {
      yl = y.lower();
      yu = y.lower();
    }
  } else {
    yl = y.upper();
    yu = y.lower();
  }

  return I(rnd.div_down(xl, yl), rnd.div_up(xu, yu));
}

template <class T>
inline interval_gpu<T> div_positive(interval_gpu<T> const &x,
                                               T const &yu) {
  // assert(yu > 0);
  if (x.lower() == 0 && x.upper() == 0) return x;

  rounded_arith<T> rnd;
  typedef interval_gpu<T> I;
  const T &xl = x.lower();
  const T &xu = x.upper();

  if (xu < 0)
    return I(rnd.neg_inf(), rnd.div_up(xu, yu));
  else if (xl < 0)
    return I(rnd.neg_inf(), rnd.pos_inf());
  else
    return I(rnd.div_down(xl, yu), rnd.pos_inf());
}

template <class T>
inline interval_gpu<T> div_negative(interval_gpu<T> const &x,
                                               T const &yl) {
  // assert(yu > 0);
  if (x.lower() == 0 && x.upper() == 0) return x;

  rounded_arith<T> rnd;
  typedef interval_gpu<T> I;
  const T &xl = x.lower();
  const T &xu = x.upper();

  if (xu < 0)
    return I(rnd.div_down(xu, yl), rnd.pos_inf());
  else if (xl < 0)
    return I(rnd.neg_inf(), rnd.pos_inf());
  else
    return I(rnd.neg_inf(), rnd.div_up(xl, yl));
}

template <class T>
inline interval_gpu<T> div_zero_part1(interval_gpu<T> const &x,
                                                 interval_gpu<T> const &y,
                                                 bool &b) {
  if (x.lower() == 0 && x.upper() == 0) {
    b = false;
    return x;
  }

  rounded_arith<T> rnd;
  typedef interval_gpu<T> I;
  const T &xl = x.lower();
  const T &xu = x.upper();
  const T &yl = y.lower();
  const T &yu = y.upper();

  if (xu < 0) {
    b = true;
    return I(rnd.neg_inf(), rnd.div_up(xu, yu));
  } else if (xl < 0) {
    b = false;
    return I(rnd.neg_inf(), rnd.pos_inf());
  } else {
    b = true;
    return I(rnd.neg_inf(), rnd.div_up(xl, yl));
  }
}

template <class T>
inline interval_gpu<T> div_zero_part2(interval_gpu<T> const &x,
                                                 interval_gpu<T> const &y) {
  rounded_arith<T> rnd;
  typedef interval_gpu<T> I;
  const T &xl = x.lower();
  const T &xu = x.upper();
  const T &yl = y.lower();
  const T &yu = y.upper();

  if (xu < 0)
    return I(rnd.div_down(xu, yl), rnd.pos_inf());
  else
    return I(rnd.div_down(xl, yu), rnd.pos_inf());
}

template <class T>
inline interval_gpu<T> division_part1(interval_gpu<T> const &x,
                                                 interval_gpu<T> const &y,
                                                 bool &b) {
  b = false;

  if (y.lower() <= 0 && y.upper() >= 0)
    if (y.lower() != 0)
      if (y.upper() != 0)
        return div_zero_part1(x, y, b);
      else
        return div_negative(x, y.lower());
    else if (y.upper() != 0)
      return div_positive(x, y.upper());
    else
      return interval_gpu<T>::empty();
  else
    return div_non_zero(x, y);
}

template <class T>
inline interval_gpu<T> division_part2(interval_gpu<T> const &x,
                                                 interval_gpu<T> const &y,
                                                 bool b = true) {
  if (!b) return interval_gpu<T>::empty();

  return div_zero_part2(x, y);
}

template <class T>
inline interval_gpu<T> square(interval_gpu<T> const &x) {
  typedef interval_gpu<T> I;
  rounded_arith<T> rnd;
  const T &xl = x.lower();
  const T &xu = x.upper();

  if (xl >= 0)
    return I(rnd.mul_down(xl, xl), rnd.mul_up(xu, xu));
  else if (xu <= 0)
    return I(rnd.mul_down(xu, xu), rnd.mul_up(xl, xl));
  else
    return I(static_cast<T>(0),
             rnd.max(rnd.mul_up(xl, xl), rnd.mul_up(xu, xu)));
}

#endif


// --- from cuda_interval_rounded_arith.h ---
/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Type-specific implementation of rounded arithmetic operators.
// Thin layer over the CUDA intrinsics.

#ifndef CUDA_INTERVAL_ROUNDED_ARITH_H
#define CUDA_INTERVAL_ROUNDED_ARITH_H

// Generic class, no actual implementation yet
template <class T>
struct rounded_arith {
  T add_down(const T &x, const T &y);
  T add_up(const T &x, const T &y);
  T sub_down(const T &x, const T &y);
  T sub_up(const T &x, const T &y);
  T mul_down(const T &x, const T &y);
  T mul_up(const T &x, const T &y);
  T div_down(const T &x, const T &y);
  T div_up(const T &x, const T &y);
  T median(const T &x, const T &y);
  T sqrt_down(const T &x);
  T sqrt_up(const T &x);
  T int_down(const T &x);
  T int_up(const T &x);

  T pos_inf();
  T neg_inf();
  T nan();
  T min(T const &x, T const &y);
  T max(T const &x, T const &y);
};

// Specialization for float
template <>
struct rounded_arith<float> {
  float add_down(const float &x, const float &y) {
    //return __fadd_rd(x, y);
    return x+y;
  }

  float add_up(const float &x, const float &y) {
    //return __fadd_ru(x, y);
    return x+y;
  }

  float sub_down(const float &x, const float &y) {
    //return __fadd_rd(x, -y);
    return x-y;
  }

  float sub_up(const float &x, const float &y) {
    //return __fadd_ru(x, -y);
    return x-y;
  }

  float mul_down(const float &x, const float &y) {
    //return __fmul_rd(x, y);
    return x*y;
  }

  float mul_up(const float &x, const float &y) {
    //return __fmul_ru(x, y);
    return x*y;
  }

  float div_down(const float &x, const float &y) {
    //return __fdiv_rd(x, y);
    return x/y;
  }

  float div_up(const float &x, const float &y) {
    //return __fdiv_ru(x, y);
    return x/y;
  }

  float median(const float &x, const float &y) {
    return (x + y) * .5f;
  }

  //float sqrt_down(const float &x) { return __fsqrt_rd(x); }
  float sqrt_down(const float &x) { return sqrtf(x); }

  //float sqrt_up(const float &x) { return __fsqrt_ru(x); }
  float sqrt_up(const float &x) { return sqrtf(x); }

  float int_down(const float &x) { return floorf(x); }

  float int_up(const float &x) { return ceilf(x); }

  float neg_inf() { return __int_as_float(0xff800000); }

  float pos_inf() { return __int_as_float(0x7f800000); }

  float nan() { return nanf(""); }

  float min(float const &x, float const &y) { return fminf(x, y); }

  float max(float const &x, float const &y) { return fmaxf(x, y); }
};

// Specialization for double
template <>
struct rounded_arith<double> {
  double add_down(const double &x, const double &y) {
    //return __dadd_rd(x, y);
    return x+y;
  }

  double add_up(const double &x, const double &y) {
    //return __dadd_ru(x, y);
    return x+y;
  }

  double sub_down(const double &x, const double &y) {
    //return __dadd_rd(x, -y);
    return x-y;
  }

  double sub_up(const double &x, const double &y) {
    //return __dadd_ru(x, -y);
    return x-y;
  }

  double mul_down(const double &x, const double &y) {
    //return __dmul_rd(x, y);
    return x*y;
  }

  double mul_up(const double &x, const double &y) {
    //return __dmul_ru(x, y);
    return x*y;
  }

  double div_down(const double &x, const double &y) {
    //return __ddiv_rd(x, y);
    return x/y;
  }

  double div_up(const double &x, const double &y) {
    //return __ddiv_ru(x, y);
    return x/y;
  }
  double median(const double &x, const double &y) {
    return (x + y) * .5;
  }

  //double sqrt_down(const double &x) { return __dsqrt_rd(x); }
  double sqrt_down(const double &x) { return sqrt(x); }

  //double sqrt_up(const double &x) { return __dsqrt_ru(x); }
  double sqrt_up(const double &x) { return sqrt(x); }

  double int_down(const double &x) { return floor(x); }

  double int_up(const double &x) { return ceil(x); }

  double neg_inf() {
    return __longlong_as_double(0xfff0000000000000ull);
  }

  double pos_inf() {
    return __longlong_as_double(0x7ff0000000000000ull);
  }
  double nan() { return ::nan(""); }

  double min(double const &x, double const &y) { return fmin(x, y); }

  double max(double const &x, double const &y) { return fmax(x, y); }
};

#endif


// --- from gpu_interval.h ---
/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CUDA_INTERVAL_H
#define CUDA_INTERVAL_H


// Stack in local memory. Managed independently for each thread.
template <class T, int N>
class local_stack {
 private:
  T buf[N];
  int tos;

 public:
  local_stack() : tos(-1) {}
  T const &top() const { return buf[tos]; }
  T &top() { return buf[tos]; }
  void push(T const &v) { buf[++tos] = v; }
  T pop() { return buf[tos--]; }
  bool full() { return tos == (N - 1); }
  bool empty() { return tos == -1; }
};

// Stacks in global memory.
// Same function as local_stack, but accessible from the host.
// Interleaved between threads by blocks of THREADS elements.
// Independent stack for each thread, no sharing of data between threads.
template <class T, int N, int THREADS>
class global_stack {
 private:
  T *buf;
  int free_index;

 public:
  // buf should point to an allocated global buffer of
  // size N * THREADS * sizeof(T)
  global_stack(T *buf, int thread_id)
      : buf(buf), free_index(thread_id) {}

  void push(T const &v) {
    buf[free_index] = v;
    free_index += THREADS;
  }
  T pop() {
    free_index -= THREADS;
    return buf[free_index];
  }
  bool full() { return free_index >= N * THREADS; }
  bool empty() { return free_index < THREADS; }
  int size() { return free_index / THREADS; }
};

// The function F of which we want to find roots, defined on intervals
// Should typically depend on thread_id (indexing an array of coefficients...)
template <class T>
interval_gpu<T> f(interval_gpu<T> const &x, int thread_id) {
  typedef interval_gpu<T> I;
  T alpha = -T(thread_id) / T(THREADS);
  return square(x - I(1)) + I(alpha) * x;
}

// First derivative of F, also defined on intervals
template <class T>
interval_gpu<T> fd(interval_gpu<T> const &x, int thread_id) {
  typedef interval_gpu<T> I;
  T alpha = -T(thread_id) / T(THREADS);
  return I(2) * x + I(alpha - 2);
}

// Is this interval small enough to stop iterating?
template <class T>
bool is_minimal(interval_gpu<T> const &x, int thread_id) {
  T const epsilon_x = 1e-6f;
  T const epsilon_y = 1e-6f;
  return !empty(x) && (width(x) <= epsilon_x * abs(median(x)) ||
                       width(f(x, thread_id)) <= epsilon_y);
}

// In some cases, Newton iterations converge slowly.
// Bisecting the interval accelerates convergence.
template <class T>
bool should_bisect(interval_gpu<T> const &x,
                              interval_gpu<T> const &x1,
                              interval_gpu<T> const &x2, T alpha) {
  T wmax = alpha * width(x);
  return (!empty(x1) && width(x1) > wmax) || (!empty(x2) && width(x2) > wmax);
}

// Main interval Newton loop.
// Keep refining a list of intervals stored in a stack.
// Always keep the next interval to work on in registers
// (avoids excessive spilling to local mem)
template <class T, int THREADS, int DEPTH_RESULT>
void newton_interval(
    global_stack<interval_gpu<T>, DEPTH_RESULT, THREADS> &result,
    interval_gpu<T> const &ix0, int thread_id) {
  typedef interval_gpu<T> I;
  int const DEPTH_WORK = 128;

  T const alpha = .99f;  // Threshold before switching to bisection

  // Intervals to be processed
  local_stack<I, DEPTH_WORK> work;

  // We start with the whole domain
  I ix = ix0;

  while (true) {
    // Compute (x - F({x})/F'(ix)) inter ix
    // -> may yield 0, 1 or 2 intervals
    T x = median(ix);
    I iq = f(I(x), thread_id);
    I id = fd(ix, thread_id);

    bool has_part2;
    I part1, part2 = I::empty();
    part1 = division_part1(iq, id, has_part2);
    part1 = intersect(I(x) - part1, ix);

    if (has_part2) {
      part2 = division_part2(iq, id);
      part2 = intersect(I(x) - part2, ix);
    }

    // Do we have small-enough intervals?
    if (is_minimal(part1, thread_id)) {
      result.push(part1);
      part1 = I::empty();
    }

    if (has_part2 && is_minimal(part2, thread_id)) {
      result.push(part2);
      part2 = I::empty();
    }

    if (should_bisect(ix, part1, part2, alpha)) {
      // Not so good improvement
      // Switch to bisection method for this step
      part1 = I(ix.lower(), x);
      part2 = I(x, ix.upper());
      has_part2 = true;
    }

    if (!empty(part1)) {
      // At least 1 solution
      // We will compute part1 next
      ix = part1;

      if (has_part2 && !empty(part2)) {
        // 2 solutions
        // Save the second solution for later
        work.push(part2);
      }
    } else if (has_part2 && !empty(part2)) {
      // 1 solution
      // Work on that next
      ix = part2;
    } else {
      // No solution
      // Do we still have work to do in the stack?
      if (work.empty())  // If not, we are done
        break;
      else
        ix = work.pop();  // Otherwise, pick an interval to work on
    }
  }
}

// Naive implementation, no attempt to keep the top of the stack in registers
template <class T, int THREADS, int DEPTH_RESULT>
void newton_interval_naive(
    global_stack<interval_gpu<T>, DEPTH_RESULT, THREADS> &result,
    interval_gpu<T> const &ix0, int thread_id) {
  typedef interval_gpu<T> I;
  int const DEPTH_WORK = 128;
  T const alpha = .99f;  // Threshold before switching to bisection

  // Intervals to be processed
  local_stack<I, DEPTH_WORK> work;

  // We start with the whole domain
  work.push(ix0);

  while (!work.empty()) {
    I ix = work.pop();

    if (is_minimal(ix, thread_id)) {
      result.push(ix);
    } else {
      // Compute (x - F({x})/F'(ix)) inter ix
      // -> may yield 0, 1 or 2 intervals
      T x = median(ix);
      I iq = f(I(x), thread_id);
      I id = fd(ix, thread_id);

      bool has_part2;
      I part1, part2 = I::empty();
      part1 = division_part1(iq, id, has_part2);
      part1 = intersect(I(x) - part1, ix);

      if (has_part2) {
        part2 = division_part2(iq, id);
        part2 = intersect(I(x) - part2, ix);
      }

      if (should_bisect(ix, part1, part2, alpha)) {
        // Not so good improvement
        // Switch to bisection method for this step
        part1 = I(ix.lower(), x);
        part2 = I(x, ix.upper());
        has_part2 = true;
      }

      if (!empty(part1)) {
        work.push(part1);
      }

      if (has_part2 && !empty(part2)) {
        work.push(part2);
      }
    }
  }
}

template <class T>
void test_interval_newton(interval_gpu<T> *buffer,
                          int *nresults,
                          interval_gpu<T> i,
                          int implementation_choice)
{
  int thread_id = _bid_x * BLOCK_SIZE + _tid_x;
  typedef interval_gpu<T> I;

  // Intervals to return
  global_stack<I, DEPTH_RESULT, THREADS> result(buffer, thread_id);

  switch (implementation_choice) {
    case 0:
      newton_interval_naive<T, THREADS>(result, i, thread_id);
      break;

    case 1:
      newton_interval<T, THREADS>(result, i, thread_id);
      break;

    default:
      newton_interval_naive<T, THREADS>(result, i, thread_id);
  }

  nresults[thread_id] = result.size();
}

#endif


// --- from interval.h ---
/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INTERVAL_H
#define INTERVAL_H

#define DEVICE 0
#define TYPE double

typedef TYPE T;

int const BLOCK_SIZE = 64;
int const GRID_SIZE = 1024;
int const THREADS = GRID_SIZE * BLOCK_SIZE;
int const DEPTH_RESULT = 128;

#endif
