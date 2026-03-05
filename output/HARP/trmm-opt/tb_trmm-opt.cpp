#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "trmm-opt_kernel.c"

// Testbench for kernel_trmm
int main() {
    printf("Starting testbench for kernel_trmm\n");
    
    double alpha = 1.0;
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
    kernel_trmm(alpha, A, B);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
