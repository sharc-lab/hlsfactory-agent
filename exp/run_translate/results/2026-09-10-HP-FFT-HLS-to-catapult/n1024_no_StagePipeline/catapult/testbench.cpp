#include "FFT.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <complex>

using namespace std;

void fft_sw_gold(complex<double>* input, complex<double>* output) {
    for (int k = 0; k < FFT_NUM; ++k) {
        complex<double> sum = 0;
        for (int n = 0; n < FFT_NUM; ++n) {
            double angle = -2.0 * PI * k * n / FFT_NUM;
            complex<double> exp_term(cos(angle), sin(angle));
            sum += input[n] * exp_term;
        }
        output[k] = sum;
    }
}

int main()
{
    complex<float> data[batch_size][FFT_NUM], dataFq[batch_size][FFT_NUM];
    complex<double> data_gold[batch_size][FFT_NUM], dataFq_gold[batch_size][FFT_NUM];

    for (int b = 0; b < batch_size; b++){
        for (int i = 0; i < FFT_NUM; i++) {
            float t = static_cast<float>(i) / FFT_NUM;
            float real_part = sin(2.0 * M_PI * 10.0 * t) + 0.5 * cos(2.0 * M_PI * 50.0 * t);
            float imag_part = exp(-5.0 * t) * sin(2.0 * M_PI * 20.0 * t);
            data[b][i] = complex<float>(real_part, imag_part);
            data_gold[b][i] = complex<double>(real_part, imag_part);
        }
    }

    cout << "FFT run FFT_TOP (array-based)" << endl;
    for (int b = 0; b < batch_size; b++){
        FFT_TOP(data[b], dataFq[b]);
    }

    for (int b = 0; b < batch_size; b++){
        fft_sw_gold(data_gold[b], dataFq_gold[b]);
    }

    cout << "FFT Comparison" << endl;
    for (int i = 0; i < FFT_NUM; i++)
        cout << dataFq[0][i] << dataFq_gold[0][i] << endl;

    cout << endl << "FFT difference" << endl;
    cout << fixed << setprecision(4);
    
    double max_error = 0.0;
    for (int i = 0; i < FFT_NUM; i++) {
        complex<double> diff = complex<double>(dataFq[0][i]) - dataFq_gold[0][i];
        cout << diff << endl;
        double err = abs(diff);
        if (err > max_error) max_error = err;
    }
    
    cout << "Max error: " << max_error << endl;
    
    if (max_error > 1.0) {
        cout << "Test FAILED - error too large" << endl;
        return 1;
    }
    
    cout << "Test PASSED" << endl;
    return 0;
}