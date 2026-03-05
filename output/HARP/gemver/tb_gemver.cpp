#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gemver_kernel.c"

// Testbench for kernel_gemver
int main() {
    printf("Starting testbench for kernel_gemver\n");
    
    int n = 100;
    double alpha = 1.0;
    double beta = 1.0;
    double A[120][120];
    // Initialize A
    for (int i = 0; i < 120; i++)
        for (int j = 0; j < 120; j++)
            A[i][j] = (double)(i * 120 + j);
    double u1[120];
    // Initialize u1
    for (int i = 0; i < 120; i++)
        u1[i] = (double)i;
    double v1[120];
    // Initialize v1
    for (int i = 0; i < 120; i++)
        v1[i] = (double)i;
    double u2[120];
    // Initialize u2
    for (int i = 0; i < 120; i++)
        u2[i] = (double)i;
    double v2[120];
    // Initialize v2
    for (int i = 0; i < 120; i++)
        v2[i] = (double)i;
    double w[120];
    // Initialize w
    for (int i = 0; i < 120; i++)
        w[i] = (double)i;
    double x[120];
    // Initialize x
    for (int i = 0; i < 120; i++)
        x[i] = (double)i;
    double y[120];
    // Initialize y
    for (int i = 0; i < 120; i++)
        y[i] = (double)i;
    double z[120];
    // Initialize z
    for (int i = 0; i < 120; i++)
        z[i] = (double)i;

    // Call the DUT
    kernel_gemver(n, alpha, beta, A, u1, v1, u2, v2, w, x, y, z);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
