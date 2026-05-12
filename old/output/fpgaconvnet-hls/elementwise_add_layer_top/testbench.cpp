#include <iostream>
#include "elementwise_add_layer_top.cpp"

int main() {
    // Call the top function with default arguments.
    elementwise_add_layer_top();
    std::cout << "Testbench executed for elementwise_add_layer_top" << std::endl;
    return 0;
}
