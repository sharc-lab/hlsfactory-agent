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

// File: config_reg.h
// this files defines const used for hlscnn config registers
// ref: top/memory_map.h

#ifndef CONFIG_REG_H__
#define CONFIG_REG_H__

#include <cmath>

namespace ilang {
namespace hlscnn {
  
  // CFG parameters
  #define CFG_REG_BITWIDTH 32
  #define CFG_REG_BYTEWIDTH CFG_REG_BITWIDTH/8
  #define CFG_REG_SIZE_BITWIDTH (int)(std::floor(std::log2(CFG_REG_BYTEWIDTH)))

  /*********************************************************/
  // define config state id, same as hlscnn design
  /*********************************************************/
  enum ConfigRegId {
    // Unused configuration registers.
    ConfigReg1,
    ConfigReg2,
    ConfigReg3,
    ConfigReg4,

    SocMemBaseAddr,
    SocMemRdWrLength,
    AccelSpadCFG,
    AxiMasterSelfTestTrigger,
    AxiMasterSelfTestRemoteBaseAddr,
    AcpMasterSelfTestTrigger,
    AcpMasterSelfTestRemoteBaseAddr,
    AxiAttrConfig,
    AccelStartFlagReg,
    AccelFCWeightsBase,
    AccelFCActivationBase,
    AccelFCSizeConfig,
    AccelBiasActivationConfig,
    AccelConvTrigger,
    AccelConvActivationBaseAddr,
    AccelConvWeightsBaseAddr,
    AccelConvOutputsBaseAddr,
    AccelConvInputSizeConfig,
    AccelConvOutputSizeConfig,
    AccelConvKernelSizeConfig,
    AccelConvChannelConfig,
    AccelReductionTrigger,
    AccelReductionInputBaseAddr,
    AccelReductionOutputBaseAddr,
    AccelReductionInputSizeConfig,
    AccelReductionBiasConfig,
    NumCfgRegisters
  };

  /*********************************************************/
  // define config state names
  /*********************************************************/

  //SocMemBaseAddr is used to specify base address of UMEM or RRAM
  //from which activation/weights will be read from (or written to)
  #define CFG_REG_SOC_MEM_BASE_ADDR "cfg_reg_soc_mem_base_addr"

  //SocMemRdWrLength is used to specify the number of sequential read or write transactions
  //starting from the base address specified in SocMemBaseAddr
  #define CFG_REG_SOC_MEM_RD_WR_LENGTH "cfg_reg_soc_mem_rd_wr_length"

  //Scratchpad configurations
  #define CFG_REG_ACCEL_SPAD_CFG "cfg_reg_accel_spad_cfg"

  // -------------------------------------------
  //  Fully connected configuration registers.
  // -------------------------------------------
  // TODO: the reg name in hlscnn code is stilll "AccelStartFlagReg"
  #define CFG_REG_ACCEL_FC_START_FLAG_REG "cfg_reg_accel_fc_start_flag_reg"
  #define CFG_REG_ACCEL_FC_WEIGHT_BASE "cfg_reg_accel_fc_weight_base"
  #define CFG_REG_ACCEL_FC_ACT_BASE "cfg_reg_accel_fc_act_base"

  // Layout of the AccelFCSizeConfig register. #15
  //
  // Batch size is encoded as the actual batch size - 1.
  //
  // | Unused |  Batch size |  Rows   |  Cols  |
  // -------------------------------------------
  // | 31-27  |    26-24    |  23-12  |  11-0  |
  // -------------------------------------------
  #define CFG_REG_ACCEL_FC_SIZE_CONFIG "cfg_reg_accel_fc_size_config"

  // Layout of the AccelBiasActivationConfig register.
  //
  // This controls whether ReLU and/or the biases are handled or not. Numbered
  // ranges denote the bit positions that store each value.
  //
  // This is used by FC and Reduction.
  //
  // Activation func encodings:
  //    - Do not run activation func: 0x0
  //    - ReLU: 0x1
  //    - ReLU threshold: 0x2
  //
  // | Unused | ReLU threshold | Do bias | Act. Func |
  // -------------------------------------------------
  // | 31-25  |   24-3  (22b)  |   2     |    1-0    |
  // -------------------------------------------------
  #define CFG_REG_ACCEL_BIAS_ACT_CONFIG "cfg_reg_accel_bias_act_config"

