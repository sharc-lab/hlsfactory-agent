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
#include <ap_int.h>

#define VEC_LEN 8
constexpr int N = 256;
constexpr int D = 1024;
constexpr int D_head = 1024;
constexpr int D_head_div_2 = D_head / 2;
constexpr int weight_size_cc0 = D * D_head / 16;
constexpr int weight_size_cc1 = D * D_head / 8;
constexpr int input_size = N * D / 16;
constexpr int output_size = N * D_head / 32;

using type_t = ap_int<16>;
using vec_t = tapa::vec_t<type_t, VEC_LEN>;

void selfAttention(
    tapa::mmap<vec_t> input_q,
    tapa::mmap<vec_t> input_k,
    tapa::mmap<vec_t> input_v,
    tapa::mmap<vec_t> Wq, 
    tapa::mmap<vec_t> Wk, 
    tapa::mmap<vec_t> Wv, 
    tapa::mmap<vec_t> top_output,
    tapa::mmap<int> cycle_count
);

template <typename T>
using aligned_vector = std::vector<T, tapa::aligned_allocator<T>>;

DEFINE_string(bitstream, "", "path to bitstream file");

int main(int argc, char *argv[]){
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    // google::InitGoogleLogging(argv[0]);

    // FLAGS_log_dir = "./csim_logs";  // Specify the directory for log files
    
    // // Optional configurations:
    // FLAGS_logtostderr = false;      // Don't log to stderr
    // FLAGS_alsologtostderr = false; 
    
    const int L = argc > 1 ? atoll(argv[1]) : N;

    srand((unsigned)time(nullptr));

    // Example input and weight matrices
    LOG(INFO) << "Initializing input matrix...";
    aligned_vector<type_t> input_q(N * D);
    aligned_vector<type_t> input_k(N * D);
    aligned_vector<type_t> input_v(N * D);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < D; ++j) {
            input_q[i * D + j] = type_t(1);
            input_k[i * D + j] = type_t(1);
            input_v[i * D + j] = type_t(1);
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


    aligned_vector<type_t> output(N * D_head);
    aligned_vector<int> cycle_count(1);

    int64_t kernel_time_ns = tapa::invoke(selfAttention, FLAGS_bitstream,
        tapa::read_only_mmap<type_t>(input_q).reinterpret<vec_t>(),
        tapa::read_only_mmap<type_t>(input_k).reinterpret<vec_t>(),
        tapa::read_only_mmap<type_t>(input_v).reinterpret<vec_t>(),
        tapa::read_only_mmap<type_t>(WQ).reinterpret<vec_t>(),
        tapa::read_only_mmap<type_t>(WK).reinterpret<vec_t>(),
        tapa::read_only_mmap<type_t>(WV).reinterpret<vec_t>(),
        tapa::write_only_mmap<type_t>(output).reinterpret<vec_t>(),
        tapa::write_only_mmap<int>(cycle_count));

    std::cout << "Cycle count: " << cycle_count[0] << std::endl;
    std::cout << "Latency: " << kernel_time_ns * 1e-9 << " s" << std::endl;

    // std::cout << "Output:" << std::endl;
    // for (int i = 0; i < N; i++) {
    //     for (int j = 0; j < D; j++) {
    //         std::cout << output[i * D + j] << " ";
    //     }
    //     std::cout << std::endl; 
    // }

    return 0;
}