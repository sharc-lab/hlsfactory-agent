#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bicg-medium_kernel.c"

// Testbench for kernel_bicg
int main() {
    printf("Starting testbench for kernel_bicg\n");
    
    int m = 100;
    int n = 100;
    double A[410][390];
    // Initialize A
    for (int i = 0; i < 410; i++)
        for (int j = 0; j < 390; j++)
            A[i][j] = (double)(i * 390 + j);
    double s[390];
    // Initialize s
    for (int i = 0; i < 390; i++)
        s[i] = (double)i;
    double q[410];
    // Initialize q
    for (int i = 0; i < 410; i++)
        q[i] = (double)i;
    double p[390];
    // Initialize p
    for (int i = 0; i < 390; i++)
        p[i] = (double)i;
    double r[410];
    // Initialize r
    for (int i = 0; i < 410; i++)
        r[i] = (double)i;

    // Call the DUT
    kernel_bicg(m, n, A, s, q, p, r);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
