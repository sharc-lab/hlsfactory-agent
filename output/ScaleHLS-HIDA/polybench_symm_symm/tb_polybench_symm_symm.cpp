#include <iostream>
#include "symm.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_symm_node0();
    std::cout << "Testbench executed for polybench_symm_symm\n";
    return 0;
}
