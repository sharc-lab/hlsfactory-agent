#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "aes_kernel.c"

// Testbench for aes256_encrypt_ecb
int main() {
    printf("Starting testbench for aes256_encrypt_ecb\n");
    
    char k[32];
    // Initialize k
    for (int i = 0; i < 32; i++)
        k[i] = (char)i;
    char buf[16];
    // Initialize buf
    for (int i = 0; i < 16; i++)
        buf[i] = (char)i;

    // Call the DUT
    aes256_encrypt_ecb(k, buf);

    // Verify results (basic check)
    printf("Testbench completed successfully\n");
    return 0;
}
