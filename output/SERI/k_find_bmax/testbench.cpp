/**
 * Testbench for k_find_bmax HLS kernel
 * SERI: Streaming Accelerator for Electron Repulsion Integrals
 * 
 * This testbench verifies the find_bmax kernel which finds the maximum
 * value in the ERI computation results.
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
void k_find_bmax(
#if GQ_KERNEL_SPLIT == 2
    hls::stream<D_WIDTH(ba_cart_eris_a_pt::bit_width / 2)>& partial_eris_a_stream_in_0,
    hls::stream<D_WIDTH(ba_cart_eris_a_pt::bit_width / 2)>& partial_eris_a_stream_in_1,
#endif
    hls::stream<D_WIDTH(ba_cart_eris_b_pt::bit_width / 2)>& partial_eris_b_stream_in_0,
    hls::stream<D_WIDTH(ba_cart_eris_b_pt::bit_width / 2)>& partial_eris_b_stream_in_1,
    hls::stream<D_WIDTH(ba_cart_eris_pt::bit_width / 2)>& cart_eris_stream_out_0,
    hls::stream<D_WIDTH(ba_cart_eris_pt::bit_width / 2)>& cart_eris_stream_out_1,
    hls::stream<fp_t>& b_max_stream_out,
    hls::stream<n_t>& n_stream_in
);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "k_find_bmax Testbench" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Testing with AM_ABCD = " << AM_ABCD << std::endl;
    std::cout << "GQ_KERNEL_SPLIT = " << GQ_KERNEL_SPLIT << std::endl;
    std::cout << std::endl;

    const int n = 3;

    // Create streams
#if GQ_KERNEL_SPLIT == 2
    hls::stream<D_WIDTH(ba_cart_eris_a_pt::bit_width / 2)> partial_eris_a_stream_in_0;
    hls::stream<D_WIDTH(ba_cart_eris_a_pt::bit_width / 2)> partial_eris_a_stream_in_1;
#endif
    hls::stream<D_WIDTH(ba_cart_eris_b_pt::bit_width / 2)> partial_eris_b_stream_in_0;
    hls::stream<D_WIDTH(ba_cart_eris_b_pt::bit_width / 2)> partial_eris_b_stream_in_1;
    hls::stream<D_WIDTH(ba_cart_eris_pt::bit_width / 2)> cart_eris_stream_out_0;
    hls::stream<D_WIDTH(ba_cart_eris_pt::bit_width / 2)> cart_eris_stream_out_1;
    hls::stream<fp_t> b_max_stream_out;
    hls::stream<n_t> n_stream_in;

    // Initialize random seed
    srand(42);

    // Write input data to streams
    for (int i = 0; i < n * ncg_d * ncg_c; ++i) {
#if GQ_KERNEL_SPLIT == 2
        partial_eris_a_stream_in_0.write(D_WIDTH(ba_cart_eris_a_pt::bit_width / 2)(rand()));
        partial_eris_a_stream_in_1.write(D_WIDTH(ba_cart_eris_a_pt::bit_width / 2)(rand()));
#endif
        partial_eris_b_stream_in_0.write(D_WIDTH(ba_cart_eris_b_pt::bit_width / 2)(rand()));
        partial_eris_b_stream_in_1.write(D_WIDTH(ba_cart_eris_b_pt::bit_width / 2)(rand()));
    }

    // Write n to input stream
    n_stream_in.write(n);

    std::cout << "Running k_find_bmax kernel..." << std::endl;

    // Call the kernel
#if GQ_KERNEL_SPLIT == 2
    k_find_bmax(
        partial_eris_a_stream_in_0, partial_eris_a_stream_in_1,
        partial_eris_b_stream_in_0, partial_eris_b_stream_in_1,
        cart_eris_stream_out_0, cart_eris_stream_out_1,
        b_max_stream_out, n_stream_in
    );
#else
    k_find_bmax(
        partial_eris_b_stream_in_0, partial_eris_b_stream_in_1,
        cart_eris_stream_out_0, cart_eris_stream_out_1,
        b_max_stream_out, n_stream_in
    );
#endif

    std::cout << "Output streams produced" << std::endl;

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "k_find_bmax testbench completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
