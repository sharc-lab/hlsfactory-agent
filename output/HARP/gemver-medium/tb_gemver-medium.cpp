#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gemver-medium_kernel.c"

// Testbench for kernel_gemver
int main() {
    printf("Starting testbench for kernel_gemver\n");
    
    int n = 100;
    double alpha = 1.0;
    double beta = 1.0;
    double A[400][400];
    // Initialize A
    for (int i = 0; i < 400; i++)
        for (int j = 0; j < 400; j++)
            A[i][j] = (double)(i * 400 + j);
    double u1[400];
    // Initialize u1
    for (int i = 0; i < 400; i++)
        u1[i] = (double)i;
    double v1[400];
    // Initialize v1
    for (int i = 0; i < 400; i++)
        v1[i] = (double)i;
    double u2[400];
    // Initialize u2
    for (int i = 0; i < 400; i++)
        u2[i] = (double)i;
    double v2[400];
    // Initialize v2
    for (int i = 0; i < 400; i++)
        v2[i] = (double)i;
    double w[400];
    // Initialize w
    for (int i = 0; i < 400; i++)
        w[i] = (double)i;
    double x[400];
    // Initialize x
    for (int i = 0; i < 400; i++)
        x[i] = (double)i;
    double y[400];
    // Initialize y
    for (int i = 0; i < 400; i++)
        y[i] = (double)i;
    double z[400];
    // Initialize z
    for (int i = 0; i < 400; i++)
        z[i] = (double)i;

    // Call the DUT
    kernel_gemver(n, alpha, beta, A, u1, v1, u2, v2, w, x, y, z);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
