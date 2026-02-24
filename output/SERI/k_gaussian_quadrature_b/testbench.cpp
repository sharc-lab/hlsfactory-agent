/**
 * Testbench for k_gaussian_quadrature_b HLS kernel
 * SERI: Streaming Accelerator for Electron Repulsion Integrals
 * 
 * This testbench verifies the gaussian_quadrature_b kernel which computes
 * the second part of the Gaussian quadrature for ERI evaluation.
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
void k_gaussian_quadrature_b(
#if BP_BYPASS == 0
    REPEAT(ARGS_UBA_IN, BP_FIFO_COUNT)
#elif BP_BYPASS == 1
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)>& ucba_stream_in_0,
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)>& ucba_stream_in_1,
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)>& ucba_stream_in_2,
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)>& ucba_stream_in_3,
#endif
    hls::stream<ap_uint<ba_cart_eris_b_pt::bit_width / 2>>& partial_eris_stream_out_0,
    hls::stream<ap_uint<ba_cart_eris_b_pt::bit_width / 2>>& partial_eris_stream_out_1,
    hls::stream<n_t>& n_stream_in,
    hls::stream<n_t>& n_stream_out
);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "k_gaussian_quadrature_b Testbench" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Testing with AM_ABCD = " << AM_ABCD << std::endl;
    std::cout << "BP_BYPASS = " << BP_BYPASS << std::endl;
    std::cout << std::endl;

    const int n = 3;

    // Create streams based on BP_BYPASS
#if BP_BYPASS == 0
    #define DEF_UBA_IN_STREAM(z) hls::stream<D_WIDTH(uba_pt::bit_width)> uba_stream_in_##z;
    REPEAT(DEF_UBA_IN_STREAM, BP_FIFO_COUNT)
#else
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)> ucba_stream_in_0;
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)> ucba_stream_in_1;
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)> ucba_stream_in_2;
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)> ucba_stream_in_3;
#endif

    hls::stream<ap_uint<ba_cart_eris_b_pt::bit_width / 2>> partial_eris_stream_out_0;
    hls::stream<ap_uint<ba_cart_eris_b_pt::bit_width / 2>> partial_eris_stream_out_1;
    hls::stream<n_t> n_stream_in;
    hls::stream<n_t> n_stream_out;

    // Initialize random seed
    srand(42);

    // Write input data to streams
#if BP_BYPASS == 0
    for (int i = 0; i < n * bp_phi; ++i) {
        #define WRITE_UBA_IN(z) uba_stream_in_##z.write(D_WIDTH(uba_pt::bit_width)(rand()));
        REPEAT(WRITE_UBA_IN, BP_FIFO_COUNT)
    }
#else
    for (int i = 0; i < n * ord_rys * nxyz / rr_unroll_factor; ++i) {
        ucba_stream_in_0.write(D_WIDTH(ucba_pt::bit_width / 4)(rand()));
        ucba_stream_in_1.write(D_WIDTH(ucba_pt::bit_width / 4)(rand()));
        ucba_stream_in_2.write(D_WIDTH(ucba_pt::bit_width / 4)(rand()));
        ucba_stream_in_3.write(D_WIDTH(ucba_pt::bit_width / 4)(rand()));
    }
#endif

    // Write n to input stream
    n_stream_in.write(n);

    std::cout << "Running k_gaussian_quadrature_b kernel..." << std::endl;

    // Call the kernel
#if BP_BYPASS == 0
    k_gaussian_quadrature_b(
        REPEAT(PASS_UBA_IN, BP_FIFO_COUNT)
        partial_eris_stream_out_0, partial_eris_stream_out_1,
        n_stream_in, n_stream_out
    );
#else
    k_gaussian_quadrature_b(
        ucba_stream_in_0, ucba_stream_in_1, ucba_stream_in_2, ucba_stream_in_3,
        partial_eris_stream_out_0, partial_eris_stream_out_1,
        n_stream_in, n_stream_out
    );
#endif

    // Verify n_stream output
    n_t n_out = n_stream_out.read();
    if (n_out != n) {
        std::cerr << "ERROR: n_stream mismatch. Expected " << n << ", got " << n_out << std::endl;
        return 1;
    }
    std::cout << "n_stream output verified: " << n_out << std::endl;

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "k_gaussian_quadrature_b testbench completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
