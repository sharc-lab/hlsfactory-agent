#include "lstm_inference.c"
#include <stdio.h>

int main() {
    // Example fixed‑point input data (using ap_fixed representation if needed)
    const int input_len = 4;
    // For simplicity, use plain float literals; the inference code handles conversion internally.
    float input[input_len] = {0.1f, 0.2f, 0.3f, 0.4f};

    // Output buffer
    float output[input_len];

    // Call the inference function (assumes signature: void lstm_inference(const float*, float*, int))
    lstm_inference(input, output, input_len);

    // Print results
    printf("Fixed‑point LSTM inference results:\\n");
    for (int i = 0; i < input_len; ++i) {
        printf("%f\\n", output[i]);
    }
    return 0;
}
