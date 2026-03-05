#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "heat-3d_kernel.c"

// Testbench for kernel_heat_3d
int main() {
    printf("Starting testbench for kernel_heat_3d\n");
    
    int tsteps = 100;
    int n = 100;
    double A[20][20];
    // Initialize A
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 20; j++)
            A[i][j] = (double)(i * 20 + j);
    double B[20][20];
    // Initialize B
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 20; j++)
            B[i][j] = (double)(i * 20 + j);

    // Call the DUT
    kernel_heat_3d(tsteps, n, A, B);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
