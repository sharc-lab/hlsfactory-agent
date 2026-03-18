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

// File: fc_param.h

// This file contains info related to FC

#ifndef FC_PARAM_H__
#define FC_PARAM_H__

#include <hlscnn/top_config.h>
#include <hlscnn/common_config.h>

namespace ilang {
namespace hlscnn {

//////////////////////////
// FC param info
// ref: utils_accel.h
/////////////////////////

#define FC_ADDR_WIDTH 32
#define FC_DATA_WIDTH 12
#define FC_BOOL_WIDTH 1
#define FC_BATCH_WIDTH 3

#define FC_WEIGHT_BASE "fc_weight_base"
#define FC_WEIGHT_BASE_BITWIDTH FC_ADDR_WIDTH

#define FC_ACT_BASE "fc_act_base"
#define FC_ACT_BASE_BITWIDTH FC_ADDR_WIDTH

#define FC_NUM_ROWS "fc_num_rows"
#define FC_NUM_ROWS_BITWIDTH FC_DATA_WIDTH

#define FC_NUM_COLS "fc_num_cols"
#define FC_NUM_COLS_BITWIDTH FC_DATA_WIDTH

#define FC_RUN_BIAS "fc_run_bias"
#define FC_RUN_BIAS_BITWIDTH FC_BOOL_WIDTH

#define FC_BATCH_SIZE "fc_batch_size"
#define FC_BATCH_SIZE_BITWIDTH FC_BATCH_WIDTH

#define FC_ACT_FUNC "fc_activation_func"
#define FC_ACT_FUNC_BITWIDTH ACT_FUNC_WIDTH

#define FC_RELU_THRESHOLD "fc_relu_threshold"
#define FC_RELU_THRESHOLD_BITWIDTH RELU_THRESHOLD_WIDTH

} // namespace hlscnn
} // namespace ilang

#endif // FC_PARAM_H__