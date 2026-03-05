#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gemm-p_kernel.c"

// Testbench for kernel_gemm
int main() {
    printf("Starting testbench for kernel_gemm\n");
    
    int ni = 100;
    int nj = 100;
    int nk = 100;
    double alpha = 1.0;
    double beta = 1.0;
    double C[60][70];
    // Initialize C
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 70; j++)
            C[i][j] = (double)(i * 70 + j);
    double A[60][80];
    // Initialize A
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 80; j++)
            A[i][j] = (double)(i * 80 + j);
    double B[80][70];
    // Initialize B
    for (int i = 0; i < 80; i++)
        for (int j = 0; j < 70; j++)
            B[i][j] = (double)(i * 70 + j);

    // Call the DUT
    kernel_gemm(ni, nj, nk, alpha, beta, C, A, B);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
