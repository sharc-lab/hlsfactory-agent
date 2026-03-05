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
constexpr int input_size = input_height * input_height * num_channel * num_channel;
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

constexpr int weight_size_kernel1 = (kernel_total_size1 * num_channel) * num_channel/16;
constexpr int weight_size_kernel2 = (kernel_total_size2 * num_channel) * num_channel/16;

using int16_v16 = tapa::vec_t<ap_int<16>, 16>;
using int16_v32 = tapa::vec_t<ap_int<16>, 32>;
using int16_v64 = tapa::vec_t<ap_int<16>, 64>;

void VAE(
    tapa::mmap<int16_v16> X,
    tapa::mmap<int16_v16> W1,
    tapa::mmap<int16_v16> W2,
    tapa::mmap<int16_v16> W3,
    tapa::mmap<int16_v16> W4,
    tapa::mmap<ap_int<32>> offchip_decoder_conv3,
    tapa::mmap<ap_int<32>> offchip_decoder_conv4,
    tapa::mmap<int> acc_out,
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
    
    aligned_vector<ap_int<16>> input(input_size);
    aligned_vector<ap_int<16>> weight1(weight_size_kernel1*16);
    aligned_vector<ap_int<16>> weight2(weight_size_kernel2*16);
    aligned_vector<ap_int<16>> weight3(weight_size_kernel1*16);
    aligned_vector<ap_int<16>> weight4(weight_size_kernel2*16);
    aligned_vector<ap_int<32>> offchip_decoder_conv3(num_channel*num_channel*hidden3_size*hidden3_size);
    aligned_vector<ap_int<32>> offchip_decoder_conv4(num_channel*num_channel*hidden4_size*hidden4_size);
    aligned_vector<int> acc_out(output_size);
    aligned_vector<int> cycle_count(1);

    int64_t kernel_time_ns = tapa::invoke(VAE, FLAGS_bitstream,
        tapa::read_only_mmap<ap_int<16>>(input).reinterpret<int16_v16>(),
        tapa::read_only_mmap<ap_int<16>>(weight1).reinterpret<int16_v16>(),
        tapa::read_only_mmap<ap_int<16>>(weight2).reinterpret<int16_v16>(),
        tapa::read_only_mmap<ap_int<16>>(weight3).reinterpret<int16_v16>(),
        tapa::read_only_mmap<ap_int<16>>(weight4).reinterpret<int16_v16>(),
        tapa::read_write_mmap<ap_int<32>>(offchip_decoder_conv3),
        tapa::read_write_mmap<ap_int<32>>(offchip_decoder_conv4),
        tapa::write_only_mmap<int>(acc_out),
        tapa::write_only_mmap<int>(cycle_count));

    std::cout << "Cycle count: " << cycle_count[0] << std::endl;
    std::cout << "Latency: " << kernel_time_ns * 1e-9 << " s" << std::endl;

    return 0;
}
