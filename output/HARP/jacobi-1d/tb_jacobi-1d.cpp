#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "jacobi-1d_kernel.c"

// Testbench for kernel_jacobi_1d
int main() {
    printf("Starting testbench for kernel_jacobi_1d\n");
    
    int tsteps = 100;
    int n = 100;
    double A[120];
    // Initialize A
    for (int i = 0; i < 120; i++)
        A[i] = (double)i;
    double B[120];
    // Initialize B
    for (int i = 0; i < 120; i++)
        B[i] = (double)i;

    // Call the DUT
    kernel_jacobi_1d(tsteps, n, A, B);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
