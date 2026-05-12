#include <iostream>
#include "correlation.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_correlation_node0();
    std::cout << "Testbench executed for polybench_correlation_correlation\n";
    return 0;
}
