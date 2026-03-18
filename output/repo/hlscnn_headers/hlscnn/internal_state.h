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

// File: internal_state.h

#ifndef _INTERNAL_STATE_H__
#define _INTERNAL_STATE_H__

#include <hlscnn/conv_param.h>
#include <hlscnn/common_config.h>
#include <hlscnn/config_reg.h>

namespace ilang {
namespace hlscnn {

//////////////////////////////////////////////////
//      internal states for Master AXI interface
//////////////////////////////////////////////////
#define ACCEL_SPAD_WR_ADDR "accel_spad_wr_addr"
#define ACCEL_SPAD_WR_ADDR_BITWIDTH TOP_SLAVE_ADDR_IN_BITWIDTH

#define ACCEL_MASTER_AXI_CHILD_VALID 1
#define ACCEL_MASTER_AXI_CHILD_INVALID 0

#define ACCEL_MASTER_AXI_CHILD_VALID_FLAG "accel_master_axi_child_valid_flag"
#define ACCEL_MASTER_AXI_CHILD_VALID_FLAG_BITWIDTH 1

#define ACCEL_MASTER_AXI_CHILD_STATE "accel_master_axi_child_state"
#define ACCEL_MASTER_AXI_CHILD_STATE_BITWIDTH 3

#define MASTER_AXI_CHILD_STATE_IDLE 0
#define MASTER_AXI_CHILD_STATE_SPAD0_RD 1
#define MASTER_AXI_CHILD_STATE_SPAD1_RD 2
#define MASTER_AXI_CHILD_STATE_ACT_RD 3

// #define ACCEL_MASTER_AXI_CHILD_RD_RECV_VALID_FLAG                                     \
//   "accel_master_axi_child_rd_recv_valid_flag"
// #define ACCEL_MASTER_AXI_CHILD_RD_RECV_VALID_FLAG_BITWIDTH 1

/////////////////////////////////////////////
//      interanl states of Conv
/////////////////////////////////////////////
#define ACCEL_CONV_CHILD_VALID 1
#define ACCEL_CONV_CHILD_INVALID 0

#define ACCEL_CONV_CHILD_VALID_FLAG "accel_conv_child_valid_flag"
#define ACCEL_CONV_CHILD_VALID_FLAG_BITWIDTH 1

#define ACCEL_CONV_CHILD_STATE "accel_conv_child_state"
#define ACCEL_CONV_CHILD_STATE_BITWIDTH 5

#define CONV_CHILD_STATE_IDLE 0
//FSM state related to activation fetching
#define CONV_CHILD_STATE_ACT_FILTER_ID 1
#define CONV_CHILD_STATE_ACT_INPUT_CHANNEL_BLOCK 2
#define CONV_CHILD_STATE_ACT_INPUT_ROW 3
#define CONV_CHILD_STATE_ACT_INPUT_COL 4
#define CONV_CHILD_STATE_ACT_SEND_RD_REQ 5
#define CONV_CHILD_STATE_ACT_RECV_RD_RESP 6
// 08172020: add new instr to support abstraction of axi master interface
#define CONV_CHILD_STATE_ACT_SET_REQ_LEN 16
#define CONV_CHILD_STATE_ACT_FETCH_ACT 17
// FSM state related to weight fetching
#define CONV_CHILD_STATE_WEIGHT_INIT 7
#define CONV_CHILD_STATE_WEIGHT_ROW_FETCH 8
#define CONV_CHILD_STATE_WEIGHT_COL_FETCH 9
#define CONV_CHILD_STATE_WEIGHT_CHECK_BOUND 10
#define CONV_CHILD_STATE_WEIGHT_SEND_DP 11
// FSM state related to datapath 
#define CONV_CHILD_STATE_DP_MAC_PSUM 12
#define CONV_CHILD_STATE_FETCH_OUT_ACT 13
#define CONV_CHILD_STATE_BIAS_RELU 14
#define CONV_CHILD_STATE_OUT 15

#define CONV_CHILD_STATE_DONE 31

#define CONV_CHILD_FILTER_ID "conv_child_filter_id"
#define CONV_CHILD_FILTER_ID_BITWIDTH CONV_FILTER_SIZE_T

#define CONV_CHILD_CHAN_BLOCK_ID "conv_child_chan_block_id"
#define CONV_CHILD_CHAN_BLOCK_ID_BITWIDTH CONV_CHANNEL_SIZE_T

#define CONV_CHILD_INPUT_ROW_ID "conv_child_input_row_id"
#define CONV_CHILD_INPUT_ROW_ID_BITWIDTH CONV_ROW_SIZE_T

#define CONV_CHILD_INPUT_COL_ID "conv_child_input_col_id"
#define CONV_CHILD_INPUT_COL_ID_BITWIDTH CONV_ROW_SIZE_T

// update 08232020: add a new state for holding the values of column index 
// in the act fetch loop param (for multiple data fetching with one AXI cmd)
#define CONV_CHILD_INPUT_COL_ID_LOOP "conv_child_input_col_id_loop"
#define CONV_CHILD_INPUT_COL_ID_LOOP_BITWIDTH CONV_ROW_SIZE_T

#define CONV_CHILD_KERNEL_COL_ID "conv_child_kernel_col_id"
#define CONV_CHILD_KERNEL_COL_ID_BITWIDTH CONV_KERNEL_SIZE_T

#define CONV_CHILD_KERNEL_ROW_ID "conv_child_kernel_row_id"
#define CONV_CHILD_KERNEL_ROW_ID_BITWIDTH CONV_KERNEL_SIZE_T

#define CONV_CHILD_ACT_ARRAY "conv_child_act_array"
#define CONV_CHILD_ACT_ARRAY_0 "conv_child_act_array_0"
#define CONV_CHILD_ACT_ARRAY_1 "conv_child_act_array_1"
#define CONV_CHILD_ACT_ARRAY_2 "conv_child_act_array_2"
#define CONV_CHILD_ACT_ARRAY_3 "conv_child_act_array_3"
#define CONV_CHILD_ACT_ARRAY_4 "conv_child_act_array_4"
#define CONV_CHILD_ACT_ARRAY_5 "conv_child_act_array_5"
#define CONV_CHILD_ACT_ARRAY_6 "conv_child_act_array_6"
#define CONV_CHILD_ACT_ARRAY_7 "conv_child_act_array_7"

#define CONV_CHILD_ACT_ARRAY_BITWIDTH ACT_TOTAL_BITWIDTH

#define CONV_CHILD_WEIGHT_ADDR "conv_child_weight_addr"
#define CONV_CHILD_WEIGHT_ADDR_BITWIDTH TOP_SLAVE_ADDR_IN_BITWIDTH

#define CONV_CHILD_WEIGHT_ARRAY "conv_child_weight_array"
#define CONV_CHILD_WEIGHT_ARRAY_0 "conv_child_weight_array_0"
#define CONV_CHILD_WEIGHT_ARRAY_1 "conv_child_weight_array_1"
#define CONV_CHILD_WEIGHT_ARRAY_2 "conv_child_weight_array_2"
#define CONV_CHILD_WEIGHT_ARRAY_3 "conv_child_weight_array_3"
#define CONV_CHILD_WEIGHT_ARRAY_4 "conv_child_weight_array_4"
#define CONV_CHILD_WEIGHT_ARRAY_5 "conv_child_weight_array_5"
#define CONV_CHILD_WEIGHT_ARRAY_6 "conv_child_weight_array_6"
#define CONV_CHILD_WEIGHT_ARRAY_7 "conv_child_weight_array_7"

#define CONV_CHILD_WEIGHT_ARRAY_BITWIDTH WEIGHT_TOTAL_BITWIDTH

#define CONV_CHILD_O_ACT_ARRAY "conv_child_o_act_array"
#define CONV_CHILD_O_ACT_ARRAY_0 "conv_child_o_act_array_0"
#define CONV_CHILD_O_ACT_ARRAY_1 "conv_child_o_act_array_1"
#define CONV_CHILD_O_ACT_ARRAY_2 "conv_child_o_act_array_2"
#define CONV_CHILD_O_ACT_ARRAY_3 "conv_child_o_act_array_3"
#define CONV_CHILD_O_ACT_ARRAY_4 "conv_child_o_act_array_4"
#define CONV_CHILD_O_ACT_ARRAY_5 "conv_child_o_act_array_5"
#define CONV_CHILD_O_ACT_ARRAY_6 "conv_child_o_act_array_6"
#define CONV_CHILD_O_ACT_ARRAY_7 "conv_child_o_act_array_7"

#define CONV_CHILD_O_ACT_ARRAY_BITWIDTH ACT_TOTAL_BITWIDTH

#define CONV_CHILD_OUT_ARRAY "conv_child_out_array"
#define CONV_CHILD_OUT_ARRAY_0 "conv_child_out_array_0"
#define CONV_CHILD_OUT_ARRAY_1 "conv_child_out_array_1"
#define CONV_CHILD_OUT_ARRAY_2 "conv_child_out_array_2"
#define CONV_CHILD_OUT_ARRAY_3 "conv_child_out_array_3"
#define CONV_CHILD_OUT_ARRAY_4 "conv_child_out_array_4"
#define CONV_CHILD_OUT_ARRAY_5 "conv_child_out_array_5"
#define CONV_CHILD_OUT_ARRAY_6 "conv_child_out_array_6"
#define CONV_CHILD_OUT_ARRAY_7 "conv_child_out_array_7"

#define CONV_CHILD_OUT_ARRAY_BITWIDTH ACT_TOTAL_BITWIDTH

#define CONV_CHILD_MAC_PSUM "conv_child_mac_psum"
#define CONV_CHILD_MAC_PSUM_BITWIDTH PSUM_TOTAL_BITWIDTH

#define CONV_CHILD_ACTIVATION_PSUM "conv_child_activation_psum"
#define CONV_CHILD_ACTIVATION_PSUM_BITWIDTH ACT_TOTAL_BITWIDTH

// 08172020 model the multiple activation fetching behaviors
#define CONV_CHILD_ACT_REQ_LENGTH "conv_child_act_req_length"
#define CONV_CHILD_ACT_REQ_LENGTH_BITWIDTH CONV_CHILD_INPUT_COL_ID_BITWIDTH

#define CONV_CHILD_ACT_FETCH_CNTR "conv_child_act_fetch_cntr"
#define CONV_CHILD_ACT_FETCH_CNTR_BITWIDTH CONV_ROW_SIZE_T


//////////////////////////////////////////////////////////
// internal states for SPAD child instructions 
//////////////////////////////////////////////////////////
#define SPAD_CHILD_VALID_FLAG "spad_child_valid_flag"
#define SPAD_CHILD_VALID_FLAG_BITWIDTH 1

// counter for current read/write length
#define SPAD_RD_WR_CNTR "spad_rd_wr_cntr"
#define SPAD_RD_WR_CNTR_BITWIDTH CFG_REG_BITWIDTH

// reg for distinguish spad read/write on spad0 or spad1
#define SPAD_CHILD_TARGET "spad_child_target"
#define SPAD_CHILD_TARGET_BITWIDTH 1





} // namespace hlscnn
} // namespace ilang;

#endif // _INTERNAL_STATE_H__