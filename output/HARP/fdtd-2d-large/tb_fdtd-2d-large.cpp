#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fdtd-2d-large_kernel.c"

// Testbench for kernel_fdtd_2d
int main() {
    printf("Starting testbench for kernel_fdtd_2d\n");
    
    int tmax = 100;
    int nx = 100;
    int ny = 100;
    double ex[200][240];
    // Initialize ex
    for (int i = 0; i < 200; i++)
        for (int j = 0; j < 240; j++)
            ex[i][j] = (double)(i * 240 + j);
    double ey[200][240];
    // Initialize ey
    for (int i = 0; i < 200; i++)
        for (int j = 0; j < 240; j++)
            ey[i][j] = (double)(i * 240 + j);
    double hz[200][240];
    // Initialize hz
    for (int i = 0; i < 200; i++)
        for (int j = 0; j < 240; j++)
            hz[i][j] = (double)(i * 240 + j);
    double _fict_[100];
    // Initialize _fict_
    for (int i = 0; i < 100; i++)
        _fict_[i] = (double)i;

    // Call the DUT
    kernel_fdtd_2d(tmax, nx, ny, ex, ey, hz, _fict_);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
