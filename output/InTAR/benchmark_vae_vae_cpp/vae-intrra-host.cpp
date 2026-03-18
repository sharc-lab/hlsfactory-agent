#include <iostream>
#include <cmath>
#include <numeric>
#include <string>
#include <tapa.h>
#include <ap_int.h>
#include <hls_math.h>

// Hyperparameters
constexpr int num_channel = 2;
constexpr int input_height = 28; // Height of input image
constexpr int input_width = 28;  // Width of input image
constexpr int input_size = input_height * input_height * num_channel;
constexpr int kernel_size1 = 8;  // Kernel size for first convolution
constexpr int kernel_total_size1 = kernel_size1 * kernel_size1;
constexpr int kernel_size2 = 4;  // Kernel size for second convolution
constexpr int kernel_total_size2 = kernel_size2 * kernel_size2;
constexpr int hidden1_size = input_height - kernel_size1 + 1;
constexpr int hidden2_size = hidden1_size - kernel_size2 + 1;
constexpr int hidden3_size = hidden2_size + kernel_size1 - 1;
constexpr int hidden4_size = hidden3_size + kernel_size2 - 1;
constexpr int latent_dim = 324;   // Latent space dimensionality
constexpr int output_height = input_height;
constexpr int output_width = input_width;
constexpr int output_size = output_height * output_width * num_channel;

constexpr int weight_size_cc0 = (kernel_total_size1 + kernel_total_size2 + kernel_total_size1 * num_channel) * num_channel/16;
constexpr int weight_size_cc1 = (kernel_total_size1 + kernel_total_size2 + kernel_total_size2 * num_channel) * num_channel/16;


using int16_v16 = tapa::vec_t<ap_int<16>, 16>;
using int16_v32 = tapa::vec_t<ap_int<16>, 32>;
using int16_v64 = tapa::vec_t<ap_int<16>, 64>;

void VAE(
    tapa::mmap<int16_v16> X_acc0,
    tapa::mmap<int16_v16> X_acc1,
    tapa::mmap<int16_v16> W_acc0,
    tapa::mmap<int16_v16> W_acc1,
    tapa::mmap<int> acc1_out,
    tapa::mmap<int> cycle_count
);

template <typename T>
using aligned_vector = std::vector<T, tapa::aligned_allocator<T>>;

DEFINE_string(bitstream, "", "path to bitstream file");

int main(int argc, char *argv[]){
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    const int L = argc > 1 ? atoll(argv[1]) : input_size;

    srand((unsigned)time(nullptr));

    // Example input and weight matrices
    
    aligned_vector<ap_int<16>> input1(input_size);
    aligned_vector<ap_int<16>> input2(input_size);
    aligned_vector<ap_int<16>> weight_cc0(weight_size_cc0*16);
    aligned_vector<ap_int<16>> weight_cc1(weight_size_cc1*16);
    aligned_vector<int> acc1_out(output_size);
    aligned_vector<int> cycle_count(1);

    int64_t kernel_time_ns = tapa::invoke(VAE, FLAGS_bitstream,
        tapa::read_only_mmap<ap_int<16>>(input1).reinterpret<int16_v16>(),
        tapa::read_only_mmap<ap_int<16>>(input2).reinterpret<int16_v16>(),
        tapa::read_only_mmap<ap_int<16>>(weight_cc0).reinterpret<int16_v16>(),
        tapa::read_only_mmap<ap_int<16>>(weight_cc1).reinterpret<int16_v16>(),
        tapa::write_only_mmap<int>(acc1_out),
        tapa::write_only_mmap<int>(cycle_count));

    std::cout << "Cycle count: " << cycle_count[0] << std::endl;
    std::cout << "Latency: " << kernel_time_ns * 1e-9 << " s" << std::endl;

    return 0;
}