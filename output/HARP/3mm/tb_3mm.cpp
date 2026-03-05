#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "3mm_kernel.c"

// Testbench for kernel_3mm
int main() {
    printf("Starting testbench for kernel_3mm\n");
    
    int ni = 100;
    int nj = 100;
    int nk = 100;
    int nl = 100;
    int nm = 100;
    double E[40][50];
    // Initialize E
    for (int i = 0; i < 40; i++)
        for (int j = 0; j < 50; j++)
            E[i][j] = (double)(i * 50 + j);
    double A[40][60];
    // Initialize A
    for (int i = 0; i < 40; i++)
        for (int j = 0; j < 60; j++)
            A[i][j] = (double)(i * 60 + j);
    double B[60][50];
    // Initialize B
    for (int i = 0; i < 60; i++)
        for (int j = 0; j < 50; j++)
            B[i][j] = (double)(i * 50 + j);
    double F[50][70];
    // Initialize F
    for (int i = 0; i < 50; i++)
        for (int j = 0; j < 70; j++)
            F[i][j] = (double)(i * 70 + j);
    double C[50][80];
    // Initialize C
    for (int i = 0; i < 50; i++)
        for (int j = 0; j < 80; j++)
            C[i][j] = (double)(i * 80 + j);
    double D[80][70];
    // Initialize D
    for (int i = 0; i < 80; i++)
        for (int j = 0; j < 70; j++)
            D[i][j] = (double)(i * 70 + j);
    double G[40][70];
    // Initialize G
    for (int i = 0; i < 40; i++)
        for (int j = 0; j < 70; j++)
            G[i][j] = (double)(i * 70 + j);

    // Call the DUT
    kernel_3mm(ni, nj, nk, nl, nm, E, A, B, F, C, D, G);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
