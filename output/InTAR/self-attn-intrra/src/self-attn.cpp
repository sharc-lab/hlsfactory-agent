#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <bits/stdc++.h>

using namespace std;

typedef vector<vector<float>> Matrix;
constexpr int seq_len = 256;
constexpr int D = 1024;
constexpr int D_head = 1024;

// Kernel

// Helper function to perform matrix multiplication
void matMul(const Matrix& A, const Matrix& B, Matrix& result) {
    size_t rows = A.size();
    size_t cols = B[0].size();
    size_t common_dim = A[0].size();

    result.assign(rows, vector<float>(cols, 0));
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            for (size_t k = 0; k < common_dim; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Helper function to apply softmax to a vector
void softmax(const vector<float>& input, vector<float>& output) {
    output.resize(input.size());
    float sum = 0.0;

    for (size_t i = 0; i < input.size(); ++i) {
        output[i] = exp(input[i]);
        sum += output[i];
    }
    for (size_t i = 0; i < output.size(); ++i) {
        output[i] /= sum;
    }
}

// Helper function to apply softmax row-wise on a matrix
void softmax(const Matrix& input, Matrix& output) {
    output.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        softmax(input[i], output[i]);
    }
}

// Transpose a matrix
void transpose(const Matrix& A, Matrix& result) {
    size_t rows = A.size();
    size_t cols = A[0].size();
    result.assign(cols, vector<float>(rows, 0));
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[j][i] = A[i][j];
        }
    }
}

// Self-attention computation
void selfAttention(const Matrix& input, const Matrix& Wq, const Matrix& Wk, const Matrix& Wv, Matrix& output) {
    // Step 1: Compute Query, Key, and Value matrices
    Matrix Q, K, V;
    matMul(input, Wq, Q);
    matMul(input, Wk, K);
    matMul(input, Wv, V);

    // Step 2: Compute scaled dot-product attention scores
    Matrix K_transposed;
    transpose(K, K_transposed);
    Matrix scores;
    matMul(Q, K_transposed, scores);

    float scale = sqrt(K[0].size());
    for (size_t i = 0; i < scores.size(); ++i) {
        for (size_t j = 0; j < scores[i].size(); ++j) {
            scores[i][j] /= scale; // scaled attention
        }
    }

    // Step 3: Apply softmax to get attention weights
    Matrix attention_weights;
    softmax(scores, attention_weights);

    // Step 4: Compute the output as weighted sum of Value matrix
    matMul(attention_weights, V, output);
}


// Host

int main() {
    // Example input and weight matrices
    Matrix input;
    input.assign(seq_len, vector<float>(D, 0.1f));

    Matrix Wq;
    Wq.assign(D, vector<float>(D, 0.2f));

    Matrix Wk;
    Wk.assign(D, vector<float>(D, 0.1f));

    Matrix Wv;
    Wv.assign(D, vector<float>(D, 0.3f));

    // Compute self-attention
    Matrix output;
    selfAttention(input, Wq, Wk, Wv, output);

    return 0;
}
