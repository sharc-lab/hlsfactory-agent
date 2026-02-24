/**
 * Testbench for k_rr_b HLS kernel
 * SERI: Streaming Accelerator for Electron Repulsion Integrals
 * 
 * This testbench verifies the k_rr_b kernel which is the second part
 * of the split recurrence relations computation.
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
void k_rr_b(
    hls::stream<D_WIDTH(xyz_derived_pt::bit_width)>& xyz_derived_stream_in,
    hls::stream<D_WIDTH(rys_wt_pt::bit_width)>& rys_wt_stream_in,
    hls::stream<D_WIDTH(tb_b_pt::bit_width)>& tb_b_stream_in,
    hls::stream<D_WIDTH(tb_c_pt::bit_width)>& tb_c_stream_in,
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 2)>& ucba_stream_out_1,
    hls::stream<n_t>& n_stream_in,
    hls::stream<n_t>& n_stream_out
);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "k_rr_b Testbench" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Testing with AM_ABCD = " << AM_ABCD << std::endl;
    std::cout << "ord_rys = " << ord_rys << std::endl;
    std::cout << std::endl;

    const int n = 3;

    // Create input streams
    hls::stream<D_WIDTH(xyz_derived_pt::bit_width)> xyz_derived_stream_in;
    hls::stream<D_WIDTH(rys_wt_pt::bit_width)> rys_wt_stream_in;
    hls::stream<D_WIDTH(tb_b_pt::bit_width)> tb_b_stream_in;
    hls::stream<D_WIDTH(tb_c_pt::bit_width)> tb_c_stream_in;
    hls::stream<n_t> n_stream_in;

    // Create output streams
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 2)> ucba_stream_out_1;
    hls::stream<n_t> n_stream_out;

    // Initialize random seed
    srand(42);

    // Write input data to streams
    for (int i = 0; i < n; ++i) {
        xyz_derived_stream_in.write(D_WIDTH(xyz_derived_pt::bit_width)(rand()));
        rys_wt_stream_in.write(D_WIDTH(rys_wt_pt::bit_width)(rand()));
        tb_b_stream_in.write(D_WIDTH(tb_b_pt::bit_width)(rand()));
        tb_c_stream_in.write(D_WIDTH(tb_c_pt::bit_width)(rand()));
    }

    // Write n to input stream
    n_stream_in.write(n);

    std::cout << "Running k_rr_b kernel..." << std::endl;

    // Call the kernel
    k_rr_b(
        xyz_derived_stream_in, rys_wt_stream_in, tb_b_stream_in, tb_c_stream_in,
        ucba_stream_out_1, n_stream_in, n_stream_out
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
    std::cout << "k_rr_b testbench completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
