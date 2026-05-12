#include "softmax.h"
#include "common/config.h"
#include <iostream>
int main() {
    const uint64_t size = 8;
    RawDataT in[size];
    RawDataT out[size];
    for (uint64_t i = 0; i < size; ++i) {
        in[i] = i;
    }
    softmax(in, out, size);
    std::cout << "Softmax testbench completed.\\n";
    return 0;
}
