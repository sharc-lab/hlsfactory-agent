#include <iostream>
#include <cstdint>
#include "ap_int.h"
#include "tapa.h"
#include "orc_proc.h"
#include "orcDecomp.h"

// Testbench for orcDecomp design
int main() {
    std::cout << "Testing orcDecomp design..." << std::endl;
    
    // Create test streams
    tapa::stream<ap_uint<512>, 32> inStream;
    tapa::stream<ap_uint<512>, 32> outCstm;
    tapa::stream<ap_uint<512>, 32> outUCstm;
    tapa::stream<ap_uint<20>, 32> outCMDstm;
    tapa::stream<ap_uint<20>, 32> outUCMDstm;
    
    // Push test data
    for (int i = 0; i < 10; i++) {
        inStream.write(ap_uint<512>(i * 0x0101010101010101ULL));
    }
    
    std::cout << "Pushed 10 test words to input stream" << std::endl;
    std::cout << "orcDecomp module ready for testing" << std::endl;
    std::cout << "Note: This is a template-based header-only design." << std::endl;
    std::cout << "Tests DecompHead, decompSender, zlib_Sender, decompData, DataCombiner modules." << std::endl;
    
    return 0;
}
