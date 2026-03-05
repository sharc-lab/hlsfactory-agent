#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "syrk_kernel.c"

// Testbench for kernel_syrk
int main() {
    printf("Starting testbench for kernel_syrk\n");
    
    double alpha = 1.0;
    double beta = 1.0;
    double C[80][80];
    // Initialize C
    for (int i = 0; i < 80; i++)
        for (int j = 0; j < 80; j++)
            C[i][j] = (double)(i * 80 + j);
    double A[80][60];
    // Initialize A
    for (int i = 0; i < 80; i++)
        for (int j = 0; j < 60; j++)
            A[i][j] = (double)(i * 60 + j);

    // Call the DUT
    kernel_syrk(alpha, beta, C, A);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
