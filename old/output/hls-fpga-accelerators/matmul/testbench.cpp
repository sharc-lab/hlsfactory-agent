#include "matmul.h"
#include "common/config.h"
#include <iostream>
int main() {
    const uint64_t size = 8;
    RawDataT A[size];
    RawDataT B[size];
    RawDataT C[size];
    for (uint64_t i = 0; i < size; ++i) {
        A[i] = i;
        B[i] = i * 2;
    }
    matmul(A, B, C, size);
    std::cout << "Matmul testbench done.\\n";
    return 0;
}
