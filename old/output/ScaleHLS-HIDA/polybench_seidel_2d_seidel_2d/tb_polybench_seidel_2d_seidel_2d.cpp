#include <iostream>
#include "seidel_2d.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_seidel_2d_node0();
    std::cout << "Testbench executed for polybench_seidel_2d_seidel_2d\n";
    return 0;
}
