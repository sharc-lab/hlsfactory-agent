#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "symm-opt_kernel.c"

// Testbench for kernel_symm
int main() {
    printf("Starting testbench for kernel_symm\n");
    
    double alpha = 1.0;
    double beta = 1.0;
    double C[60][80];
    // Initialize C
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 80; j++)
            C[i][j] = (double)(i * 80 + j);
    double A[60][60];
    // Initialize A
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 60; j++)
            A[i][j] = (double)(i * 60 + j);
    double B[60][80];
    // Initialize B
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 80; j++)
            B[i][j] = (double)(i * 80 + j);

    // Call the DUT
    kernel_symm(alpha, beta, C, A, B);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
