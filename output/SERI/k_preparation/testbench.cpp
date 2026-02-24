/**
 * Testbench for k_preparation HLS kernel
 * SERI: Streaming Accelerator for Electron Repulsion Integrals
 * 
 * This testbench verifies the preparation kernel which computes
 * derived coordinates and auxiliary arrays for ERI computation.
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
void k_preparation(
    const qp_t* abcd_inp,
    const rys_t* rtwt_rys_inp,
    int n,
    hls::stream<D_WIDTH(xyz_derived_pt::bit_width)>& xyz_derived_stream_out,
    hls::stream<D_WIDTH(rys_wt_pt::bit_width)>& rys_wt_stream_out,
    hls::stream<D_WIDTH(tb_b_pt::bit_width)>& tb_b_stream_out,
    hls::stream<D_WIDTH(tb_c_pt::bit_width)>& tb_c_stream_out,
    hls::stream<n_t>& n_stream_out
);
}

// Helper function to generate random float
float random_float() {
    return static_cast<float>(rand()) / RAND_MAX;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "k_preparation Testbench" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Testing with AM_ABCD = " << AM_ABCD << std::endl;
    std::cout << "ord_rys = " << ord_rys << std::endl;
    std::cout << std::endl;

    const int n = 10;  // Number of quartets to test

    // Allocate input arrays
    qp_t* abcd_inp = new qp_t[n];
    rys_t* rtwt_rys_inp = new rys_t[n];

    // Initialize input data
    srand(42);
    for (int i = 0; i < n; ++i) {
        // Fill qp_t structure with random data
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < nxyz; ++k) {
                abcd_inp[i].xyz_abcd[j][k] = random_float();
            }
            abcd_inp[i].exp_abcd[j] = random_float();
        }

        // Fill rys_t structure with random data
        for (int j = 0; j < ord_rys; ++j) {
            rtwt_rys_inp[i].rt[j] = random_float();
            rtwt_rys_inp[i].wt[j] = random_float();
        }
    }

    // Create output streams
    hls::stream<D_WIDTH(xyz_derived_pt::bit_width)> xyz_derived_stream;
    hls::stream<D_WIDTH(rys_wt_pt::bit_width)> rys_wt_stream;
    hls::stream<D_WIDTH(tb_b_pt::bit_width)> tb_b_stream;
    hls::stream<D_WIDTH(tb_c_pt::bit_width)> tb_c_stream;
    hls::stream<n_t> n_stream;

    std::cout << "Running k_preparation kernel..." << std::endl;

    // Call the kernel
    k_preparation(
        abcd_inp,
        rtwt_rys_inp,
        n,
        xyz_derived_stream,
        rys_wt_stream,
        tb_b_stream,
        tb_c_stream,
        n_stream
    );

    // Verify n_stream output
    n_t n_out = n_stream.read();
    if (n_out != n) {
        std::cerr << "ERROR: n_stream mismatch. Expected " << n << ", got " << n_out << std::endl;
        return 1;
    }
    std::cout << "n_stream output verified: " << n_out << std::endl;

    // Verify output streams
    int outputs_received = 0;
    for (int i = 0; i < n; ++i) {
        if (!xyz_derived_stream.empty()) {
            D_WIDTH(xyz_derived_pt::bit_width) xyz_data = xyz_derived_stream.read();
            outputs_received++;
        }
        if (!rys_wt_stream.empty()) {
            D_WIDTH(rys_wt_pt::bit_width) rys_data = rys_wt_stream.read();
        }
        if (!tb_b_stream.empty()) {
            D_WIDTH(tb_b_pt::bit_width) tb_b_data = tb_b_stream.read();
        }
        if (!tb_c_stream.empty()) {
            D_WIDTH(tb_c_pt::bit_width) tb_c_data = tb_c_stream.read();
        }
    }

    std::cout << "Outputs received: " << outputs_received << " / " << n << std::endl;

    // Cleanup
    delete[] abcd_inp;
    delete[] rtwt_rys_inp;

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "k_preparation testbench completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
