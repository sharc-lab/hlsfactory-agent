#include <iostream>
#include "vgg16.cpp"

int main() {
    // Call the top function with dummy arguments (if any)
    // The testbench uses default‑constructed arguments; adjust as needed.
    forward_node1();
    std::cout << "Testbench executed for pytorch_vgg16_vgg16\n";
    return 0;
}
