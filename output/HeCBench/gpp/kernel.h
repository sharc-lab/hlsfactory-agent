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
#ifndef BLOCK_DIM_Y
#define BLOCK_DIM_Y 1
#endif
#ifndef GRID_DIM_Y
#define GRID_DIM_Y 1
#endif

// --- from main.cu ---
#include <string.h>
#include <chrono>

#ifndef dataType
#define dataType double
#endif




// --- from CustomComplex.h ---
//
// Complex number computation class
//
#ifndef __CustomComplex
#define __CustomComplex

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sys/time.h>

#if defined(__NVCC__) || defined(__HIPCC__)
#define ESS #else
#define ESS
#endif

#define nstart 0
#define nend 3

template <class T>
class CustomComplex
{
  private:
  public:
    T x;
    T y;
    explicit CustomComplex()
    {
      x = 0.00;
      y = 0.00;
    }

    ESS
      explicit CustomComplex(const T& a, const T& b)
      {
        x = a;
        y = b;
      }

    ESS
      CustomComplex(const CustomComplex& src)
      {
        x = src.x;
        y = src.y;
      }

    ESS
      CustomComplex& operator=(const CustomComplex& src)
      {
        x = src.x;
        y = src.y;
        return *this;
      }

    ESS
      CustomComplex& operator+=(const CustomComplex& src)
      {
        x = src.x + this->x;
        y = src.y + this->y;
        return *this;
      }

    ESS
      CustomComplex& operator-=(const CustomComplex& src)
      {
        x = src.x - this->x;
        y = src.y - this->y;
        return *this;
      }

    ESS
      CustomComplex& operator-()
      {
        x = -this->x;
        y = -this->y;
        return *this;
      }

    ESS
      CustomComplex conj()
      {
        T re_this = this->x;
        T im_this = -1 * this->y;

        CustomComplex<T> result(re_this, im_this);
        return result;
      }

    ESS
      T real() { return this->x; }

    ESS
      T imag() { return this->y; }

    ESS
      CustomComplex& operator~() { return *this; }

    void print() const
    {
      printf("( %f, %f) ", this->x, this->y);
      printf("\n");
    }

    friend std::ostream& operator<<(std::ostream& os, const CustomComplex<T>& obj)
    {
      os << "( " << obj.x << ", " << obj.y << ") ";
      return os;
    }

    T get_real() const { return this->x; }

    T get_imag() const { return this->y; }

    void set_real(T val) { this->x = val; }

    void set_imag(T val) { this->y = val; }

    // 6 flops
      ESS 
      friend inline CustomComplex<T> operator*(const CustomComplex<T>& a,
          const CustomComplex<T>& b)
      {
        T                x_this = a.x * b.x - a.y * b.y;
        T                y_this = a.x * b.y + a.y * b.x;
        CustomComplex<T> result(x_this, y_this);
        return (result);
      }

    // 2 flops
      ESS 
      friend inline CustomComplex<T> operator*(const CustomComplex<T> &a,
          const T &b)
      {
        CustomComplex<T> result(a.x * b, a.y * b);
        return result;
      }

      ESS 
      friend inline CustomComplex<T> operator*(const T &b,
          const CustomComplex<T> &a)
      {
        CustomComplex<T> result(a.x * b, a.y * b);
        return result;
      }

      ESS 
      friend inline CustomComplex<T> operator-(const CustomComplex<T> &a,
          const CustomComplex<T> &b)
      {
        CustomComplex<T> result(a.x - b.x, a.y - b.y);
        return result;
      }

    // 2 flops
      ESS 
      friend inline CustomComplex<T> operator-(const T &a,
          const CustomComplex<T> &src)
      {
        CustomComplex<T> result(a - src.x, 0 - src.y);
        return result;
      }

      ESS
      friend inline CustomComplex<T> operator+(const T &a,
          CustomComplex<T> &src)
      {
        CustomComplex<T> result(a + src.x, src.y);
        return result;
      }

      ESS
      friend inline CustomComplex<T> operator+(CustomComplex<T> &a,
          CustomComplex<T> &b)
      {
        CustomComplex<T> result(a.x + b.x, a.y + b.y);
        return result;
      }

      ESS
      friend inline CustomComplex<T> operator/(CustomComplex<T> &a,
          CustomComplex<T> &b)
      {
        CustomComplex<T> b_conj      = CustomComplex_conj(b);
        CustomComplex<T> numerator   = a * b_conj;
        CustomComplex<T> denominator = b * b_conj;

        T re_this = numerator.x / denominator.x;
        T im_this = numerator.y / denominator.x;

        CustomComplex<T> result(re_this, im_this);
        return result;
      }

      ESS
      friend inline CustomComplex<T> operator/(CustomComplex<T> &a, T &b)
      {
        CustomComplex<T> result(a.x / b, a.y / b);
        return result;
      }

