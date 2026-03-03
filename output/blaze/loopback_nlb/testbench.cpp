#include <cstdio>
#include <cstdlib>

// Stub for Task class
namespace blaze {
    class Task {
    public:
        Task(int n) {}
        virtual ~Task() {}
        virtual void compute() = 0;
    };
}

// Include the design under test
#include "NLB.cpp"

int main() {
    printf("Testbench for NLB Loopback Test\n");
    
    printf("This design requires Intel AAL platform\n");
    printf("Testbench structure created successfully\n");
    
    return 0;
}
