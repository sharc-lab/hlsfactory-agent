#include <vector>
#include <cmath>
#include <iostream>
#include <string>
#include <ctime>
#include <cmath>
#include <tapa.h>
#include <gflags/gflags.h>
#include <ap_int.h>

constexpr int D = 1024;
constexpr int D_ffn = 5504;
constexpr int N_head = 16;
constexpr int MAX_SEQ_LEN = 1024;
constexpr int NUM_SLR = 3;
constexpr int NUM_DUM_SLR = 4;
constexpr int D_head = D / N_head;
constexpr int FFN_WEIGHT_SIZE = D * D_ffn;
constexpr int OUT_WEIGHT_SIZE = D * D;
constexpr int QKV_WEIGHT_SIZE = D * D / N_head * NUM_DUM_SLR * 2; // multi-head attention

using std::vector;
using int_v16 = tapa::vec_t<int, 16>;
using int4_v128 = tapa::vec_t<ap_int<4>, 128>;
using int8_v64 = tapa::vec_t<ap_int<8>, 64>;

void opt_kernel(
    const int L,
    const int L_out,
    const int seq_len,
    // tapa::mmap<int> inst, // inst[0] = L, inst[1] = reload_weight
    tapa::mmap<ap_uint<512>> X_acc0,
    tapa::mmap<ap_uint<512>> X_acc1,
    tapa::mmap<ap_uint<512>> W_acc0,
    tapa::mmap<ap_uint<512>> W_acc1,
    tapa::mmap<ap_uint<128>> acc0_out,
    // tapa::mmap<ap_uint<64>> acc1_out,
    tapa::mmap<int> cycle_count
);

template <typename T>
using aligned_vector = std::vector<T, tapa::aligned_allocator<T>>;

DEFINE_string(bitstream, "", "path to bitstream file");

