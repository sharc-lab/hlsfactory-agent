#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>

// Helper typedef for 2D matrix
typedef std::vector<std::vector<float>> Matrix;

constexpr int image_size = 224;
constexpr int kernel_size = 4;

// Function to print a matrix (for debugging)
void printMatrix(const Matrix& mat) {
    for (const auto& row : mat) {
        for (float val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

// Perform 2D convolution
void convolve(const Matrix& input, const Matrix& kernel, Matrix& output) {
    int inputSize = input.size();
    int kernelSize = kernel.size();
    int outputSize = (input[0].size() - kernelSize) + 1;

    output.resize(outputSize, std::vector<float>(outputSize, 0));

    for (int i = 0; i < outputSize; ++i) {
        for (int j = 0; j < outputSize; ++j) {
            for (int m = 0; m < kernelSize; ++m) {
                for (int n = 0; n < kernelSize; ++n) {
                    output[i][j] += input[i + m][j + n] * kernel[m][n];
                }
            }
        }
    }
}

// Apply ReLU activation
void relu(const Matrix& input, Matrix& output) {
    output = input;
    for (auto& row : output) {
        for (auto& val : row) {
            val = std::max(0.0f, val);
        }
    }
}

// Upsample by a factor of 2
void upsample(const Matrix& input, Matrix& output) {
    int inputSize = input.size();
    int outputSize = inputSize * 2;
    output.resize(outputSize, std::vector<float>(outputSize, 0));

    for (int i = 0; i < inputSize; ++i) {
        for (int j = 0; j < inputSize; ++j) {
            output[2 * i][2 * j] = input[i][j];
            output[2 * i + 1][2 * j] = input[i][j];
            output[2 * i][2 * j + 1] = input[i][j];
            output[2 * i + 1][2 * j + 1] = input[i][j];
        }
    }
}

// Max pooling with 2x2 filter and stride 2
void maxPool(const Matrix& input, Matrix& output) {
    int inputSize = input.size();
    int outputSize = inputSize / 2;
    output.resize(outputSize, std::vector<float>(outputSize, 0));

    for (int i = 0; i < outputSize; ++i) {
        for (int j = 0; j < outputSize; ++j) {
            float maxVal = 0;
            for (int m = 0; m < 2; ++m) {
                for (int n = 0; n < 2; ++n) {
                    maxVal = std::max(maxVal, input[2 * i + m][2 * j + n]);
                }
            }
            output[i][j] = maxVal;
        }
    }
}

void CNN4L(const Matrix& input, const Matrix& weight1, const Matrix& weight2, const Matrix& weight3, const Matrix& weight4, Matrix& output){
    Matrix layer1, layer2, layer3, temp1, temp2, temp3, temp4;
    convolve(input, weight1, temp1);
    relu(temp1, temp1);
    upsample(temp1, layer1);

    std::cout << layer1.size() * layer1[0].size() * 4 << std::endl;

    convolve(layer1, weight2, temp2);
    relu(temp2, temp2);
    upsample(temp2, layer2);

    std::cout << layer2.size() * layer2[0].size() * 4 << std::endl;

    convolve(layer2, weight3, temp3);
    relu(temp3, temp3);
    maxPool(temp3, layer3);

    std::cout << layer3.size() * layer3[0].size() * 4 << std::endl;

    convolve(layer3, weight4, temp4);
    relu(temp4, temp4);
    maxPool(temp4, output);

    std::cout << output.size() * output[0].size() * 4 << std::endl;
}

void init_kernel(Matrix& kernel, int height, int width){
    kernel.resize(height, std::vector<float>(width, 0));
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            kernel[i][j] = ((rand() % 100) / 100.0) - 0.5;
        }
    }
}

// Main function demonstrating the 4-layer CNN
int main() {
    // Example input matrix (8x8)
    Matrix input;
    input.assign(image_size, std::vector<float>(image_size, 0.3f));

    Matrix kernel1, kernel2, kernel3, kernel4;
    init_kernel(kernel1, kernel_size, kernel_size);
    init_kernel(kernel2, kernel_size, kernel_size);
    init_kernel(kernel3, kernel_size, kernel_size);
    init_kernel(kernel4, kernel_size, kernel_size);

    Matrix output;

    CNN4L(input, kernel1, kernel2, kernel3, kernel4, output);

    std::cout << "finish" << std::endl;

    return 0;
}
