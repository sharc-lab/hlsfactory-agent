#include "knn.cpp"
int main() {
    tapa::mmap<INTERFACE_WIDTH> dummy_input;
    tapa::mmap<INT32> dummy_output;
    Knn(dummy_input, dummy_output);
    return 0;
}
EOF && clang++ -std=c++17 -I/workspace/stubs -I/output/CHIP-KNN/singlePE -w -fsyntax-only /output/CHIP-KNN/singlePE/knn.cpp /output/CHIP-KNN/singlePE/testbench.cpp > /output/CHIP-KNN/singlePE/compile_log.txt 2>&1
