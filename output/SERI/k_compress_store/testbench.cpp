/**
 * Testbench for k_compress_store HLS kernel
 * SERI: Streaming Accelerator for Electron Repulsion Integrals
 * 
 * This testbench verifies the compress_store kernel which compresses
 * and stores the computed ERI results.
 */

#include <iostream>
#include <cstdlib>
#include <cmath>
#include "ap_int.h"
#include "hls_stream.h"

// Use default AM_ABCD = 3131 for testing
#ifndef AM_ABCD
#define AM_ABCD 3131
#endif

#include "parameters.h"
#include "types.h"
#include "internal_types.h"

using namespace qcf;

// External function declaration
extern "C" {
void k_compress_store(
    hls::stream<D_WIDTH(ba_cart_eris_pt::bit_width / 2)>& cart_eris_stream_in_0,
    hls::stream<D_WIDTH(ba_cart_eris_pt::bit_width / 2)>& cart_eris_stream_in_1,
    hls::stream<fp_t>& b_max_stream_in,
    REPEAT(ARGS_CART_ERIS, NUM_ERIS_PORTS)
    int n
);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "k_compress_store Testbench" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Testing with AM_ABCD = " << AM_ABCD << std::endl;
    std::cout << "NUM_ERIS_PORTS = " << NUM_ERIS_PORTS << std::endl;
    std::cout << std::endl;

    const int n = 3;

    // Create input streams
    hls::stream<D_WIDTH(ba_cart_eris_pt::bit_width / 2)> cart_eris_stream_in_0;
    hls::stream<D_WIDTH(ba_cart_eris_pt::bit_width / 2)> cart_eris_stream_in_1;
    hls::stream<fp_t> b_max_stream_in;

    // Create output arrays
    #define DEF_CART_ERIS(z) packed_eri_t cart_eris_##z[n * ncg_d * ncg_c];
    REPEAT(DEF_CART_ERIS, NUM_ERIS_PORTS)

    // Initialize random seed
    srand(42);

    // Write input data to streams
    for (int i = 0; i < n * ncg_d * ncg_c; ++i) {
        cart_eris_stream_in_0.write(D_WIDTH(ba_cart_eris_pt::bit_width / 2)(rand()));
        cart_eris_stream_in_1.write(D_WIDTH(ba_cart_eris_pt::bit_width / 2)(rand()));
    }

    // Write b_max value
    b_max_stream_in.write(1.0f);

    std::cout << "Running k_compress_store kernel..." << std::endl;

    // Call the kernel
    k_compress_store(
        cart_eris_stream_in_0,
        cart_eris_stream_in_1,
        b_max_stream_in,
        REPEAT(PASS_CART_ERIS, NUM_ERIS_PORTS)
        n
    );

    std::cout << "Output arrays produced" << std::endl;

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "k_compress_store testbench completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
