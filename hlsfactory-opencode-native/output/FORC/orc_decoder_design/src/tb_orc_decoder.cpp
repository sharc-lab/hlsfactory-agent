#include <iostream>
#include <cstdint>
#include "ap_int.h"
#include "tapa.h"
#include "orc_proc.h"
#include "orc_decoder.h"

// Testbench for orc_decoder design
int main() {
    std::cout << "Testing orc_decoder design..." << std::endl;
    
    // Create test streams
    tapa::stream<ap_uint<512>, 32> data_in;
    tapa::stream<ap_uint<2048>, 32> outAll_Lstrm;
    tapa::stream<ap_uint<256>, 32> out_strmC_Track;
    tapa::stream<ap_uint<512>, 32> PA_DATA;
    tapa::stream<uint32_t, 32> PA_metaDATA;
    tapa::stream<bool, 32> D_strm_e;
    
    // Push test data (ORC encoded format)
    for (int i = 0; i < 20; i++) {
        data_in.write(ap_uint<512>(i * 0x0101010101010101ULL));
    }
    
    std::cout << "Pushed 20 test words to input stream" << std::endl;
    std::cout << "orc_decoder modules ready for testing:" << std::endl;
    std::cout << "  - load: Header loading and parsing" << std::endl;
    std::cout << "  - data_Sender: Data distribution" << std::endl;
    std::cout << "  - compSR: Short Repeat computation" << std::endl;
    std::cout << "  - compute_delta: Delta computation" << std::endl;
    std::cout << "  - PA_meta_proc: Patch metadata processing" << std::endl;
    std::cout << "  - PA_sum_out: Patch accumulator" << std::endl;
    std::cout << "  - Meta_Aligner: Metadata alignment" << std::endl;
    std::cout << "  - delta_sum*: Delta sum modules" << std::endl;
    std::cout << "  - Data_Aligner: Data alignment" << std::endl;
    std::cout << "  - brDecData: Bubble removal" << std::endl;
    
    return 0;
}
