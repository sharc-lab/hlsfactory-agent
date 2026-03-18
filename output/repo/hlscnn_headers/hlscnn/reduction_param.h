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

// File: reduction_param.h

#ifndef REDUCTION_PARAM_H__
#define REDUCTION_PARAM_H__

#include <hlscnn/top_config.h>
#include <hlscnn/common_config.h>
#include <cmath>

namespace ilang {
namespace hlscnn {

//////////////////////////
// reduction param info
// ref: utils_accel.h
/////////////////////////

#define REDUCTION_MAX_CHAN_NUM 1024
#define REDUCTION_MAX_ROW_NUM 1024
#define REDUCTION_NUM_CHAN_BITWIDTH (int)(std::ceil(std::log2(REDUCTION_MAX_CHAN_NUM)))
#define REDUCTION_NUM_ROW_BITWIDTH (int)(std::ceil(std::log2(REDUCTION_MAX_ROW_NUM)))

#define REDUCTION_BOOL_WIDTH 1

#define REDUCTION_INPUT_BASE_ADDR "reduction_input_base_addr"
#define REDUCTION_INPUT_BASE_ADDR_BITWIDTH TOP_SLAVE_ADDR_IN_BITWIDTH

#define REDUCTION_OUTPUT_BASE_ADDR "reduction_output_base_addr"
#define REDUCTION_OUTPUT_BASE_ADDR_BITWIDTH TOP_SLAVE_ADDR_IN_BITWIDTH

#define REDUCTION_INPUT_ROW_NUM "reduction_input_row_num"
#define REDUCTION_INPUT_ROW_NUM_BITWIDTH REDUCTION_NUM_ROW_BITWIDTH

#define REDUCTION_INPUT_COL_NUM "reduction_input_col_num"
#define REDUCITON_INPUT_COL_NUM_BITWIDTH REDUCTION_NUM_ROW_BITWIDTH

#define REDUCTION_INPUT_CHAN_NUM "reduction_input_chan_num"
#define REDUCTION_INPUT_CHAN_NUM_BITWIDTH REDUCTION_NUM_CHAN_BITWIDTH

#define REDUCTION_ACT_FUNC "reduction_act_func"
#define REDUCTION_ACT_FUNC_BITWIDTH ACT_FUNC_WIDTH

// The following are declared as ac_fixed, which should be used uninterpreted function to handle it
#define REDUCTION_BIAS "reduction_bias"
#define REDUCTION_BIAS_BITWIDTH WEIGHT_TOTAL_BITWIDTH

#define REDUCTION_RELU_THRESHOLD "reduction_relu_threshold"
#define REDUCTION_RELU_THRESHOLD_BITWIDTH RELU_THRESHOLD_WIDTH



} // namespace hlscnn
} // namespace ilang

#endif // REDUCTION_PARAM_H__