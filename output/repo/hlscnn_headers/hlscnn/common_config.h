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

// File: common_config.h

// This file contains info from hlscnn: common.h

#ifndef COMMON_CONFIG_H__
#define COMMON_CONFIG_H__

#include <hlscnn/top_config.h>

namespace ilang {
namespace hlscnn {

#define NIC_MEM_ELEM_BYTEWIDTH 16 // in bytes

// FXP weight params
#define WEIGHT_TOTAL_BITWIDTH 16
#define WEIGHT_NUM_PER_NIC (NIC_MEM_ELEM_BYTEWIDTH/(WEIGHT_TOTAL_BITWIDTH/8))
// FXP activation params
#define ACT_TOTAL_BITWIDTH 16
#define ACT_NUM_PER_NIC (NIC_MEM_ELEM_BYTEWIDTH/(ACT_TOTAL_BITWIDTH/8))
// FXP PSUM(accumulator) params
#define PSUM_TOTAL_BITWIDTH 32
#define PSUM_INT_BITWIDTH 16
#define PSUM_FRAC_BITWIDTH (PSUM_TOTAL_BITWIDTH - PSUM_INT_BITWIDTH)
// Output activations size
#define OUTPUT_TOTAL_BITWIDTH 16
// accelerator param
#define DPATH_LANE_NUM WEIGHT_NUM_PER_NIC
#define FC_MAC_BATCH 8
// channel block tiling params
#define CHANNEL_BLOCK_SIZE ACT_NUM_PER_NIC

////////////////////////////////////////////
//            FC related
////////////////////////////////////////////
#define FC_WEIGHT_INT_BITWIDTH 2
#define FC_WEIGHT_FRAC_BITWIDTH (WEIGHT_TOTAL_BITWIDTH - FC_WEIGHT_INT_BITWIDTH)

#define FC_ACT_INT_BITWIDTH 9
#define FC_ACT_FRAC_BITWIDTH (ACT_TOTAL_BITWIDTH - FC_ACT_INT_BITWIDTH)

#define FC_OUTPUT_INT_BITWIDTH 12
#define FC_OUTPUT_FRAC_BITWIDTH (OUTPUT_TOTAL_BITWIDTH - FC_OUTPUT_INT_BITWIDTH)

////////////////////////////////////////////
//            conv related
////////////////////////////////////////////
#define CONV_WEIGHT_INT_BITWIDTH 2
#define CONV_WEIGHT_FRAC_BITWIDTH (WEIGHT_TOTAL_BITWIDTH - FC_WEIGHT_INT_BITWIDTH)

#define CONV_ACT_INT_BITWIDTH 9
#define CONV_ACT_FRAC_BITWIDTH (ACT_TOTAL_BITWIDTH - FC_ACT_INT_BITWIDTH)

#define CONV_OUTPUT_INT_BITWIDTH 12
#define CONV_OUTPUT_FRAC_BITWIDTH (OUTPUT_TOTAL_BITWIDTH - FC_OUTPUT_INT_BITWIDTH)

// conv_internal.h
#define CONV_VECTOR_SIZE 8

// this part is from utils_accel.h
#define ACT_FUNC_WIDTH 2
#define RELU_THRESHOLD_WIDTH 22

} // namespace hlscnn
} // namespace ilang

#endif // COMMON_CONFIG_H__