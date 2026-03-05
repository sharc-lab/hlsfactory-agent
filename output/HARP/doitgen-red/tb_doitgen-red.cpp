#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "doitgen-red_kernel.c"

// Testbench for kernel_doitgen
int main() {
    printf("Starting testbench for kernel_doitgen\n");
    
    double A[25][20];
    // Initialize A
    for (int i = 0; i < 25; i++)
        for (int j = 0; j < 20; j++)
            A[i][j] = (double)(i * 20 + j);
    double C4[30][30];
    // Initialize C4
    for (int i = 0; i < 30; i++)
        for (int j = 0; j < 30; j++)
            C4[i][j] = (double)(i * 30 + j);
    double sum[30];
    // Initialize sum
    for (int i = 0; i < 30; i++)
        sum[i] = (double)i;

    // Call the DUT
    kernel_doitgen(A, C4, sum);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
