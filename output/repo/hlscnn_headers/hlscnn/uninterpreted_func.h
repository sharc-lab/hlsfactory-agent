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

// File: uninterpreted_func.h

#ifndef UNINTERPRETED_FUNC_H__
#define UNINTERPRETED_FUNC_H__

#include <ilang/ilang++.h>
#include <vector>

namespace ilang {
namespace hlscnn {

static auto mul_in = SortRef::BV(WEIGHT_TOTAL_BITWIDTH);
static auto mul_out = SortRef::BV(PSUM_TOTAL_BITWIDTH);
static auto psum_type = SortRef::BV(PSUM_TOTAL_BITWIDTH);
static auto act_type = SortRef::BV(ACT_TOTAL_BITWIDTH);

static std::vector<SortRef> ConvMac_in = {psum_type, mul_in, mul_in};

static FuncRef ConvMac("ConvMac", psum_type, ConvMac_in);
static FuncRef ConvMacPsum2Act("ConvMacPsum2Act", act_type, psum_type);

static auto act_psum_type = SortRef::BV(ACT_TOTAL_BITWIDTH);


static FuncRef ActAdd2Psum("ActAdd2Psum", psum_type, act_type, act_type);
static FuncRef ConvAddBias("ConvAddBias", psum_type, psum_type, act_type);

static FuncRef Psum2Act("Psum2Act", act_psum_type, psum_type);
static FuncRef PsumRelu("PsumRelu", psum_type, psum_type);

} // namespace ilang
} // namespace hlscnn

#endif