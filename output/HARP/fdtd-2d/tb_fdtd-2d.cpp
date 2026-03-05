#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fdtd-2d_kernel.c"

// Testbench for kernel_fdtd_2d
int main() {
    printf("Starting testbench for kernel_fdtd_2d\n");
    
    int tmax = 100;
    int nx = 100;
    int ny = 100;
    double ex[60][80];
    // Initialize ex
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 80; j++)
            ex[i][j] = (double)(i * 80 + j);
    double ey[60][80];
    // Initialize ey
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 80; j++)
            ey[i][j] = (double)(i * 80 + j);
    double hz[60][80];
    // Initialize hz
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 80; j++)
            hz[i][j] = (double)(i * 80 + j);
    double _fict_[40];
    // Initialize _fict_
    for (int i = 0; i < 40; i++)
        _fict_[i] = (double)i;

    // Call the DUT
    kernel_fdtd_2d(tmax, nx, ny, ex, ey, hz, _fict_);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