int main(int argc, char *argv[]){
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    const int L = argc > 1 ? atoll(argv[1]) : MAX_SEQ_LEN;

    srand((unsigned)time(nullptr));

    // data preparation
    aligned_vector<int> inst = {L, 1};
    aligned_vector<ap_int<8>> X_acc0(L * D, 0);
    aligned_vector<ap_int<8>> X_acc1(L * D, 0);
    aligned_vector<ap_int<8>> W_acc0(D * D_head * NUM_DUM_SLR * 10 + D * D_ffn, 0);
    aligned_vector<ap_int<8>> W_acc1(D * D_head * NUM_DUM_SLR * 10 + D * D_ffn, 0);
    aligned_vector<ap_uint<128>> acc0_out(NUM_SLR * L * D / 8);
    // aligned_vector<ap_uint<512>> acc0_out(NUM_SLR, aligned_vector<ap_uint<512>>(L * L / 16));
    aligned_vector<ap_uint<64>> acc1_out(NUM_SLR * L * D / 8);
    aligned_vector<int> cycle_count(1);


    vector<int> X_copy(L * D);
    vector<vector<int>> W_acc0_split(NUM_DUM_SLR, vector<int>(D * D_head * 8));
    vector<vector<int>> W_acc1_split(NUM_DUM_SLR, vector<int>(D * D_head * 8));
    vector<vector<int>> W_k_split(NUM_DUM_SLR, vector<int>(D * D_head * 8));
    vector<aligned_vector<int>> q_golden(NUM_DUM_SLR, aligned_vector<int>(L * D_head));
    vector<aligned_vector<int>> k_golden(NUM_DUM_SLR, aligned_vector<int>(L * D_head));
    vector<aligned_vector<int>> attn_golden(NUM_DUM_SLR, aligned_vector<int>(L * L));
    vector<aligned_vector<int>> acc1_out_golden(NUM_DUM_SLR, aligned_vector<int>(L * D_head));

    // for(int i = 0; i < L * D; i++){
    //     int val = (rand() % 8) + 1;
    //     ap_int<32> full = tapa::bit_cast<ap_int<32>>(val);
    //     X_copy[i] = val;
    //     X_acc0[i] = ap_int<8>(full(7, 0));
    //     X_acc1[i] = ap_int<8>(full(7, 0));
    // }

    // for(int i = 0; i < D * D_head * NUM_DUM_SLR * 4; i++){
    //     int val = (rand() % 6) - 1;
    //     ap_int<32> full = tapa::bit_cast<ap_int<32>>(val);
    //     W_acc0[i/2]((i%2+1)*4-1, (i%2)*4) = ap_int<4>(full(3, 0));
    //     W_acc0_split[(i / 32) % 4][(i / 128) * 32 + (i % 32)] = val;
    // }

    // for(int i = 0; i < D * D_head * NUM_DUM_SLR * 4; i++){
    //     int val = (rand() % 6) - 1;
    //     ap_int<32> full = tapa::bit_cast<ap_int<32>>(val);
    //     W_acc1[i/2]((i%2+1)*4-1, (i%2)*4) = ap_int<4>(full(3, 0));
    //     W_acc1_split[(i / 32) % 4][(i / 128) * 32 + (i % 32)] = val;
    // }

    // for(int i = D * D_head * NUM_DUM_SLR * 4; i < D * D_head * NUM_DUM_SLR * 12; i++){
    //     int val = (rand() % 6) - 1;
    //     int ind = i - D * D_head * NUM_DUM_SLR * 4;
    //     ap_int<32> full = tapa::bit_cast<ap_int<32>>(val);
    //     W_acc0[i/2]((i%2+1)*4-1, (i%2)*4) = ap_int<4>(full(3, 0));
    //     W_acc1[i/2]((i%2+1)*4-1, (i%2)*4) = ap_int<4>(full(3, 0));
    //     W_k_split[(ind / 32) % 4][(ind / 128) * 32 + (ind % 32)] = val;
    // }

    // // cpu 
    // for(int i = 0; i < NUM_SLR; i++){
    //     // WqX
    //     for(int j = 0; j < L; j++){
    //         for(int k = 0; k < D_head; k++){
    //             int acc = 0;
    //             for(int l = 0; l < D; l++){
    //                 acc += X_copy[j*D+l] * W_acc0_split[i][l*D_head + k];
    //             }
    //             q_golden[i][j * D_head + k] = std::min(std::max((acc >> 8), -128), 127);
    //         }
    //     }

    //     //WvX
    //     for(int j = 0; j < L; j++){
    //         for(int k = 0; k < D_head; k++){
    //             int acc = 0;
    //             for(int l = 0; l < D; l++){
    //                 acc += X_copy[j*D+l] * W_acc1_split[i][l*D_head + k];
    //             }
    //             acc1_out_golden[i][j * D_head + k] = std::min(std::max((acc >> 8), -128), 127);
    //         }
    //     }

    //     //WkX
    //     for(int j = 0; j < L; j++){
    //         for(int k = 0; k < D_head; k++){
    //             int acc = 0;
    //             for(int l = 0; l < D; l++){
    //                 acc += X_copy[j*D+l] * W_k_split[i][l*D_head + k];
    //             }
    //             k_golden[i][j * D_head + k] = std::min(std::max((acc >> 8), -128), 127);
    //         }
    //     }

    //     // QK^T
    //     for(int j = 0; j < L; j++){
    //         for(int k = 0; k < L; k++){
    //             int acc = 0;
    //             for(int l = 0; l < D_head; l++){
    //                 acc += q_golden[i][k*D_head+l] * k_golden[i][j*D_head+l];
    //             }
    //             attn_golden[i][j*D_head+k] = acc;
    //         }
    //     }
    // }


    // invoke the kernel
    int64_t kernel_time_ns = 0;
    for(int i = 0; i < 24; i++){
        kernel_time_ns += tapa::invoke(opt_kernel, FLAGS_bitstream,
            L * D, L * D / 16, L,
            // tapa::read_only_mmap<int>(inst), 
            tapa::read_only_mmap<ap_int<8>>(X_acc0).reinterpret<ap_uint<512>>(), 
            tapa::read_only_mmap<ap_int<8>>(X_acc1).reinterpret<ap_uint<512>>(), 
            tapa::read_only_mmap<ap_int<8>>(W_acc0).reinterpret<ap_uint<512>>(), 
            tapa::read_only_mmap<ap_int<8>>(W_acc1).reinterpret<ap_uint<512>>(), 
            tapa::write_only_mmap<ap_uint<128>>(acc0_out), 
            // tapa::write_only_mmap<ap_uint<64>>(acc1_out), 
            tapa::write_only_mmap<int>(cycle_count));
    }
    
    std::clog << "cycle time: " << cycle_count[0] << std::endl;
    std::clog << "kernel time: " << kernel_time_ns * 1e-9 << " s" << std::endl;
        
}

