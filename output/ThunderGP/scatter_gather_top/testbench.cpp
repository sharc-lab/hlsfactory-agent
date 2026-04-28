#include <iostream>
extern "C" {
    void scatter_gather_top_top(...);
}
int main() {
    std::cout << "Running dummy testbench for scatter_gather_top_top\n";
    scatter_gather_top_top();
    return 0;
}
