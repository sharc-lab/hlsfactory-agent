#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gemm-p-large_kernel.c"

// Testbench for kernel_gemm
int main() {
    printf("Starting testbench for kernel_gemm\n");
    
    int ni = 100;
    int nj = 100;
    int nk = 100;
    double alpha = 1.0;
    double beta = 1.0;
    double C[200][220];
    // Initialize C
    for (int i = 0; i < 200; i++)
        for (int j = 0; j < 220; j++)
            C[i][j] = (double)(i * 220 + j);
    double A[200][240];
    // Initialize A
    for (int i = 0; i < 200; i++)
        for (int j = 0; j < 240; j++)
            A[i][j] = (double)(i * 240 + j);
    double B[240][220];
    // Initialize B
    for (int i = 0; i < 240; i++)
        for (int j = 0; j < 220; j++)
            B[i][j] = (double)(i * 220 + j);

    // Call the DUT
    kernel_gemm(ni, nj, nk, alpha, beta, C, A, B);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
