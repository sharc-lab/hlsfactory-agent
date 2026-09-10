#ifndef FFT_H
#define FFT_H

#include <ac_channel.h>
#include <ac_int.h>
#include <complex>
#include <iomanip>   // std::setprecision, std::setw
#include <iostream>  // std::cout, std::fixed
#include <cmath>

using namespace std;
typedef float dtype_test;
typedef double dtype_gold;
#define batch_size 1 
#define FFT_NUM 256
#define EXP2_FFT 8

#define UF 32

// Vector of 64 complex<float> values (replaces Vitis vector type)
struct vec_cf64 {
    complex<float> data[64];
    complex<float>& operator[](int i) { return data[i]; }
    const complex<float>& operator[](int i) const { return data[i]; }
};


#ifndef PI
#define PI  3.14159265358979323846
#endif

void FFT_TOP(ac_channel<vec_cf64> & in, 
    ac_channel<vec_cf64> & out
);


#endif