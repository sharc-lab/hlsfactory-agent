#include <iostream>
#include "gesummv.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    kernel_gesummv_node0();
    std::cout << "Testbench executed for polybench_gesummv_gesummv\n";
    return 0;
}
