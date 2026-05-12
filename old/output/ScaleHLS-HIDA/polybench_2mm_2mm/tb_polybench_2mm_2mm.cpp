#include <iostream>
#include "2mm.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_2mm_node0();
    std::cout << "Testbench executed for polybench_2mm_2mm\n";
    return 0;
}
