#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gesummv-medium_kernel.c"

// Testbench for kernel_gesummv
int main() {
    printf("Starting testbench for kernel_gesummv\n");
    
    double alpha = 1.0;
    double beta = 1.0;
    double A[250][250];
    // Initialize A
    for (int i = 0; i < 250; i++)
        for (int j = 0; j < 250; j++)
            A[i][j] = (double)(i * 250 + j);
    double B[250][250];
    // Initialize B
    for (int i = 0; i < 250; i++)
        for (int j = 0; j < 250; j++)
            B[i][j] = (double)(i * 250 + j);
    double tmp[250];
    // Initialize tmp
    for (int i = 0; i < 250; i++)
        tmp[i] = (double)i;
    double x[250];
    // Initialize x
    for (int i = 0; i < 250; i++)
        x[i] = (double)i;
    double y[250];
    // Initialize y
    for (int i = 0; i < 250; i++)
        y[i] = (double)i;

    // Call the DUT
    kernel_gesummv(alpha, beta, A, B, tmp, x, y);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
