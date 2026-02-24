#include <iostream>
#include <cstdint>
#include "ap_int.h"
#include "tapa.h"
#include "orc_proc.h"
#include "orc_filter.h"

// Testbench for orc_filter design
int main() {
    std::cout << "Testing orc_filter design..." << std::endl;
    
    // Create test streams
    tapa::stream<ap_uint<512>, 32> inConfStrm;
    tapa::stream<ap_uint<512>, 32> inAllStrm;
    tapa::stream<uint16_t, 32> inFilIdx;
    tapa::stream<bool, 32> eFd_strm;
    tapa::stream<ap_uint<512>, 32> outFilterStrm;
    tapa::stream<uint16_t, 32> outIdxStrm;
    tapa::stream<uint8_t, 32> dataCnt;
    tapa::stream<bool, 32> eFDout_strm;
    
    // Setup filter configuration
    ap_uint<512> filter_conf = 0;
    filter_conf.range(7, 0) = 0;      // idx_flag = 0 (range mode)
    filter_conf.range(15, 8) = 1;     // range_flag = 1
    filter_conf.range(23, 16) = 5;    // RROP = FOP_GTE
    filter_conf.range(31, 24) = 5;    // LROP = FOP_GTE
    filter_conf.range(63, 32) = 100;  // RR = 100
    filter_conf.range(95, 64) = 0;    // LR = 0
    
    inConfStrm.write(filter_conf);
    
    // Push test data
    for (int i = 0; i < 10; i++) {
        ap_uint<512> data = 0;
        for (int j = 0; j < 16; j++) {
            data.range(j * 32 + 31, j * 32) = i * 16 + j;
        }
        inAllStrm.write(data);
        eFd_strm.write(true);
    }
    eFd_strm.write(false);  // End of stream
    
    std::cout << "Pushed filter configuration and test data" << std::endl;
    std::cout << "Filter configured for range: [0, 100]" << std::endl;
    std::cout << "orc_filter modules ready for testing:" << std::endl;
    std::cout << "  - FilterData: Main filter processing" << std::endl;
    std::cout << "  - br_s0e: Bubble remover stage 0" << std::endl;
    std::cout << "  - br_s1: Bubble remover stage 1" << std::endl;
    std::cout << "  - br_WrTracker: Write tracker and final stage" << std::endl;
    
    return 0;
}
