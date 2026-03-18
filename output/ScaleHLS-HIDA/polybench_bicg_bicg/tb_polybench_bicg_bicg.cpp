#include <iostream>
#include "bicg.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_bicg_node0();
    std::cout << "Testbench executed for polybench_bicg_bicg\n";
    return 0;
}
