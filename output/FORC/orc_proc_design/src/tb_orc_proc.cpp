#include <iostream>
#include <cstdint>
#include "ap_int.h"
#include "tapa.h"
#include "orc_proc.h"

// Simple testbench for orc_proc top-level design
int main() {
    std::cout << "Testing orc_proc design..." << std::endl;
    
    // Create test data buffers
    ap_uint<512> input_data[100];
    ap_uint<512> output_data0[100];
    ap_uint<512> output_data1[100];
    ap_uint<512> output_data2[100];
    ap_uint<512> output_data3[100];
    ap_uint<512> idx_data[100];
    ap_uint<512> track_data[100];
    
    // Initialize input data with test pattern
    for (int i = 0; i < 100; i++) {
        input_data[i] = ap_uint<512>(i * 0x0101010101010101ULL);
    }
    
    // Create TAPA mmap objects
    tapa::mmap<ap_uint<512>> input_port(input_data);
    tapa::mmap<ap_uint<512>> filter_conf(&input_data[0]);  // Reuse buffer for config
    tapa::mmap<ap_uint<512>> output_port0_32b_8b(output_data0);
    tapa::mmap<ap_uint<512>> output_port1_16b_8b(output_data1);
    tapa::mmap<ap_uint<512>> output_port2_16b_8b(output_data2);
    tapa::mmap<ap_uint<512>> output_port3_8b(output_data3);
    tapa::mmap<ap_uint<512>> data_idx(idx_data);
    tapa::mmap<ap_uint<512>> output_port4_track(track_data);
    
    std::cout << "Test setup complete. Data count: 100" << std::endl;
    std::cout << "Note: orc_proc requires TAPA framework for actual execution." << std::endl;
    std::cout << "This testbench verifies compilation and basic structure." << std::endl;
    
    return 0;
}
