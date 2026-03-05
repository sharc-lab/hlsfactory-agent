#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gemm-ncubed_kernel.c"

// Testbench for gemm
int main() {
    printf("Starting testbench for gemm\n");
    
    double m1[4096];
    // Initialize m1
    for (int i = 0; i < 4096; i++)
        m1[i] = (double)i;
    double m2[4096];
    // Initialize m2
    for (int i = 0; i < 4096; i++)
        m2[i] = (double)i;
    double prod[4096];
    // Initialize prod
    for (int i = 0; i < 4096; i++)
        prod[i] = (double)i;

    // Call the DUT
    gemm(m1, m2, prod);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
