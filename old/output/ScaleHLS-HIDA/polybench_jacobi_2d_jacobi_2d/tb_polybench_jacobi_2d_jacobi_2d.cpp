#include <iostream>
#include "jacobi_2d.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_jacobi_2d_node1();
    std::cout << "Testbench executed for polybench_jacobi_2d_jacobi_2d\n";
    return 0;
}
