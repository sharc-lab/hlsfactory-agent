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

// File: utils.h

#ifndef UTILS_H__
#define UTILS_H__

#include <ilang/ilang++.h>
#include <hlscnn/hlscnn_top.h>

namespace ilang {
namespace hlscnn {

inline void DefineCfgReg_helper(Ila& m, const std::string& reg_name) {
  m.NewBvState(reg_name, CFG_REG_BITWIDTH);
}

inline std::string GetStateName(const std::string& name, const int& idx) {
  return (name + "_" + std::to_string(idx));
}

inline ExprRef GetActVectorState(const Ila& child, const std::string& name,
                                                        const ExprRef& idx) {
  auto value = 
    Ite(idx == 0, child.state(GetStateName(name, 0)),
    Ite(idx == 1, child.state(GetStateName(name, 1)),
    Ite(idx == 2, child.state(GetStateName(name, 2)),
    Ite(idx == 3, child.state(GetStateName(name, 3)),
    Ite(idx == 4, child.state(GetStateName(name, 4)),
    Ite(idx == 5, child.state(GetStateName(name, 5)),
    Ite(idx == 6, child.state(GetStateName(name, 6)),
                  child.state(GetStateName(name, 7))
                  )))))));
  return value;
}

ExprRef act_gen_get_addr(const Ila& child, const ExprRef& input_row,
                                                  const ExprRef& input_col,
                                                  const ExprRef& chan_block);

ExprRef conv_out_of_bound(const Ila& child, const ExprRef& input_row,
                                                   const ExprRef& input_col,
                                                   const ExprRef& k_row,
                                                   const ExprRef& k_col);

ExprRef WtGetAddr(const Ila& child, const ExprRef& filter_id,
                                               const ExprRef& k_row,
                                               const ExprRef& k_col,
                                               const ExprRef& chan_block);

ExprRef OutActGetAddr(const Ila& child, const ExprRef& input_row,
                                        const ExprRef& input_col,
                                        const ExprRef& k_row,
                                        const ExprRef& k_col,
                                        const ExprRef& filter_idx);

ExprRef WtIsLastPsum(const Ila& child, const ExprRef& act_row,
                                       const ExprRef& act_col,
                                       const ExprRef& k_row,
                                       const ExprRef& k_col,
                                       const ExprRef& chan_block);

ExprRef GetCfgRegAlignedData();

void SetConfigRegWrInstr(Ila& m, const int& reg_idx, const std::string& reg_name);


}

}

#endif // UTILS_H__