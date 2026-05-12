#include <iostream>
extern "C" {
    void vertexApply(...);
}
int main() {
    std::cout << "Running dummy testbench for vertexApply\n";
    vertexApply();
    return 0;
}
