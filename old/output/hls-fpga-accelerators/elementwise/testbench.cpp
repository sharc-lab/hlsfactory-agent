#include "elementwise.h"
#include "config.h"
#include <iostream>
int main() {
    const uint64_t size = 8;
    RawDataT in1[size];
    RawDataT in2[size];
    RawDataT out[size];
    for (uint64_t i = 0; i < size; ++i) {
        in1[i] = i;
        in2[i] = i * 2;
    }
    elementwise(in1, in2, out, size, 0); // OP_ADD
    std::cout << "Testbench completed." << std::endl;
    return 0;
}
