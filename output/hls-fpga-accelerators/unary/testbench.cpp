#include "unary.h"
#include "common/config.h"
#include <iostream>
int main() {
    const uint64_t size = 8;
    RawDataT in[size];
    RawDataT out[size];
    for (uint64_t i = 0; i < size; ++i) {
        in[i] = i;
    }
    unary(in, out, size, 0);
    std::cout << "Unary testbench completed.\\n";
    return 0;
}
