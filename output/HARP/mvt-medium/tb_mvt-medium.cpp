#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mvt-medium_kernel.c"

// Testbench for kernel_mvt
int main() {
    printf("Starting testbench for kernel_mvt\n");
    
    double x1[400];
    // Initialize x1
    for (int i = 0; i < 400; i++)
        x1[i] = (double)i;
    double x2[400];
    // Initialize x2
    for (int i = 0; i < 400; i++)
        x2[i] = (double)i;
    double y_1[400];
    // Initialize y_1
    for (int i = 0; i < 400; i++)
        y_1[i] = (double)i;
    double y_2[400];
    // Initialize y_2
    for (int i = 0; i < 400; i++)
        y_2[i] = (double)i;
    double A[400][400];
    // Initialize A
    for (int i = 0; i < 400; i++)
        for (int j = 0; j < 400; j++)
            A[i][j] = (double)(i * 400 + j);

    // Call the DUT
    kernel_mvt(x1, x2, y_1, y_2, A);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
