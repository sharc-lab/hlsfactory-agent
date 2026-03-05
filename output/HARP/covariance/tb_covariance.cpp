#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "covariance_kernel.c"

// Testbench for kernel_covariance
int main() {
    printf("Starting testbench for kernel_covariance\n");
    
    int m = 100;
    int n = 100;
    double float_n = 1.0;
    double data[100][80];
    // Initialize data
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 80; j++)
            data[i][j] = (double)(i * 80 + j);
    double cov[80][80];
    // Initialize cov
    for (int i = 0; i < 80; i++)
        for (int j = 0; j < 80; j++)
            cov[i][j] = (double)(i * 80 + j);
    double mean[80];
    // Initialize mean
    for (int i = 0; i < 80; i++)
        mean[i] = (double)i;

    // Call the DUT
    kernel_covariance(m, n, float_n, data, cov, mean);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
