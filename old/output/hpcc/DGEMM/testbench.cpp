#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

// Simple DGEMM kernel: C = alpha * A * B + beta * C
void dgemm_kernel(int m, int n, int k, 
                  double alpha, const double *A, int lda,
                  const double *B, int ldb,
                  double beta, double *C, int ldc) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < m; i++) {
            double sum = 0.0;
            for (int l = 0; l < k; l++) {
                sum += A[i + l * lda] * B[l + j * ldb];
            }
            C[i + j * ldc] = alpha * sum + beta * C[i + j * ldc];
        }
    }
}

// Testbench for DGEMM
int main() {
    const int N = 32;  // Small test size
    const double alpha = 1.0;
    const double beta = 0.0;
    
    // Allocate matrices
    double *A = (double*)malloc(N * N * sizeof(double));
    double *B = (double*)malloc(N * N * sizeof(double));
    double *C = (double*)malloc(N * N * sizeof(double));
    
    if (!A || !B || !C) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Initialize matrices
    for (int i = 0; i < N * N; i++) {
        A[i] = (double)(i % 10) / 10.0;
        B[i] = (double)(i % 7) / 7.0;
        C[i] = 0.0;
    }
    
    // Perform DGEMM: C = alpha * A * B + beta * C
    dgemm_kernel(N, N, N, alpha, A, N, B, N, beta, C, N);
    
    // Verify result
    // For A[i] = i%10/10, B[i] = i%7/7
    // Check a few elements
    int pass = 1;
    for (int i = 0; i < N && pass; i++) {
        for (int j = 0; j < N && pass; j++) {
            double expected = 0.0;
            for (int l = 0; l < N; l++) {
                expected += A[i + l * N] * B[l + j * N];
            }
            if (fabs(C[i + j * N] - expected) > 1e-10) {
                pass = 0;
            }
        }
    }
    
    printf("DGEMM Test %s\n", pass ? "PASSED" : "FAILED");
    
    free(A);
    free(B);
    free(C);
    
    return pass ? 0 : 1;
}
