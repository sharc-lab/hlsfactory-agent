#include <iostream>
#include "oclErrorCodes.cpp"
int main() {
    // Attempt to call the top function; if it takes arguments, this will be a compile‑time error.
    // Users should adapt the testbench for the actual kernel signature.
    oclErrorCodes();
    std::cout << "Testbench executed.\n";
    return 0;
}
