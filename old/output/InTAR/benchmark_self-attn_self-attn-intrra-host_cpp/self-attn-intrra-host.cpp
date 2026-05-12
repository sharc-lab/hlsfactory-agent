#include <vector>
#include <cmath>
#include <iostream>
#include <string>
#include <ctime>
#include <cmath>
#include <tapa.h>
#include <gflags/gflags.h>
#include <ap_int.h>

constexpr int seq_len = 256;
constexpr int D = 1024;
constexpr int D_head = 1024;
constexpr int D_head_div_2 = D_head / 2;
constexpr int weight_size_cc0 = D * D_head / 16;
constexpr int weight_size_cc1 = D * D_head / 8;
constexpr int input_size = seq_len * D / 16;
constexpr int output_size = seq_len * D_head / 32;

using int16_v16 = tapa::vec_t<ap_int<16>, 16>;

void selfAttention(
    tapa::mmap<int16_v16> X_acc0,
    tapa::mmap<int16_v16> X_acc1,
    tapa::mmap<int16_v16> W_acc0,
    tapa::mmap<int16_v16> W_acc1,
    tapa::mmap<int16_v16> acc0_out,
    tapa::mmap<int16_v16> acc1_out,
    tapa::mmap<int> cycle_count
);

template <typename T>
using aligned_vector = std::vector<T, tapa::aligned_allocator<T>>;

DEFINE_string(bitstream, "", "path to bitstream file");

int main(int argc, char *argv[]){
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    const int L = argc > 1 ? atoll(argv[1]) : seq_len;

    srand((unsigned)time(nullptr));

    // Example input and weight matrices
    
    aligned_vector<ap_int<16>> input1(seq_len * D);
    aligned_vector<ap_int<16>> input2(seq_len * D);
    aligned_vector<ap_int<16>> weight_cc0(weight_size_cc0*16);
    aligned_vector<ap_int<16>> weight_cc1(weight_size_cc1*16);
    aligned_vector<ap_int<16>> acc0_out(seq_len * D_head / 2);
    aligned_vector<ap_int<16>> acc1_out(seq_len * D_head / 2);
    aligned_vector<int> cycle_count(1);

    int64_t kernel_time_ns = tapa::invoke(selfAttention, FLAGS_bitstream,
        tapa::read_only_mmap<ap_int<16>>(input1).reinterpret<int16_v16>(),
        tapa::read_only_mmap<ap_int<16>>(input2).reinterpret<int16_v16>(),
        tapa::read_only_mmap<ap_int<16>>(weight_cc0).reinterpret<int16_v16>(),
        tapa::read_only_mmap<ap_int<16>>(weight_cc1).reinterpret<int16_v16>(),
        tapa::write_only_mmap<ap_int<16>>(acc0_out).reinterpret<int16_v16>(),
        tapa::write_only_mmap<ap_int<16>>(acc1_out).reinterpret<int16_v16>(),
        tapa::write_only_mmap<int>(cycle_count));

    std::cout << "Cycle count: " << cycle_count[0] << std::endl;
    std::cout << "Latency: " << kernel_time_ns * 1e-9 << " s" << std::endl;

    return 0;
}