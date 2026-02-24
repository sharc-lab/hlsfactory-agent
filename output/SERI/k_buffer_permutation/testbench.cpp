/**
 * Testbench for k_buffer_permutation HLS kernel
 * SERI: Streaming Accelerator for Electron Repulsion Integrals
 * 
 * This testbench verifies the buffer_permutation kernel which handles
 * data permutation and buffering for the ERI computation pipeline.
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
void k_buffer_permutation(
    hls::stream<D_WIDTH(ucba_pt::bit_width / 2)>& ucba_stream_in_0,
    hls::stream<D_WIDTH(ucba_pt::bit_width / 2)>& ucba_stream_in_1,
    REPEAT(ARGS_UBA_OUT, BP_FIFO_COUNT)
    hls::stream<n_t>& n_stream_in,
    hls::stream<n_t>& n_stream_out
);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "k_buffer_permutation Testbench" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Testing with AM_ABCD = " << AM_ABCD << std::endl;
    std::cout << "BP_FIFO_COUNT = " << BP_FIFO_COUNT << std::endl;
    std::cout << std::endl;

    const int n = 5;

    // Create input streams
    hls::stream<D_WIDTH(ucba_pt::bit_width / 2)> ucba_stream_in_0;
    hls::stream<D_WIDTH(ucba_pt::bit_width / 2)> ucba_stream_in_1;
    hls::stream<n_t> n_stream_in;
    hls::stream<n_t> n_stream_out;

    // Create output streams based on BP_FIFO_COUNT
    #define DEF_UBA_OUT_STREAM(z) hls::stream<D_WIDTH(uba_pt::bit_width)> uba_stream_out_##z;
    REPEAT(DEF_UBA_OUT_STREAM, BP_FIFO_COUNT)

    // Initialize random seed
    srand(42);

    // Write input data to streams
    for (int i = 0; i < n * bp_phi * len_d; ++i) {
        ucba_stream_in_0.write(D_WIDTH(ucba_pt::bit_width / 2)(rand()));
        ucba_stream_in_1.write(D_WIDTH(ucba_pt::bit_width / 2)(rand()));
    }

    // Write n to input stream
    n_stream_in.write(n);

    std::cout << "Running k_buffer_permutation kernel..." << std::endl;

    // Call the kernel
    k_buffer_permutation(
        ucba_stream_in_0,
        ucba_stream_in_1,
        REPEAT(PASS_UBA_OUT, BP_FIFO_COUNT)
        n_stream_in,
        n_stream_out
    );

    // Verify n_stream output
    n_t n_out = n_stream_out.read();
    if (n_out != n) {
        std::cerr << "ERROR: n_stream mismatch. Expected " << n << ", got " << n_out << std::endl;
        return 1;
    }
    std::cout << "n_stream output verified: " << n_out << std::endl;

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "k_buffer_permutation testbench completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
