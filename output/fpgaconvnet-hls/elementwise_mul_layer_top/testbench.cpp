#include <iostream>
#include "elementwise_mul_layer_top.cpp"

int main() {
    // Call the top function with default arguments.
    elementwise_mul_layer_top();
    std::cout << "Testbench executed for elementwise_mul_layer_top" << std::endl;
    return 0;
}