  // -------------------------------------------
  //  Convolutional configuration registers.
  // -------------------------------------------
  #define CFG_REG_ACCEL_CONV_TRIGGER "cfg_reg_accel_conv_trigger"

  #define CFG_REG_ACCEL_CONV_ACT_BASE_ADDR "cfg_reg_accel_conv_act_base_addr"
  
  #define CFG_REG_ACCEL_CONV_WEIGHT_BASE_ADDR "cfg_reg_accel_conv_weight_base_addr"

  #define CFG_REG_ACCEL_CONV_OUTPUT_BASE_ADDR "cfg_reg_accel_conv_output_base_addr"

  // Layout of the AccelConvInputSizeConfig register.
  //
  // | Input channels | Input rows | Input cols |
  // --------------------------------------------
  // |      31-20     |    19-10   |     9-0    |
  // --------------------------------------------
  #define CFG_REG_ACCEL_CONV_INPUT_SIZE_CFG "cfg_reg_accel_conv_input_size_cfg"

  // Layout of the AccelConvOutputSizeConfig register.
  //
  // | Output channels | Output rows | Output cols |
  // -----------------------------------------------
  // |       31-20     |     19-10   |     9-0     |
  // -----------------------------------------------
  #define CFG_REG_ACCEL_CONV_OUTPUT_SIZE_CFG "cfg_reg_accel_conv_output_size_cfg"

  // Layout of the AccelKernelSizeConfig register.
  //
  // | Unused | Kernel Stride | Kernel rows | Kernel cols |
  // ------------------------------------------------------
  // |  31-24 |    23:16      |    15-8     |     7-0     |
  // ------------------------------------------------------
  #define CFG_REG_ACCEL_KERNEL_SIZE_CFG "cfg_reg_accel_kernel_size_cfg"

  // Layout of the AccelConvChannelConfig register.
  //
  // Note that the number of filters is equal to the output channel dimension,
  // so we don't include space for it here.
  /// This is done to accomodate all the
  /// weird tall and skinny kernels that
  /// come aross across in Voice recognition(Deep Speech)/ NLP Networks.
  // | Unused| ENABLE_WB |   FILTER_IDX        | ENABLE_ACCUMLATION_ON_OUTPUT_SPAD |ENABLE_RELU         |ENABLE_BIAS|CHANNEL_BIAS         |
  //
  // ------------------------------------------------------------------------------------------------------------------
  // |  31-24|    22     |  21-19              |     18                            |         17         |     16   |     15-0            |
  // ------------------------------------------------------------------------------------------------------------------
  #define CFG_REG_ACCEL_CONV_CHANNEL_CFG "cfg_reg_accel_conv_channel_cfg"



  // -------------------------------------------
  //  Reduction configuration registers.
  // -------------------------------------------
  #define CFG_REG_ACCEL_REDUCTION_TRIGGER "cfg_reg_accel_reduction_trigger"

  #define CFG_REG_ACCEL_REDUCTION_INPUT_BASE_ADDR "cfg_reg_accel_reduction_input_base_addr"

  #define CFG_REG_ACCEL_REDUCTION_OUTPUT_BASE_ADDR "cfg_reg_accel_reduction_output_base_addr"

  // Layout of the AccelReductionInputSizeConfig register.
  //
  // | Unused | Input channels |  Rows  | Columns |
  // ----------------------------------------------
  // | 31-30  |     29-20      | 19-10  |   9-0   |
  // ----------------------------------------------
  #define CFG_REG_ACCEL_REDUCTION_INPUT_SIZE_CFG "cfg_reg_accel_reduction_input_size_cfg"

  // Layout of the AccelReductionBiasConfig register.
  //
  // Use this register in conjunction with the AccelBiasActivationConfig
  // register.
  //
  // |  Unused  |   Bias   |
  // -----------------------
  // |  31-16   |   15-0   |
  // ----------------------------------------------
  #define CFG_REG_ACCEL_REDUCTION_BIAS_CONFIG "cfg_reg_accel_reduction_bias_config"

} // namespace hlscnn
} // namespace ilang


#endif // CONFIG_REG_H__