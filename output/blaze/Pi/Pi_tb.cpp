#include "Pi.cpp"
#include <cstdlib>
#include <cstring>
int main() {
    // Instantiate Pi task
    Pi pi_task;
    // Allocate dummy input (2 doubles per data point)
    const int num_data = 10;
    double *input = (double*)std::malloc(sizeof(double) * num_data * 2);
    // Fill with random values
    for (int i = 0; i < num_data * 2; ++i) input[i] = (double)std::rand() / RAND_MAX;
    // Allocate dummy output (2 uint32_t)
    uint32_t *output = (uint32_t*)std::malloc(sizeof(uint32_t) * 2);
    // Set internal pointers using the stub methods (they are no‑ops, so we just invoke compute)
    pi_task.compute();
    // Clean up
    std::free(input);
    std::free(output);
    return 0;
}
