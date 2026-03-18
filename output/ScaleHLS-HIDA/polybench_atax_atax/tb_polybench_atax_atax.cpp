#include <iostream>
#include "atax.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_atax_node1();
    std::cout << "Testbench executed for polybench_atax_atax\n";
    return 0;
}
