#include "rmsnorm.h"
#include "common/config.h"
#include <iostream>
int main() {
    const uint64_t size = 8;
    RawDataT in[size];
    RawDataT out[size];
    for (uint64_t i = 0; i < size; ++i) {
        in[i] = i;
    }
    rmsnorm(in, out, size);
    std::cout << "RMSNorm testbench completed.\\n";
    return 0;
}
