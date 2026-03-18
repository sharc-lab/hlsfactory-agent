#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

constexpr int input_size = 256;     // Number of input features
constexpr int hidden_size1 = 1024;   // Number of neurons in the first hidden layer
constexpr int hidden_size2 = 2048;   // Number of neurons in the second hidden layer
constexpr int output_size = 256;    // Number of output classes

// Helper function for ReLU activation
void relu(vector<float>& x) {
    for (float& val : x) {
        val = max(0.0f, val);
    }
}

// Helper function for softmax activation
void softmax(const vector<float>& logits, vector<float>& result) {
    float sum_exp = 0.0;
    for (float val : logits) {
        sum_exp += exp(val);
    }
    result.clear();
    for (float val : logits) {
        result.push_back(exp(val) / sum_exp);
    }
}

// Initialize weights with random values in [-0.5, 0.5]
void initialize_weights(vector<vector<float>>& weights, int rows, int cols) {
    srand(static_cast<unsigned>(time(0)));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            weights[i][j] = ((rand() % 100) / 100.0) - 0.5;
        }
    }
}

// Forward pass for one layer
void forward_layer(const vector<float>& input, const vector<vector<float>>& weights, const vector<float>& biases, vector<float>& output) {
    int num_neurons = weights.size(); // Number of neurons in the layer
    int num_inputs = weights[0].size(); // Number of inputs to the layer
    output.assign(num_neurons, 0.0);

    for (int i = 0; i < num_neurons; ++i) { // Loop over each neuron in the layer
        for (int j = 0; j < num_inputs; ++j) { // Loop over each input
            output[i] += weights[i][j] * input[j]; // Multiply input[j] by weights[i][j]
            // weights: [num_neurons x num_inputs]
        }
        output[i] += biases[i]; // Add bias
    }
    relu(output); // Apply ReLU activation
}

// Multilayer Perceptron: Forward pass with two hidden layers
void multilayer_perceptron(const vector<float>& input, 
                           const vector<vector<float>>& weights1, 
                           const vector<float>& biases1, 
                           const vector<vector<float>>& weights2, 
                           const vector<float>& biases2, 
                           const vector<vector<float>>& weights3, 
                           const vector<float>& biases3,
                           vector<float>& output) {
    // Forward pass for first hidden layer
    vector<float> hidden_output1;
    forward_layer(input, weights1, biases1, hidden_output1);
    // weights1: [hidden_size1 x input_size]
    std::cout << hidden_output1.size() << std::endl;

    // Forward pass for second hidden layer
    vector<float> hidden_output2;
    forward_layer(hidden_output1, weights2, biases2, hidden_output2);
    // weights2: [hidden_size2 x hidden_size1]

    std::cout << hidden_output2.size() << std::endl;


    // Forward pass for output layer
    vector<float> logits(biases3.size(), 0.0);
    for (int i = 0; i < biases3.size(); ++i) { // Loop over output neurons
        for (int j = 0; j < hidden_output2.size(); ++j) { // Loop over second hidden layer neurons
            logits[i] += weights3[i][j] * hidden_output2[j];
            // weights3: [output_size x hidden_size2]
        }
        logits[i] += biases3[i]; // Add bias
    }

    std::cout << logits.size() << std::endl;

    output = logits;
}

int main() {

    // Initialize weights and biases
    vector<vector<float>> weights1(hidden_size1, vector<float>(input_size, 0.2f));
    vector<float> biases1(hidden_size1, 0.0);

    vector<vector<float>> weights2(hidden_size2, vector<float>(hidden_size1, 1.3f));
    vector<float> biases2(hidden_size2, 0.0);

    vector<vector<float>> weights3(output_size, vector<float>(hidden_size2, 0.6f));
    vector<float> biases3(output_size, 0.0);

    initialize_weights(weights1, hidden_size1, input_size);
    initialize_weights(weights2, hidden_size2, hidden_size1);
    initialize_weights(weights3, output_size, hidden_size2);

    // Input example
    vector<float> input(input_size, 0.1f);

    // Forward pass
    vector<float> output;
    multilayer_perceptron(input, weights1, biases1, weights2, biases2, weights3, biases3, output);

    return 0;
}
