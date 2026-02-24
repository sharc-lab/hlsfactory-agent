#include <iostream>
#include <cstdint>
#include "ap_int.h"
#include "tapa.h"
#include "orc_proc.h"
#include "zlibTapa.h"

// Testbench for zlibTapa design
int main() {
    std::cout << "Testing zlibTapa design..." << std::endl;
    
    // Create test streams
    tapa::stream<ap_uint<17>, 32> inStream;
    tapa::stream<ap_uint<17>, 32> outStream;
    tapa::stream<bool, 32> inDEos;
    tapa::stream<bool, 32> outLMDEos;
    tapa::stream<bool, 32> outLUDEos;
    tapa::stream<ap_uint<9>, 32> litLenStream;
    tapa::stream<ap_uint<9>, 32> matchLenStream;
    tapa::stream<ap_uint<16>, 32> offsetStream;
    tapa::stream<ap_uint<10>, 32> lzOutStream;
    tapa::stream<ap_uint<64>, 32> litStream;
    tapa::stream<ap_uint<72>, 32> decompressOut;
    
    // Initialize end-of-stream signal
    inDEos.write(true);
    
    // Push test data
    for (int i = 0; i < 10; i++) {
        inStream.write(ap_uint<17>(i + 1));
    }
    inStream.write(ap_uint<17>(1));  // EOS marker
    
    std::cout << "Pushed test data to streams" << std::endl;
    std::cout << "zlibTapa modules ready for testing:" << std::endl;
    std::cout << "  - lzProcessingUnit: LZ77 processing" << std::endl;
    std::cout << "  - lzLiteralUpsizer: Literal upsizing" << std::endl;
    std::cout << "  - lzMultiByteDecompress: Decompression" << std::endl;
    std::cout << "  - huffmanDecoder: Huffman decoding" << std::endl;
    
    return 0;
}
