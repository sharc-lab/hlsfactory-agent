/*
FIXME: This host is from intrra host. 
*/

#include <vector>
#include <cmath>
#include <iostream>
#include <string>
#include <ctime>
#include <cmath>
#include <tapa.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <ap_int.h>

#define N 256
#define D 1024
#define VEC_LEN 16

constexpr int D_head = 1024;
constexpr int input_size = N * (D / VEC_LEN);
constexpr int output_size = N * D_head / 32;

using type_t = ap_int<16>;
using vec_t = tapa::vec_t<type_t, VEC_LEN>;

void selfAttention(
    tapa::mmap<vec_t> input,
    tapa::mmap<vec_t> WQ,
    tapa::mmap<vec_t> WK,
    tapa::mmap<vec_t> WV,
    tapa::mmap<vec_t> offchip_Q,
    tapa::mmap<vec_t> offchip_K,
    tapa::mmap<vec_t> offchip_V,
    tapa::mmap<vec_t> output,
    tapa::mmap<int> cycle_count
);

template <typename T>
using aligned_vector = std::vector<T, tapa::aligned_allocator<T>>;

DEFINE_string(bitstream, "", "path to bitstream file");


int main(int argc, char *argv[]){
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    srand((unsigned)time(nullptr));

    // Example input matrix (8x8)
    LOG(INFO) << "Initializing input matrix...";
    aligned_vector<type_t> input(N * D);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < D; ++j) {
            input[i * D + j] = type_t(1);
        }
    } 
    LOG(INFO) << "Input matrix initialized with all 1s";

    type_t WQ_primitive[D][D];
    type_t WK_primitive[D][D];
    type_t WV_primitive[D][D];
    for (int i = 0; i < D; i++){
        for (int j = 0; j < D; j++){
            WQ_primitive[i][j] = type_t(1);
            WK_primitive[i][j] = type_t(1);
            WV_primitive[i][j] = type_t(1);
        }
    }
    LOG(INFO) << "Weight matrices initialized with all 1s";
    aligned_vector<type_t> WQ(D * D);
    aligned_vector<type_t> WK(D * D);
    aligned_vector<type_t> WV(D * D);

    // transpose WQ_primitive, WK_primitive, WV_primitive
    for (int i = 0; i < D; i++){
        for (int j = 0; j < D; j++){
            WQ[j * D + i] = WQ_primitive[i][j];
            WK[j * D + i] = WK_primitive[i][j];
            WV[j * D + i] = WV_primitive[i][j];
        }
    }
    LOG(INFO) << "Weight matrices transposed";

    aligned_vector<int> cycle_count(1);

    aligned_vector<type_t> offchip_Q(N * D);
    aligned_vector<type_t> offchip_K(N * D);
    aligned_vector<type_t> offchip_V(N * D);
    aligned_vector<type_t> output(N * D);
    
    LOG(INFO) << "Offchip memory initialized";

    int64_t kernel_time_ns = tapa::invoke(selfAttention, FLAGS_bitstream,
        tapa::read_only_mmap<type_t>(input).reinterpret<vec_t>(),
        tapa::read_only_mmap<type_t>(WQ).reinterpret<vec_t>(),
        tapa::read_only_mmap<type_t>(WK).reinterpret<vec_t>(),
        tapa::read_only_mmap<type_t>(WV).reinterpret<vec_t>(),
        tapa::read_write_mmap<type_t>(offchip_Q).reinterpret<vec_t>(),
        tapa::read_write_mmap<type_t>(offchip_K).reinterpret<vec_t>(),
        tapa::read_write_mmap<type_t>(offchip_V).reinterpret<vec_t>(),
        tapa::write_only_mmap<type_t>(output).reinterpret<vec_t>(),
        tapa::write_only_mmap<int>(cycle_count)
    );

    LOG(INFO) << "Kernel invoked";

    std::cout << "Cycle count: " << cycle_count[0] << std::endl;
    std::cout << "Kernel time (ns): " << kernel_time_ns << std::endl;
    std::cout << "Kernel time (us): " << float(kernel_time_ns)/1000.0 << std::endl;
    std::cout << "Kernel time (ms): " << float(kernel_time_ns)/1000000.0 << std::endl;

    // for (int i = 0; i < N; i++){
    //     for (int j = 0; j < D; j++){
    //         std::cout << output[i * D + j] << ' ';
    //     }
    //     std::cout << '\n';
    // }

    return 0;
}