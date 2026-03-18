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

// File: conv_param.h

#ifndef CONV_PARAM_H__
#define CONV_PARAM_H__

#include <hlscnn/top_config.h>
#include <hlscnn/common_config.h>
#include <cmath>

namespace ilang {
namespace hlscnn {

//////////////////////////
// CONV param info
// ref: utils_accel.h
/////////////////////////

#define CONV_MAX_ROWS 1024
#define CONV_MAX_KERNEL_SIZE 256
#define CONV_MAX_KERNEL_NUM 4096
#define CONV_MAX_FILTER_NUM 256
#define CONV_NUM_ROW_BITWIDTH (int)(std::ceil(std::log2(CONV_MAX_ROWS)))
#define CONV_NUM_CHAN_BITWIDTH (int)(std::ceil(std::log2(CONV_MAX_KERNEL_NUM)))
#define CONV_NUM_FILTER_BITWIDTH (int)(std::ceil(std::log2(CONV_MAX_FILTER_NUM)))
#define CONV_KERNEL_SIZE_BITWIDTH (int)(std::ceil(std::log2(CONV_MAX_KERNEL_SIZE)))

#define CONV_ROW_SIZE_T (CONV_NUM_ROW_BITWIDTH + 1)
#define CONV_CHANNEL_SIZE_T (CONV_NUM_CHAN_BITWIDTH + 1)
#define CONV_KERNEL_SIZE_T (CONV_KERNEL_SIZE_BITWIDTH + 1)
#define CONV_FILTER_SIZE_T (CONV_NUM_FILTER_BITWIDTH + 1)

#define CONV_BOOL_WIDTH 1

#define CONV_ACT_BASE "conv_activation_base"
#define CONV_ACT_BASE_BITWIDTH TOP_SLAVE_ADDR_IN_BITWIDTH

#define CONV_WEIGHT_BASE "conv_weight_base"
#define CONV_WEIGHT_BASE_BITWIDTH TOP_SLAVE_ADDR_IN_BITWIDTH

#define CONV_SPAD_OUTPUT_BASE "conv_spad_output_base"
#define CONV_SPAD_OUTPUT_BASE_BITWIDTH TOP_SLAVE_ADDR_IN_BITWIDTH

#define CONV_INPUT_ROW_NUM "conv_input_row_num"
#define CONV_INPUT_ROW_NUM_BITWIDTH CONV_NUM_ROW_BITWIDTH

#define CONV_INPUT_COL_NUM "conv_input_col_num"
#define CONV_INPUT_COL_NUM_BITWIDTH CONV_NUM_ROW_BITWIDTH

#define CONV_INPUT_CHAN_NUM "conv_input_chan_num"
#define CONV_INPUT_CHAN_NUM_BITWIDTH CONV_NUM_CHAN_BITWIDTH 

#define CONV_OUTPUT_CHAN_NUM "conv_output_chan_num"
#define CONV_OUTPUT_CHAN_NUM_BITWIDTH CONV_NUM_CHAN_BITWIDTH

#define CONV_KERNEL_ROW_NUM "conv_kernel_row_num"
#define CONV_KERNEL_ROW_NUM_BITWIDTH CONV_KERNEL_SIZE_BITWIDTH

#define CONV_KERNEL_COL_NUM "conv_kernel_col_num"
#define CONV_KERNEL_COL_NUM_BITWIDTH CONV_KERNEL_SIZE_BITWIDTH

#define CONV_KERNEL_C_STRIDE "conv_kernel_c_stride"
#define CONV_KERNEL_C_STRIDE_BITWIDTH CONV_KERNEL_SIZE_BITWIDTH

#define CONV_KERNEL_R_STRIDE "conv_kernel_r_stride"
#define CONV_KERNEL_R_STRIDE_BITWIDTH CONV_KERNEL_SIZE_BITWIDTH

#define CONV_ENABLE_BIAS "conv_enable_bias"
#define CONV_ENABLE_BIAS_BITWIDTH CONV_BOOL_WIDTH

#define CONV_ENABLE_RELU "conv_enable_relu"
#define CONV_ENABLE_RELU_BITWIDTH CONV_BOOL_WIDTH

#define CONV_ENABLE_ACCUM "conv_enable_accum"
#define CONV_ENABLE_ACCUM_BITWIDTH CONV_BOOL_WIDTH

#define CONV_ENABLE_WB "conv_enable_wb"
#define CONV_ENABLE_WB_BITWIDTH CONV_BOOL_WIDTH

#define CONV_OFILTER_IDX "conv_ofilter_idx"
#define CONV_OFILTER_IDX_BITWIDTH CONV_NUM_FILTER_BITWIDTH

#define CONV_OUTPUT_ROW_NUM "conv_output_row_num"
#define CONV_OUTPUT_ROW_NUM_BITWIDTH CONV_NUM_ROW_BITWIDTH

#define CONV_OUTPUT_COL_NUM "conv_output_col_num"
#define CONV_OUTPUT_COL_NUM_BITWIDTH CONV_NUM_ROW_BITWIDTH


// The following are declared as ac_fixed, which should be used uninterpreted function to handle it
#define CONV_CHAN_BIAS "conv_chan_bias"
#define CONV_CHAN_BIAS_BITWIDTH WEIGHT_TOTAL_BITWIDTH

// 08142020: model multiple activation fetch request in activation fetching
#define CONV_BURST_LENGTH 8


} // namespace hlscnn
} // namespace ilang

#endif // CONV_PARAM_H__