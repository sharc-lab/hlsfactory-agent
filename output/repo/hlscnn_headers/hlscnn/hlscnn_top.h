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

// File: hlscnn_top.h

#ifndef HLSCNN_TOP_H__
#define HLSCNN_TOP_H__

#include <ilang/ilang++.h>
#include <string>
#include <ilang/util/log.h>

#include <hlscnn/top_config.h>
#include <hlscnn/common_config.h>
#include <hlscnn/config_reg.h>
#include <hlscnn/conv_param.h>
#include <hlscnn/fc_param.h>
#include <hlscnn/reduction_param.h>
#include <hlscnn/internal_state.h>
#include <hlscnn/utils.h>
#include <hlscnn/uninterpreted_func.h>

namespace ilang {

namespace hlscnn {

Ila GetHlscnnIla(const std::string& model_name = "hlscnn");

void DefineTopIO(Ila& m);

void DefineConfigReg(Ila& m);
void DefineFCParam(Ila& m);
void DefineConvParam(Ila& m);
void DefineReduceParam(Ila& m);

void DefineArchState(Ila& m);
void DefineInternalState(Ila& m);

void DefineInitCond(Ila& m);

void DefineConfigInstr(Ila& m);
void DefineSPADInstr(Ila& m);
void DefineAccelConvTrigger(Ila& m);

void DefineVirMemInstr(Ila& m);
// child instructions
void DefineAXIMasterChild(Ila& m);
void DefineAccelConvChild(Ila& m);
void DefineSPADInstrChild(Ila& m);

}
};

#endif // HLSCNN_TOP_H__
