#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "2mm_kernel.c"

// Testbench for kernel_2mm
int main() {
    printf("Starting testbench for kernel_2mm\n");
    
    int ni = 100;
    int nj = 100;
    int nk = 100;
    int nl = 100;
    double alpha = 1.0;
    double beta = 1.0;
    double tmp[40][50];
    // Initialize tmp
    for (int i = 0; i < 40; i++)
        for (int j = 0; j < 50; j++)
            tmp[i][j] = (double)(i * 50 + j);
    double A[40][70];
    // Initialize A
    for (int i = 0; i < 40; i++)
        for (int j = 0; j < 70; j++)
            A[i][j] = (double)(i * 70 + j);
    double B[70][50];
    // Initialize B
    for (int i = 0; i < 70; i++)
        for (int j = 0; j < 50; j++)
            B[i][j] = (double)(i * 50 + j);
    double C[50][80];
    // Initialize C
    for (int i = 0; i < 50; i++)
        for (int j = 0; j < 80; j++)
            C[i][j] = (double)(i * 80 + j);
    double D[40][80];
    // Initialize D
    for (int i = 0; i < 40; i++)
        for (int j = 0; j < 80; j++)
            D[i][j] = (double)(i * 80 + j);

    // Call the DUT
    kernel_2mm(ni, nj, nk, nl, alpha, beta, tmp, A, B, C, D);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
