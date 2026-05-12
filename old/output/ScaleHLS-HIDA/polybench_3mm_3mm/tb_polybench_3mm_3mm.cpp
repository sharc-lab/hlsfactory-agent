#include <iostream>
#include "3mm.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_3mm_node0();
    std::cout << "Testbench executed for polybench_3mm_3mm\n";
    return 0;
}
