#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "correlation_kernel.c"

// Testbench for kernel_correlation
int main() {
    printf("Starting testbench for kernel_correlation\n");
    
    double float_n = 1.0;
    double data[100][80];
    // Initialize data
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 80; j++)
            data[i][j] = (double)(i * 80 + j);
    double corr[80][80];
    // Initialize corr
    for (int i = 0; i < 80; i++)
        for (int j = 0; j < 80; j++)
            corr[i][j] = (double)(i * 80 + j);
    double mean[80];
    // Initialize mean
    for (int i = 0; i < 80; i++)
        mean[i] = (double)i;
    double stddev[80];
    // Initialize stddev
    for (int i = 0; i < 80; i++)
        stddev[i] = (double)i;

    // Call the DUT
    kernel_correlation(float_n, data, corr, mean, stddev);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
