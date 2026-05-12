#include "matmul.h"
#include "../common/config.h"

void matmul(RawDataT *A, RawDataT *B, RawDataT *C, uint64_t size) {
    // Stub implementation: simply copy A to C
    for (uint64_t i = 0; i < size; ++i) {
        C[i] = A[i];
    }
}
