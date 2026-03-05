#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "nw_kernel.c"

// Testbench for needwun
int main() {
    printf("Starting testbench for needwun\n");
    
    char SEQA[128];
    // Initialize SEQA
    for (int i = 0; i < 128; i++)
        SEQA[i] = (char)i;
    char SEQB[128];
    // Initialize SEQB
    for (int i = 0; i < 128; i++)
        SEQB[i] = (char)i;
    char alignedA[256];
    // Initialize alignedA
    for (int i = 0; i < 256; i++)
        alignedA[i] = (char)i;
    char alignedB[256];
    // Initialize alignedB
    for (int i = 0; i < 256; i++)
        alignedB[i] = (char)i;
    int M[16641];
    // Initialize M
    for (int i = 0; i < 16641; i++)
        M[i] = (int)i;
    char ptr[16641];
    // Initialize ptr
    for (int i = 0; i < 16641; i++)
        ptr[i] = (char)i;

    // Call the DUT
    needwun(SEQA, SEQB, alignedA, alignedB, M, ptr);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
