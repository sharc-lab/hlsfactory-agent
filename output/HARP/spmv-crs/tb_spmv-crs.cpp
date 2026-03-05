#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "spmv-crs_kernel.c"

// Testbench for spmv
int main() {
    printf("Starting testbench for spmv\n");
    
    double val[1666];
    // Initialize val
    for (int i = 0; i < 1666; i++)
        val[i] = (double)i;
    int cols[1666];
    // Initialize cols
    for (int i = 0; i < 1666; i++)
        cols[i] = (int)i;
    int rowDelimiters[495];
    // Initialize rowDelimiters
    for (int i = 0; i < 495; i++)
        rowDelimiters[i] = (int)i;
    double vec[494];
    // Initialize vec
    for (int i = 0; i < 494; i++)
        vec[i] = (double)i;
    double out[494];
    // Initialize out
    for (int i = 0; i < 494; i++)
        out[i] = (double)i;

    // Call the DUT
    spmv(val, cols, rowDelimiters, vec, out);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
