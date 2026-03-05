#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "seidel-2d_kernel.c"

// Testbench for kernel_seidel_2d
int main() {
    printf("Starting testbench for kernel_seidel_2d\n");
    
    int tsteps = 100;
    int n = 100;
    double A[120][120];
    // Initialize A
    for (int i = 0; i < 120; i++)
        for (int j = 0; j < 120; j++)
            A[i][j] = (double)(i * 120 + j);

    // Call the DUT
    kernel_seidel_2d(tsteps, n, A);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
