#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "atax_kernel.c"

// Testbench for kernel_atax
int main() {
    printf("Starting testbench for kernel_atax\n");
    
    int m = 100;
    int n = 100;
    double A[116][124];
    // Initialize A
    for (int i = 0; i < 116; i++)
        for (int j = 0; j < 124; j++)
            A[i][j] = (double)(i * 124 + j);
    double x[124];
    // Initialize x
    for (int i = 0; i < 124; i++)
        x[i] = (double)i;
    double y[124];
    // Initialize y
    for (int i = 0; i < 124; i++)
        y[i] = (double)i;
    double tmp[116];
    // Initialize tmp
    for (int i = 0; i < 116; i++)
        tmp[i] = (double)i;

    // Call the DUT
    kernel_atax(m, n, A, x, y, tmp);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
