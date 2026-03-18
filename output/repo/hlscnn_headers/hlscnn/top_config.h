// =============================================================================
// MIT License
//
// Copyright (c) 2019 Princeton University
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// =============================================================================

// File: top_config.h

#ifndef TOP_CONFIG_H__
#define TOP_CONFIG_H__

#include <hlscnn/config_reg.h>

namespace ilang {
namespace hlscnn {

  #define TOP_SLAVE_IF_WR "top_slave_if_wr"
  #define TOP_SLAVE_IF_WR_BITWIDTH 1

  #define TOP_SLAVE_IF_RD "top_slave_if_rd"
  #define TOP_SLAVE_IF_RD_BITWIDTH 1
  // AXI protocol
  // axi_cfg.h
  
  // there are two AXI interface involved
  // a slave interface for configuration writes and a master interface for fetching 
  // weights and activations

  /////////////////////////////////////////
  //  AXI slave interface
  ////////////////////////////////////////

  // the slave in has both address and data

  #define TOP_SLAVE_ADDR_IN "top_slave_addr_in"
  #define TOP_SLAVE_ADDR_IN_BITWIDTH 32

  #define TOP_SLAVE_DATA_IN_0 "top_slave_data_in_0"
  #define TOP_SLAVE_DATA_IN_1 "top_slave_data_in_1"
  #define TOP_SLAVE_DATA_IN_2 "top_slave_data_in_2"
  #define TOP_SLAVE_DATA_IN_3 "top_slave_data_in_3"
  #define TOP_SLAVE_DATA_IN_4 "top_slave_data_in_4"
  #define TOP_SLAVE_DATA_IN_5 "top_slave_data_in_5"
  #define TOP_SLAVE_DATA_IN_6 "top_slave_data_in_6"
  #define TOP_SLAVE_DATA_IN_7 "top_slave_data_in_7"
  #define TOP_SLAVE_DATA_IN_8 "top_slave_data_in_8"
  #define TOP_SLAVE_DATA_IN_9 "top_slave_data_in_9"
  #define TOP_SLAVE_DATA_IN_10 "top_slave_data_in_10"
  #define TOP_SLAVE_DATA_IN_11 "top_slave_data_in_11"
  #define TOP_SLAVE_DATA_IN_12 "top_slave_data_in_12"
  #define TOP_SLAVE_DATA_IN_13 "top_slave_data_in_13"
  #define TOP_SLAVE_DATA_IN_14 "top_slave_data_in_14"
  #define TOP_SLAVE_DATA_IN_15 "top_slave_data_in_15"

  #define TOP_SLAVE_DATA_IN_BITWIDTH 8

  //////////////////////////////////////////////
  // AXI master interface
  //////////////////////////////////////////////
  
  // AXI master interface has both write and read 
  // write as "out" and read as "in"

  // don't model the write req for now.
  // it seems that the HLSCNN only use master for reading data

  // master read only has data
  #define TOP_MASTER_IF_RD "top_master_if_rd"
  #define TOP_MASTER_IF_RD_BITWIDTH 1

  #define TOP_MASTER_RD_ADDR_OUT "top_master_rd_addr_out"
  #define TOP_MASTER_RD_ADDR_OUT_BITWIDTH 32

  // a valid signal for data in
  // use this and top master_if_rd to immitate the behavior of push and pop
  #define TOP_MASTER_RD_RESP_VALID_FLAG "top_master_rd_resp_valid_flag"
  #define TOP_MASTER_RD_RESP_VALID_FLAG_BITWIDTH 1

  #define TOP_MASTER_DATA_IN "top_master_data_in"
  #define TOP_MASTER_DATA_IN_0 "top_master_data_in_0"
  #define TOP_MASTER_DATA_IN_1 "top_master_data_in_1"
  #define TOP_MASTER_DATA_IN_2 "top_master_data_in_2"
  #define TOP_MASTER_DATA_IN_3 "top_master_data_in_3"
  #define TOP_MASTER_DATA_IN_4 "top_master_data_in_4"
  #define TOP_MASTER_DATA_IN_5 "top_master_data_in_5"
  #define TOP_MASTER_DATA_IN_6 "top_master_data_in_6"
  #define TOP_MASTER_DATA_IN_7 "top_master_data_in_7"
  #define TOP_MASTER_DATA_IN_8 "top_master_data_in_8"
  #define TOP_MASTER_DATA_IN_9 "top_master_data_in_9"
  #define TOP_MASTER_DATA_IN_10 "top_master_data_in_10"
  #define TOP_MASTER_DATA_IN_11 "top_master_data_in_11"
  #define TOP_MASTER_DATA_IN_12 "top_master_data_in_12"
  #define TOP_MASTER_DATA_IN_13 "top_master_data_in_13"
  #define TOP_MASTER_DATA_IN_14 "top_master_data_in_14"
  #define TOP_MASTER_DATA_IN_15 "top_master_data_in_15"

  #define TOP_MASTER_DATA_IN_BITWIDTH 8




  // memory address 
  // memory_map.h
  #define SCRATCH_PAD_0 "scratch_pad_0"
  #define SCRATCH_PAD_1 "scratch_pad_1"
  
  #define SCRATCH_PAD_DATA_BITWIDTH 8

  // scratch pad parameters
  // each memory bank should be 16-byte wide
  #define SPAD_NUM_BANK 4
  #define SPAD_DATA_BYTE_WIDTH 16
  #define SPAD_CAPACITY (SPAD_NUM_BANK * 2048)
  #define SPAD_BYTE_ENTRY_NUM (SPAD_CAPACITY * SPAD_DATA_BYTE_WIDTH)

  // base addr for configurations
  #define CONFIG_BASE_ADDR 0X0
  // base address for scratchpad 0
  #define SPAD0_BASE_ADDR 0x4000
  // base address for scratchpad 1
  #define SPAD1_BASE_ADDR (SPAD0_BASE_ADDR + SPAD_DATA_BYTE_WIDTH * SPAD_CAPACITY)
  // upper bound for valid address
  #define MEM_ADDR_MAX (SPAD1_BASE_ADDR + SPAD_DATA_BYTE_WIDTH * SPAD_CAPACITY)

  #define SLAVE_BASE_FROM_HOST 0x32000000
  #define ADDR_IN_MAX (SLAVE_BASE_FROM_HOST + NumCfgRegisters * CFG_REG_BYTEWIDTH)

  //----------------------------------------------------------------
  // This part is for abstracting the AXI master interface 
  // using an internal memory state to represent the SOC memory
  // ---------------------------------------------------------------
  #define VIRTUAL_SOC_MEMORY "virtual_soc_memory"
  #define VIRTUAL_SOC_MEMORY_DATA_BITWIDTH 8
  #define VIRTUAL_SOC_MEMORY_BYTE_ENTRY_NUM 0x30000
  #define VIRTUAL_SOC_MEMORY_ADDR_MIN 0x50000
  #define VIRTUAL_SOC_MEMORY_ADDR_MAX                                                  \
    (VIRTUAL_SOC_MEMORY_ADDR_MIN + VIRTUAL_SOC_MEMORY_BYTE_ENTRY_NUM)

  // define a virtual memory for reading the outputs
  #define VIRTUAL_OUTPUT_MEMORY "virtual_output_memory"
  // the output are activations, which are 16 bits in the HLSCNN
  #define VIRTUAL_OUTPUT_MEMORY_DATA_BITWIDTH 16
  #define VIRTUAL_OUTPUT_MEMORY_ENTRY_NUM 0X10000
   
} // namespace hlscnn
} // namespace ilang



#endif // TOP_CONFIG_H__