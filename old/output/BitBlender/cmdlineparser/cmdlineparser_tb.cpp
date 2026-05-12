#include <iostream>
#include "cmdlineparser.cpp"
int main() {
    // Attempt to call the top function; if it takes arguments, this will be a compile‑time error.
    // Users should adapt the testbench for the actual kernel signature.
    cmdlineparser();
    std::cout << "Testbench executed.\n";
    return 0;
}
