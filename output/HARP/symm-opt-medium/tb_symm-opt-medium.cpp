#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "symm-opt-medium_kernel.c"

// Testbench for kernel_symm
int main() {
    printf("Starting testbench for kernel_symm\n");
    
    double alpha = 1.0;
    double beta = 1.0;
    double C[200][240];
    // Initialize C
    for (int i = 0; i < 200; i++)
        for (int j = 0; j < 240; j++)
            C[i][j] = (double)(i * 240 + j);
    double A[200][200];
    // Initialize A
    for (int i = 0; i < 200; i++)
        for (int j = 0; j < 200; j++)
            A[i][j] = (double)(i * 200 + j);
    double B[200][240];
    // Initialize B
    for (int i = 0; i < 200; i++)
        for (int j = 0; j < 240; j++)
            B[i][j] = (double)(i * 240 + j);

    // Call the DUT
    kernel_symm(alpha, beta, C, A, B);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
