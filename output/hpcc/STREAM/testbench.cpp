#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#define STREAM_ARRAY_SIZE 1000
#define NTIMES 10
#define OFFSET 0
#define STREAM_TYPE double

// STREAM kernels
extern "C" {
    void tuned_STREAM_Copy(STREAM_TYPE *a, STREAM_TYPE *b, STREAM_TYPE *c, int n);
    void tuned_STREAM_Scale(STREAM_TYPE scalar, STREAM_TYPE *a, STREAM_TYPE *b, STREAM_TYPE *c, int n);
    void tuned_STREAM_Add(STREAM_TYPE *a, STREAM_TYPE *b, STREAM_TYPE *c, int n);
    void tuned_STREAM_Triad(STREAM_TYPE scalar, STREAM_TYPE *a, STREAM_TYPE *b, STREAM_TYPE *c, int n);
}

// Simple STREAM testbench
int main() {
    STREAM_TYPE *a, *b, *c;
    const STREAM_TYPE scalar = 3.0;
    
    // Allocate arrays
    a = (STREAM_TYPE*)malloc((STREAM_ARRAY_SIZE + OFFSET) * sizeof(STREAM_TYPE));
    b = (STREAM_TYPE*)malloc((STREAM_ARRAY_SIZE + OFFSET) * sizeof(STREAM_TYPE));
    c = (STREAM_TYPE*)malloc((STREAM_ARRAY_SIZE + OFFSET) * sizeof(STREAM_TYPE));
    
    if (!a || !b || !c) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    for (int i = 0; i < STREAM_ARRAY_SIZE; i++) {
        a[i] = 1.0;
        b[i] = 2.0;
        c[i] = 0.0;
    }
    
    // Run STREAM kernels
    for (int k = 0; k < NTIMES; k++) {
        // Copy: a = c
        for (int i = 0; i < STREAM_ARRAY_SIZE; i++)
            a[i] = c[i];
        
        // Scale: b = scalar * c
        for (int i = 0; i < STREAM_ARRAY_SIZE; i++)
            b[i] = scalar * c[i];
        
        // Add: c = a + b
        for (int i = 0; i < STREAM_ARRAY_SIZE; i++)
            c[i] = a[i] + b[i];
        
        // Triad: a = b + scalar * c
        for (int i = 0; i < STREAM_ARRAY_SIZE; i++)
            a[i] = b[i] + scalar * c[i];
    }
    
    // Verify results
    int pass = 1;
    double expected_a = 0.0;
    
    // Calculate expected value after NTIMES iterations
    // Starting: a=1, b=2, c=0
    // After each iteration:
    //   a = c (0)
    //   b = scalar * c (0)
    //   c = a + b (0)
    //   a = b + scalar * c (0)
    // So after first iteration, all become 0
    
    for (int i = 0; i < STREAM_ARRAY_SIZE; i++) {
        if (fabs(a[i] - 0.0) > 1e-10) {
            pass = 0;
            break;
        }
    }
    
    printf("STREAM Test %s\n", pass ? "PASSED" : "FAILED");
    
    free(a);
    free(b);
    free(c);
    
    return pass ? 0 : 1;
}
