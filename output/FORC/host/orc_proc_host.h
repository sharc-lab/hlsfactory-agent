#ifndef orc_proc_host_h
#define orc_proc_host_h

#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <stdexcept>
#include <algorithm>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdio.h>
#include <aio.h>
#include <fcntl.h>
#include <chrono>
#include <thread>

#include <orc/orc-config.hh>
#include <orc/Reader.hh>
#include <orc/Exceptions.hh>
#include <orc/OrcFile.hh>

#include <tapa.h>
//leave this space between tapa.h and ap_int.h
#include <ap_int.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

// #define __CL_ENABLE_EXCEPTIONS
// #include <CL/cl.h>

#include <tinyxml.h>
#include <xclbin.h>
#define CL_HPP_ENABLE_EXCEPTIONS
// #define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl2.hpp>

#include <CL/opencl.h>
// #include <CL/cl_ext_xilinx.h>

#define CL_DEVICE_PCIE_BDF              0x1120  // BUS/DEVICE/FUNCTION
#include "opencl_util.h"
//g++ -O2 -o decoder data_decoder.cpp data_decoder_host.cpp -I/local-scratch/Xilinx/Vitis_HLS/2021.2/include/ -L/opt/xilinx/xrt/lib/ -lstdc++ -lpthread -lrt -lgmp -lmpfr -ltapa -lfrt -lglog -lgflags -lOpenCL

extern "C" {
    int aio_write(struct aiocb*);
    int aio_read(struct aiocb*);
    int aio_error(const struct aiocb *aiocbp);
    ssize_t aio_return(struct aiocb *aiocbp);
    int aio_suspend(const struct aiocb * const cblist[], int n, const struct timespec *timeout);
}


int nvmeFd = -1;
const uint32_t NDelta_BitMap[32] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
                                23, 24, 26, 28, 30, 32, 40, 48, 56, 64};
const uint32_t AXI_WIDTH = 512;
const uint32_t AXI_BYTES = 512/8;
const uint16_t AXI_WIDTH_H = 256;
const uint16_t AXI_WIDTH_HH = 128;
typedef ap_uint<AXI_WIDTH_H> _256b;
typedef ap_uint<AXI_WIDTH> _512b;
typedef ap_int<AXI_WIDTH> _512bi;
typedef ap_uint<AXI_WIDTH_HH> _128b;
typedef ap_uint<32> _32b;

// #define PRINT_DEBUG
const bool dataflow = 1;        //**********DATAFLOW FLAG***************//
const uint32_t DATA_MUL = 1;

const uint32_t BUFFERS_IN = 2;
const uint32_t BUFFERS_OUT = 12;
const uint32_t ALIGNED_BYTES = 4096;

uint32_t nrows = 0;
const uint32_t Myrows = 855000; 
const uint64_t FILE_CHUNKS = 1;
const uint64_t OFFSET_MUL = 0;  //TURN OFF OFFSET IF want to read same data always

const uint8_t SR = 0;
const uint8_t DIRECT = 1;
const uint8_t PATCHED = 2;
const uint8_t DELTA = 3;



#endif // orc_proc_host_h
