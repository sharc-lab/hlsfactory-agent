#include <iostream>
#include "syr2k.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_syr2k_node1();
    std::cout << "Testbench executed for polybench_syr2k_syr2k\n";
    return 0;
}
