#include <iostream>
#include "inner_product_layer_top.cpp"

int main() {
    // Call the top function with default arguments.
    inner_product_layer_top();
    std::cout << "Testbench executed for inner_product_layer_top" << std::endl;
    return 0;
}
