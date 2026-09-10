#ifndef FFT_H
#define FFT_H

#include <complex>
#include <cmath>
#include <iostream>
#include <iomanip>

using namespace std;
typedef float dtype_test;
typedef double dtype_gold;
#define batch_size 1 
#define FFT_NUM 1024
#define EXP2_FFT 10

#define UF 1


#ifndef PI
#define PI  3.14159265358979323846
#endif

void FFT_TOP(complex<float> dataIn[FFT_NUM], complex<float> dataOut[FFT_NUM]) ;

#endif
