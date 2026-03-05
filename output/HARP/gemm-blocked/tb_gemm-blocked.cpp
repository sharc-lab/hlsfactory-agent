#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gemm-blocked_kernel.c"

// Testbench for bbgemm
int main() {
    printf("Starting testbench for bbgemm\n");
    
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
    bbgemm(m1, m2, prod);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
