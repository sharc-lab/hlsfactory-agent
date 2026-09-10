#ifndef FFT_H
#define FFT_H

#include "ac_fixed.h"
#include "ac_channel.h"
#include <complex>
#include <iomanip>   // std::setprecision, std::setw
#include <iostream>  // std::cout, std::fixed
#include <cmath>

// Replacement for hls::vector<T, N> used in Vitis HLS
template<typename T, int N>
struct hls_vector {
    T data[N];
    T& operator[](int idx) { return data[idx]; }
    const T& operator[](int idx) const { return data[idx]; }
};

using namespace std;
typedef float dtype_test;
typedef double dtype_gold;
#define batch_size 1 
#define FFT_NUM 256
#define EXP2_FFT 8

#define UF 8


#ifndef PI
#define PI  3.14159265358979323846
#endif

void FFT_TOP(ac_channel<hls_vector<complex<float>, UF*2>> & in, 
    ac_channel<hls_vector<complex<float>, UF*2>> & out
);


#endif