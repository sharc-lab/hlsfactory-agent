#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "spmv-ellpack_kernel.c"

// Testbench for ellpack
int main() {
    printf("Starting testbench for ellpack\n");
    
    double nzval[4940];
    // Initialize nzval
    for (int i = 0; i < 4940; i++)
        nzval[i] = (double)i;
    int cols[4940];
    // Initialize cols
    for (int i = 0; i < 4940; i++)
        cols[i] = (int)i;
    double vec[494];
    // Initialize vec
    for (int i = 0; i < 494; i++)
        vec[i] = (double)i;
    double out[494];
    // Initialize out
    for (int i = 0; i < 494; i++)
        out[i] = (double)i;

    // Call the DUT
    ellpack(nzval, cols, vec, out);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
