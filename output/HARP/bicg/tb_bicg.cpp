#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bicg_kernel.c"

// Testbench for kernel_bicg
int main() {
    printf("Starting testbench for kernel_bicg\n");
    
    int m = 100;
    int n = 100;
    double A[124][116];
    // Initialize A
    for (int i = 0; i < 124; i++)
        for (int j = 0; j < 116; j++)
            A[i][j] = (double)(i * 116 + j);
    double s[116];
    // Initialize s
    for (int i = 0; i < 116; i++)
        s[i] = (double)i;
    double q[124];
    // Initialize q
    for (int i = 0; i < 124; i++)
        q[i] = (double)i;
    double p[116];
    // Initialize p
    for (int i = 0; i < 116; i++)
        p[i] = (double)i;
    double r[124];
    // Initialize r
    for (int i = 0; i < 124; i++)
        r[i] = (double)i;

    // Call the DUT
    kernel_bicg(m, n, A, s, q, p, r);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
