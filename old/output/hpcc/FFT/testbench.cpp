#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "hpccfft.h"

// Simple testbench for FFT kernel
int main() {
    const int N = 64;  // Small test size
    
    // Allocate arrays
    fftw_complex *a = (fftw_complex*)malloc(N * sizeof(fftw_complex));
    fftw_complex *b = (fftw_complex*)malloc(N * sizeof(fftw_complex));
    
    // Initialize with test data
    for (int i = 0; i < N; i++) {
        c_re(a[i]) = cos(2.0 * M_PI * i / N);
        c_im(a[i]) = sin(2.0 * M_PI * i / N);
    }
    
    // Create plan
    hpcc_fftw_plan plan = HPCC_fftw_create_plan(N, FFTW_FORWARD, FFTW_ESTIMATE);
    if (!plan) {
        printf("Failed to create plan\n");
        return 1;
    }
    
    // Perform FFT
    int result = HPCC_zfft1d(N, a, b, -1, plan);
    
    // Check result
    int pass = (result == 0);
    printf("FFT Test %s\n", pass ? "PASSED" : "FAILED");
    
    // Cleanup
    HPCC_fftw_destroy_plan(plan);
    free(a);
    free(b);
    
    return pass ? 0 : 1;
}
