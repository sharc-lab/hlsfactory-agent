#ifndef FFT_H
#define FFT_H

#include "ac_fixed.h"
#include "ac_channel.h"
#include "ac_int.h"
#include <complex>
#include <iomanip>   // std::setprecision, std::setw
#include <iostream>  // std::cout, std::fixed
#include <cmath>

using namespace std;
typedef float dtype_test;
typedef double dtype_gold;
#define batch_size 1 
#define FFT_NUM 1024
#define EXP2_FFT 10

#define UF 16


#ifndef PI
#define PI  3.14159265358979323846
#endif

// Replacement for hls::vector<T,N>
template<typename T, int N>
struct ac_vector {
    T data[N];
    T& operator[](int i) { return data[i]; }
    const T& operator[](int i) const { return data[i]; }
};

void FFT_TOP(ac_channel<ac_vector<complex<float>, UF*2>> & in, 
    ac_channel<ac_vector<complex<float>, UF*2>> & out
);


#endif