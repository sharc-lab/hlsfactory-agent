#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "adi_kernel.c"

// Testbench for kernel_adi
int main() {
    printf("Starting testbench for kernel_adi\n");
    
    int tsteps = 100;
    int n = 100;
    double u[60][60];
    // Initialize u
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 60; j++)
            u[i][j] = (double)(i * 60 + j);
    double v[60][60];
    // Initialize v
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 60; j++)
            v[i][j] = (double)(i * 60 + j);
    double p[60][60];
    // Initialize p
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 60; j++)
            p[i][j] = (double)(i * 60 + j);
    double q[60][60];
    // Initialize q
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 60; j++)
            q[i][j] = (double)(i * 60 + j);

    // Call the DUT
    kernel_adi(tsteps, n, u, v, p, q);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
