#ifndef orc_proc_host_1S1C_h
#define orc_proc_host_1S1C_h

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



#define BUFFERS 5

int nvmeFd = -1;
const uint32_t BUFFERS_IN = 2;
const uint32_t BUFFERS_OUT = 10;

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

const uint32_t wait_count = 30;  //current IL=68 make it 80 for hardware and try, 2147483647
const uint32_t ALIGNED_BYTES = 4096;
const uint32_t DATA_MUL = 1;
uint32_t nrows = 0;
const uint32_t RSIZE_DIV = 64;   //
bool proc_ORC = 0;
const uint32_t Myrows = 855000; //854528*70 855000 2565056 2563584 9*512 (small) 512(ss)
// /localhdd/awa159/orc_dataset/orc_decData
std::string orc_file = "/localhdd/awa159/orc_dataset/orc_decData/direct_delta_data_all_orc.orc";  //SR_8bit_12M_orc.orc
std::string read_file = "/localhdd/awa159/orc_dataset/orc_decData/8_bit_data_orc.bin"; //delta_8bitdata_70x_orc.orc,  delta_data_orc_aligned_70x.bin | delta_data_orc.bin | delta_data_inc_dec_orc.orc | direct_delta_data_all.bin
std::string check_file = "/localhdd/awa159/orc_dataset/orc_decData/8_bit_data_Fil.bin"; //8_bit_data_Fil.bin 8_bit_data_70x_Fil.bin SR_8bit_12M_Fil.bin delta_8bitdata_70x.bin,  delta_data_aligned_70x.bin | delta_data.bin | delta_data_inc_dec.bin
std::string filFLAG = "/localhdd/awa159/orc_dataset/orc_decData/8_bit_data_FF.bin";
std::string c2FileData = "/localhdd/awa159/orc_dataset/orc_decData/8_bit_data_C2_orc.orc";
std::string c2FileDataFilt = "/localhdd/awa159/orc_dataset/orc_decData/8_bit_data_C2_Fil_Desc.bin";
//customer_col0_orc.orc, orders_col0_orc.orc, partsupp_50G_col0_orc.orc, customer_col3_orc.orc, part_col0_orc.orc, part_col5_orc.orc, supplier_col0_orc.orc, supplier_col3_orc.orc
//customer_col0.bin, customer_col3.bin, part_col0.bin,              part_col5.bin, supplier_col0.bin, supplier_col3.bin
//
//all data
//all_data_pa_sr_de_di_orc.bin,     all_data_pa_sr_de_di_orc_5x.bin
//all_data_pa_sr_de_di.bin(859619),     all_data_pa_sr_de_di_5x.bin(4295735)
//Short Repeat
//SR_8bit_orc.orc,  SR_8bit_orc_12000x.bin,         SR_8bit_orc_9500x.bin , SR_8bit_12M_orc.orc, SR_32bit_12M_10R_orc.orc
//SR_8bit.bin(1253), SR_8bit_12000x.bin(15036000),  SR_8bit_9500x.bin (1253*9500), SR_8bit_12M.bin,  SR_32bit_12M_10R.bin
//PA_DI_DE
//pa_di_de_all_orc.orc
//pa_di_de_all.bin(5991384)
//Patch
//patch_data_8bit_116830x_orc.orc, patch_data_24bit_116830x_orc.orc, patch_dec_data_8b_16b_24b_orc.orc, patch_dec_data_8b_16b_24b_orc.bin | patch_dec_data_8b_16b_24b_orc_10000x.bin           | patch_dec_data_8b_16b_24b_orc_15000x.bin
//patch_data_8bit_116830x.bin, patch_data_24bit_116830x.bin, patch_dec_data_8b_16b_24b.bin, patch_dec_data_8b_16b_24b.bin         | patch_dec_data_8b_16b_24b_10000x.bin (15360000)    | patch_dec_data_8b_16b_24b_15000x.bin (23040000)
//DI+DE
// direct_delta_data_orc.orc | direct_delta_data_orc_3x.orc | delta_data_BS0_70x_orc.orc, delta_data_BS0_orc.orc, delta_data_BS0_aligned_orc_30x.bin | direct_delta_data_all_orc.orc
// direct_delta_data.bin (3421656) | direct_delta_data_3x.bin | delta_data_BS0_70x.bin, delta_data_BS0.bin (855000), delta_data_BS0_aligned_30x.bin | direct_delta_data_all.bin (5989848)
//DIRECT FILES
// 32b_16b_8b_data_orc_aligned.bin 8_bit_data_orc.bin 8_bit_data_70x_orc.orc, 32_bit_data_70x_orc.orc
// 32b_16b_8b_data_aligned.bin (2563584)  8_bit_data.bin 8_bit_data_70x.bin, 32_bit_data_70x.bin

const uint64_t FILE_CHUNKS = 1;
const uint64_t OFFSET_MUL = 0;  //TURN OFF OFFSET IF want to read same data always

const bool DataFlow = 0;
const uint64_t TOTAL_COUNT = 10;
const uint64_t NITERS = TOTAL_COUNT + 3;

const uint8_t SR = 0;
const uint8_t DIRECT = 1;
const uint8_t PATCHED = 2;
const uint8_t DELTA = 3;

#endif // orc_proc_host_h
