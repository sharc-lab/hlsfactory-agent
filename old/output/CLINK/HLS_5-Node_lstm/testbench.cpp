#include "lstm_n5_16s_16b.h"
#include <iostream>
#include <vector>

// Simple reference values (example)
static const int INPUT_SIZE = 4;
static const ap_int<16> sample_input[INPUT_SIZE] = {1, 2, 3, 4};

int main() {
    // Allocate input and output buffers
    ap_int<16> input[INPUT_SIZE];
    ap_int<16> output[INPUT_SIZE];

    // Initialize inputs
    for (int i = 0; i < INPUT_SIZE; ++i) {
        input[i] = sample_input[i];
    }

    // Call the top‑level LSTM function (adjust name if needed)
    lstm_n5_16s_16b(input, output);

    // Simple verification: print results
    std::cout << "LSTM output:\\n";
    for (int i = 0; i < INPUT_SIZE; ++i) {
        std::cout << output[i] << std::endl;
    }

    // In a real test you would compare against expected values
    return 0;
}
