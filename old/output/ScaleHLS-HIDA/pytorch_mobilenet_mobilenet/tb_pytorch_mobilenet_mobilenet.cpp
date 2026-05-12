#include <iostream>
#include "mobilenet.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    forward_node1();
    std::cout << "Testbench executed for pytorch_mobilenet_mobilenet\n";
    return 0;
}
