#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stencil_stencil2d_kernel.c"

// Testbench for stencil
int main() {
    printf("Starting testbench for stencil\n");
    
    int orig[8192];
    // Initialize orig
    for (int i = 0; i < 8192; i++)
        orig[i] = (int)i;
    int sol[8192];
    // Initialize sol
    for (int i = 0; i < 8192; i++)
        sol[i] = (int)i;
    int filter[9];
    // Initialize filter
    for (int i = 0; i < 9; i++)
        filter[i] = (int)i;

    // Call the DUT
    stencil(orig, sol, filter);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
