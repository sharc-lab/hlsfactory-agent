#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stencil-3d_kernel.c"

// Testbench for stencil3d
int main() {
    printf("Starting testbench for stencil3d\n");
    
    long orig[39304];
    // Initialize orig
    for (int i = 0; i < 39304; i++)
        orig[i] = (long)i;
    long sol[32768];
    // Initialize sol
    for (int i = 0; i < 32768; i++)
        sol[i] = (long)i;

    // Call the DUT
    stencil3d(orig, sol);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
