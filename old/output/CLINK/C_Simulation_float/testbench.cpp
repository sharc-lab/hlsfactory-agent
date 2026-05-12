#include "lstm_inference.c"
#include <stdio.h>

int main() {
    // Example input data (adjust size as needed)
    const int input_len = 4;
    float input[input_len] = {0.1f, 0.2f, 0.3f, 0.4f};

    // Output buffer
    float output[input_len];

    // Call the inference function (assumes signature: void lstm_inference(const float*, float*, int))
    lstm_inference(input, output, input_len);

    // Print results
    printf("LSTM inference results:\\n");
    for (int i = 0; i < input_len; ++i) {
        printf("%f\\n", output[i]);
    }
    return 0;
}
