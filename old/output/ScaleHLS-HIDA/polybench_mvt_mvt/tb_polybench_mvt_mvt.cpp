#include <iostream>
#include "mvt.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_mvt_node0();
    std::cout << "Testbench executed for polybench_mvt_mvt\n";
    return 0;
}
