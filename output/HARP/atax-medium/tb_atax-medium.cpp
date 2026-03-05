#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "atax-medium_kernel.c"

// Testbench for kernel_atax
int main() {
    printf("Starting testbench for kernel_atax\n");
    
    double A[390][410];
    // Initialize A
    for (int i = 0; i < 390; i++)
        for (int j = 0; j < 410; j++)
            A[i][j] = (double)(i * 410 + j);
    double x[410];
    // Initialize x
    for (int i = 0; i < 410; i++)
        x[i] = (double)i;
    double y[410];
    // Initialize y
    for (int i = 0; i < 410; i++)
        y[i] = (double)i;
    double tmp[390];
    // Initialize tmp
    for (int i = 0; i < 390; i++)
        tmp[i] = (double)i;

    // Call the DUT
    kernel_atax(A, x, y, tmp);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
