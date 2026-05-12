#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

// Hyperparameters
const int input_height = 28; // Height of input image
const int input_width = 28;  // Width of input image
const int kernel_size1 = 8;  // Kernel size for first convolution
const int kernel_size2 = 4;  // Kernel size for second convolution
const int latent_dim = 324;   // Latent space dimensionality
const int output_height = input_height;
const int output_width = input_width;

// Activation function (ReLU)
void relu(std::vector<std::vector<float>>& input) {
    for (auto& row : input) {
        for (auto& val : row) {
            val = std::max(0.0f, val);
        }
    }
}

// Sigmoid activation function
void sigmoid(std::vector<std::vector<float>>& input) {
    for (auto& row : input) {
        for (auto& val : row) {
            val = 1.0f / (1.0f + std::exp(-val));
        }
    }
}

// 2D Convolution operation
void conv2d(const std::vector<std::vector<float>>& input, std::vector<std::vector<float>>& output, 
            const std::vector<float>& kernel, float bias, 
            int input_h, int input_w, int kernel_size) {
    int output_h = input_h - kernel_size + 1;
    int output_w = input_w - kernel_size + 1;

    output.assign(output_h, std::vector<float>(output_w, bias));

    for (int i = 0; i < output_h; ++i) {
        for (int j = 0; j < output_w; ++j) {
            for (int ki = 0; ki < kernel_size; ++ki) {
                for (int kj = 0; kj < kernel_size; ++kj) {
                    output[i][j] += input[i + ki][j + kj] * kernel[ki * kernel_size + kj];
                }
            }
        }
    }

    std::cout << "conv2d output size: " << output_h * output_w << " elements\n";
}

// 2D Transpose Convolution operation
void conv2d_transpose(const std::vector<std::vector<float>>& input, std::vector<std::vector<float>>& output, 
                      const std::vector<float>& kernel, float bias, 
                      int input_h, int input_w, int kernel_size, int output_h, int output_w) {
    output.assign(output_h, std::vector<float>(output_w, bias));

    for (int i = 0; i < input_h; ++i) {
        for (int j = 0; j < input_w; ++j) {
            for (int ki = 0; ki < kernel_size; ++ki) {
                for (int kj = 0; kj < kernel_size; ++kj) {
                    int oi = i + ki;
                    int oj = j + kj;
                    if (oi < output_h && oj < output_w) {
                        output[oi][oj] += input[i][j] * kernel[ki * kernel_size + kj];
                    }
                }
            }
        }
    }

    std::cout << "conv2d_transpose output size: " << output_h * output_w << " elements\n";
}

// Encoder: Input -> Convolutional layers -> Latent space
void encoder(const std::vector<std::vector<float>>& input, std::vector<float>& latent_mean, std::vector<float>& latent_log_var) {
    int hidden1_h = input_height - kernel_size1 + 1;
    int hidden1_w = input_width - kernel_size1 + 1;
    int hidden2_h = hidden1_h - kernel_size2 + 1;
    int hidden2_w = hidden1_w - kernel_size2 + 1;

    std::vector<std::vector<float>> hidden1, hidden2;

    // Kernels for convolution layers (initialize with small random values)
    std::vector<float> kernel1(kernel_size1 * kernel_size1, 0.01f);
    std::vector<float> kernel2(kernel_size2 * kernel_size2, 0.01f);
    float bias1 = 0.1f;
    float bias2 = 0.1f;

    // First convolution
    conv2d(input, hidden1, kernel1, bias1, input_height, input_width, kernel_size1);
    relu(hidden1);

    // Second convolution
    conv2d(hidden1, hidden2, kernel2, bias2, hidden1_h, hidden1_w, kernel_size2);
    relu(hidden2);

    // Latent space
    for (int i = 0; i < latent_dim; ++i) {
        latent_mean[i] = hidden2[i / hidden2_w][i % hidden2_w] * 0.05f;
        latent_log_var[i] = hidden2[i / hidden2_w][i % hidden2_w] * 0.01f;
    }
}

// Decoder: Latent -> Convolutional transpose layers -> Output
void decoder(const std::vector<float>& latent_sample, std::vector<std::vector<float>>& output) {
    // Map latent_sample (1D) to a 2D representation
    int latent_h = 18; // Fixed height
    int latent_w = latent_dim / latent_h; // Ensure dimensions match latent_dim
    if (latent_h * latent_w != latent_dim) {
        std::cerr << "Error: latent_dim must be divisible by " << latent_h << "\n";
        return;
    }
    std::vector<std::vector<float>> latent_2d(latent_h, std::vector<float>(latent_w, 0));
    for (int i = 0; i < latent_dim; ++i) {
        latent_2d[i / latent_w][i % latent_w] = latent_sample[i];
    }

    int hidden1_h = latent_h + kernel_size1 - 1;
    int hidden1_w = latent_w + kernel_size1 - 1;
    int hidden2_h = hidden1_h + kernel_size2 - 1;
    int hidden2_w = hidden1_w + kernel_size2 - 1;

    std::vector<std::vector<float>> hidden1, hidden2;

    // Kernels for convolution transpose layers (initialize with small random values)
    std::vector<float> kernel1(kernel_size1 * kernel_size1, 0.01f);
    std::vector<float> kernel2(kernel_size2 * kernel_size2, 0.01f);
    float bias1 = 0.1f;
    float bias2 = 0.1f;

    // First convolution transpose
    conv2d_transpose(latent_2d, hidden1, kernel1, bias1, latent_h, latent_w, kernel_size1, hidden1_h, hidden1_w);
    relu(hidden1);

    // Second convolution transpose
    conv2d_transpose(hidden1, hidden2, kernel2, bias2, hidden1_h, hidden1_w, kernel_size2, hidden2_h, hidden2_w);
    relu(hidden2);

    // Final transformation to match output size
    output.assign(output_height, std::vector<float>(output_width, 0));
    for (int i = 0; i < output_height; ++i) {
        for (int j = 0; j < output_width; ++j) {
            output[i][j] = hidden2[i % hidden2_h][j % hidden2_w] * 0.01f + 0.1f;
        }
    }
    sigmoid(output);
}

// Sampling latent vector using reparameterization trick
void sample_latent(const std::vector<float>& latent_mean, const std::vector<float>& latent_log_var, std::vector<float>& latent_sample) {
    for (int i = 0; i < latent_dim; ++i) {
        float epsilon = static_cast<float>(rand()) / RAND_MAX; // Random noise
        latent_sample[i] = latent_mean[i] + std::exp(0.5f * latent_log_var[i]) * epsilon;
    }
}

void VAE(const std::vector<std::vector<float>>& input, std::vector<std::vector<float>>& output) {
    std::vector<float> latent_mean(latent_dim, 0);
    std::vector<float> latent_log_var(latent_dim, 0);
    std::vector<float> latent_sample(latent_dim, 0);

    // Forward pass
    encoder(input, latent_mean, latent_log_var);
    sample_latent(latent_mean, latent_log_var, latent_sample);
    decoder(latent_sample, output);
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    std::vector<std::vector<float>> input(input_height, std::vector<float>(input_width));
    for (int i = 0; i < input_height; ++i) {
        for (int j = 0; j < input_width; ++j) {
            input[i][j] = static_cast<float>(rand()) / RAND_MAX;
        }
    }

    std::vector<std::vector<float>> output;

    VAE(input, output);

    // Output results
    // std::cout << "Reconstructed output: \n";
    // for (const auto& row : output) {
    //     for (const auto& val : row) {
    //         std::cout << val << " ";
    //     }
    //     std::cout << std::endl;
    // }

    return 0;
}
