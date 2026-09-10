#ifndef FFT_H
#define FFT_H

#include <ac_int.h>
#include <ac_channel.h>
#include <complex>
#include <iomanip>   // std::setprecision, std::setw
#include <iostream>  // std::cout, std::fixed
#include <cmath>
#include <array>

using namespace std;
typedef float dtype_test;
typedef double dtype_gold;
#define batch_size 1 
#define FFT_NUM 256
#define EXP2_FFT 8

#define UF 16


#ifndef PI
#define PI  3.14159265358979323846
#endif

// Replace hls::vector<complex<float>, N> with std::array
using vec_cfloat = std::array<complex<float>, 32>;

void FFT_TOP(ac_channel<vec_cfloat> & in, 
    ac_channel<vec_cfloat> & out
);


#endif