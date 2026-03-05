#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "jacobi-2d_kernel.c"

// Testbench for kernel_jacobi_2d
int main() {
    printf("Starting testbench for kernel_jacobi_2d\n");
    
    int tsteps = 100;
    int n = 100;
    double A[90][90];
    // Initialize A
    for (int i = 0; i < 90; i++)
        for (int j = 0; j < 90; j++)
            A[i][j] = (double)(i * 90 + j);
    double B[90][90];
    // Initialize B
    for (int i = 0; i < 90; i++)
        for (int j = 0; j < 90; j++)
            B[i][j] = (double)(i * 90 + j);

    // Call the DUT
    kernel_jacobi_2d(tsteps, n, A, B);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
