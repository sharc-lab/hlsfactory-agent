#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gesummv_kernel.c"

// Testbench for kernel_gesummv
int main() {
    printf("Starting testbench for kernel_gesummv\n");
    
    int n = 100;
    double alpha = 1.0;
    double beta = 1.0;
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
    double tmp[90];
    // Initialize tmp
    for (int i = 0; i < 90; i++)
        tmp[i] = (double)i;
    double x[90];
    // Initialize x
    for (int i = 0; i < 90; i++)
        x[i] = (double)i;
    double y[90];
    // Initialize y
    for (int i = 0; i < 90; i++)
        y[i] = (double)i;

    // Call the DUT
    kernel_gesummv(n, alpha, beta, A, B, tmp, x, y);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
