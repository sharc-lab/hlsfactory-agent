#ifndef FFT_H
#define FFT_H

#include <complex>
#include <cmath>

#ifndef PI
#define PI  3.14159265358979323846
#endif

#define FFT_NUM 1024
#define EXP2_FFT 10

#define batch_size 1

typedef float dtype_test;
typedef double dtype_gold;

void FFT_TOP(std::complex<float> dataIn[FFT_NUM], std::complex<float> dataOut[FFT_NUM]);

#endif