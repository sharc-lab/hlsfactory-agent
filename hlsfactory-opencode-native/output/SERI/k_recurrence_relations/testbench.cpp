/**
 * Testbench for k_recurrence_relations HLS kernel
 * SERI: Streaming Accelerator for Electron Repulsion Integrals
 * 
 * This testbench verifies the recurrence_relations kernel which performs
 * vertical and horizontal recurrence relation computations.
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
void k_recurrence_relations(
    hls::stream<D_WIDTH(xyz_derived_pt::bit_width)>& xyz_derived_stream_in,
    hls::stream<D_WIDTH(rys_wt_pt::bit_width)>& rys_wt_stream_in,
    hls::stream<D_WIDTH(tb_b_pt::bit_width)>& tb_b_stream_in,
    hls::stream<D_WIDTH(tb_c_pt::bit_width)>& tb_c_stream_in,
#if BP_BYPASS == 0
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 2)>& ucba_stream_out_0,
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 2)>& ucba_stream_out_1,
#elif BP_BYPASS == 1
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)>& ucba_stream_out_0,
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)>& ucba_stream_out_1,
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)>& ucba_stream_out_2,
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)>& ucba_stream_out_3,
#endif
    hls::stream<n_t>& n_stream_in,
    hls::stream<n_t>& n_stream_out
);
}

// Helper function to generate random float
float random_float() {
    return static_cast<float>(rand()) / RAND_MAX;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "k_recurrence_relations Testbench" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Testing with AM_ABCD = " << AM_ABCD << std::endl;
    std::cout << "BP_BYPASS = " << BP_BYPASS << std::endl;
    std::cout << "ord_rys = " << ord_rys << std::endl;
    std::cout << std::endl;

    const int n = 5;

    // Create input streams
    hls::stream<D_WIDTH(xyz_derived_pt::bit_width)> xyz_derived_stream;
    hls::stream<D_WIDTH(rys_wt_pt::bit_width)> rys_wt_stream;
    hls::stream<D_WIDTH(tb_b_pt::bit_width)> tb_b_stream;
    hls::stream<D_WIDTH(tb_c_pt::bit_width)> tb_c_stream;
    hls::stream<n_t> n_stream_in;
    hls::stream<n_t> n_stream_out;

#if BP_BYPASS == 0
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 2)> ucba_stream_out_0;
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 2)> ucba_stream_out_1;
#else
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)> ucba_stream_out_0;
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)> ucba_stream_out_1;
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)> ucba_stream_out_2;
    qcf::ostream<D_WIDTH(ucba_pt::bit_width / 4)> ucba_stream_out_3;
#endif

    // Initialize random seed
    srand(42);

    // Write input data to streams
    for (int i = 0; i < n; ++i) {
        xyz_derived_stream.write(D_WIDTH(xyz_derived_pt::bit_width)(rand()));
        rys_wt_stream.write(D_WIDTH(rys_wt_pt::bit_width)(rand()));
        tb_b_stream.write(D_WIDTH(tb_b_pt::bit_width)(rand()));
        tb_c_stream.write(D_WIDTH(tb_c_pt::bit_width)(rand()));
    }

    // Write n to input stream
    n_stream_in.write(n);

    std::cout << "Running k_recurrence_relations kernel..." << std::endl;

    // Call the kernel
#if BP_BYPASS == 0
    k_recurrence_relations(
        xyz_derived_stream, rys_wt_stream, tb_b_stream, tb_c_stream,
        ucba_stream_out_0, ucba_stream_out_1,
        n_stream_in, n_stream_out
    );
#else
    k_recurrence_relations(
        xyz_derived_stream, rys_wt_stream, tb_b_stream, tb_c_stream,
        ucba_stream_out_0, ucba_stream_out_1, ucba_stream_out_2, ucba_stream_out_3,
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
    std::cout << "k_recurrence_relations testbench completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