      ESS
      friend inline CustomComplex<T> CustomComplex_conj(
          const CustomComplex<T> &src)
      {   
        T re_this = src.x;
        T im_this = -1 * src.y;
       
        CustomComplex<T> result(re_this, im_this);
        return result;
       }

      ESS
      friend inline T CustomComplex_abs(const CustomComplex<T> &src)
      {
        T re_this = src.x * src.x;
        T im_this = src.y * src.y;

        T result = sqrt(re_this + im_this);
        return result;
      }

      ESS
      friend inline T CustomComplex_real(const CustomComplex<T> &src) 
      {
        return src.x;
      }

      ESS
      friend inline T CustomComplex_imag(const CustomComplex<T> &src)
      {
        return src.y;
      }
};
#endif


// --- from kernel.h ---
void solver(
    int number_bands, int ngpown, int ncouls,
    const int * inv_igp_index,
    const int * indinv,
    const dataType * wx_array,
    const CustomComplex<dataType> * wtilde_array,
    const CustomComplex<dataType> * aqsmtemp,
    const CustomComplex<dataType> * aqsntemp,
    const CustomComplex<dataType> * I_eps_array,
    const dataType * vcoul,
    dataType * achtemp_re,
    dataType * achtemp_im) 
{
  dataType achtemp_re_loc[nend - nstart], achtemp_im_loc[nend - nstart];
  for (int iw = nstart; iw < nend; ++iw) {
    achtemp_re_loc[iw] = 0.00;
    achtemp_im_loc[iw] = 0.00;
  }

  for (int n1 = _bid_x; n1 < number_bands; n1 += GRID_DIM_X) // 512 iterations
  {
    for (int my_igp = _bid_y; my_igp < ngpown; my_igp += GRID_DIM_Y) // 1634 iterations
    {
      int indigp = inv_igp_index[my_igp];
      int igp = indinv[indigp];
      CustomComplex<dataType> sch_store1 =
          CustomComplex_conj(aqsmtemp(n1, igp)) * aqsntemp(n1, igp) * 0.5 *
          vcoul[igp];

      for (int ig = _tid_x; ig < ncouls; ig += BLOCK_DIM_X) {
        #pragma unroll
        for (int iw = nstart; iw < nend; ++iw) // 3 iterations
        {
          CustomComplex<dataType> wdiff =
              wx_array[iw] - wtilde_array(my_igp, ig);
          CustomComplex<dataType> delw =
              wtilde_array(my_igp, ig) * CustomComplex_conj(wdiff) *
              (1 / CustomComplex_real((wdiff * CustomComplex_conj(wdiff))));
          CustomComplex<dataType> sch_array =
              delw * I_eps_array(my_igp, ig) * sch_store1;

          achtemp_re_loc[iw] += CustomComplex_real(sch_array);
          achtemp_im_loc[iw] += CustomComplex_imag(sch_array);
        }
      }
    } // ngpown
  }   // number_bands

  // Add the final results here
  for (int iw = nstart; iw < nend; ++iw) {
    (achtemp_re[iw] += achtemp_re_loc[iw]);
    (achtemp_im[iw] += achtemp_im_loc[iw]);
  }
}


// --- from utils.h ---
#include <stdio.h>
#include <stdlib.h>

#define aqsmtemp_size      (number_bands * ncouls)
#define aqsntemp_size      (number_bands * ncouls)
#define I_eps_array_size   (ngpown * ncouls)
#define achtemp_size       (nend - nstart)
#define achtemp_re_size    (nend - nstart)
#define achtemp_im_size    (nend - nstart)
#define vcoul_size         ncouls
#define inv_igp_index_size ngpown
#define indinv_size        (ncouls + 1)
#define wx_array_size      (nend - nstart)
#define wtilde_array_size  (ngpown * ncouls)

#define aqsmtemp(n1, ig) aqsmtemp[n1 * ncouls + ig]
#define aqsntemp(n1, ig) aqsntemp[n1 * ncouls + ig]
#define I_eps_array(my_igp, ig) I_eps_array[my_igp * ncouls + ig]
#define wtilde_array(my_igp, ig) wtilde_array[my_igp * ncouls + ig]

inline void *safe_malloc(size_t n) {
  void *p = malloc(n);
  if (p == NULL) {
    fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", n);
    abort();
  }
  return p;
}

// Here we are checking to see if the answers are correct
inline void correctness(int problem_size, CustomComplex<dataType> result) {
  if (problem_size == 0) {
    dataType re_diff = result.get_real() - -24852.551547;
    dataType im_diff = result.get_imag() - 2957453.638101;

    if (re_diff < 0.00001 && im_diff < 0.00001)
      printf("\nBenchmark result: SUCCESS\n");
    else
      printf("\nBenchmark result: FAILURE\n");

  } else {
    dataType re_diff = result.get_real() - -0.096066;
    dataType im_diff = result.get_imag() - 11.431852;

    if (re_diff < 0.00001 && im_diff < 0.00001)
      printf("\nTest result: SUCCESS\n");
    else
      printf("\nTest result: FAILURE\n");
  }
}
