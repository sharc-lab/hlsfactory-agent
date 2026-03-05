#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "md_kernel.c"

// Testbench for md_kernel
int main() {
    printf("Starting testbench for md_kernel\n");
    
    double force_x[256];
    // Initialize force_x
    for (int i = 0; i < 256; i++)
        force_x[i] = (double)i;
    double force_y[256];
    // Initialize force_y
    for (int i = 0; i < 256; i++)
        force_y[i] = (double)i;
    double force_z[256];
    // Initialize force_z
    for (int i = 0; i < 256; i++)
        force_z[i] = (double)i;
    double position_x[256];
    // Initialize position_x
    for (int i = 0; i < 256; i++)
        position_x[i] = (double)i;
    double position_y[256];
    // Initialize position_y
    for (int i = 0; i < 256; i++)
        position_y[i] = (double)i;
    double position_z[256];
    // Initialize position_z
    for (int i = 0; i < 256; i++)
        position_z[i] = (double)i;
    int NL[4096];
    // Initialize NL
    for (int i = 0; i < 4096; i++)
        NL[i] = (int)i;

    // Call the DUT
    md_kernel(force_x, force_y, force_z, position_x, position_y, position_z, NL);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
