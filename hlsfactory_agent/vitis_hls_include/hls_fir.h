// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.

// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.

// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689

#ifndef X_HLS_FIR_H
#define X_HLS_FIR_H

/*
 * This file contains a C++ model of hls::fir.
 * It defines Vivado_HLS synthesis model.
 */
#ifndef __cplusplus
#error C++ is required to include this header file
#else

#include "ap_int.h"
#include "hls_fence.h"
#include "etc/ap_utils.h"
#include "hls_stream.h"
#include <complex>
#ifndef __SYNTHESIS__
#include <math.h>
#endif
#define CEIL(a) (((a) == (int) (a)) ? (int) (a) : (((int) (a)) + 1))
#define FLOOR(a) ((int) (a))

#ifndef AESL_SYN
#ifndef __SYNTHESIS__
#include <iostream>
#include "fir/fir_compiler_v7_2_bitacc_cmodel.h"
#endif
#endif

#include <assert.h>
#define TABLE_SIZE 54

namespace hls {

#ifdef AESL_SYN
#include "etc/autopilot_ssdm_op.h"
#endif

namespace ip_fir {

#ifdef __SYNTHESIS__
#ifndef AP_INLINE
#define AP_INLINE inline __attribute__((always_inline))
#endif
#else
#ifndef AP_INLINE
#define AP_INLINE inline
#endif
#endif // __SYNTHESIS__



static const char* firErrChkHead = "ERROR:hls::fir ";

enum filter_type {single_rate = 0, interpolation, decimation, hilbert_filter, interpolated};
static const char* fir_filter_type_str[] = {
    "single_rate", "interpolation", 
    "decimation", "hilbert", "interpolated"
};

enum rate_change_type {integer = 0, fixed_fractional};
static const char* fir_rate_change_type_str[] = {
    "integer", "fixed_fractional"
};

enum chan_seq {basic = 0, advanced};
static const char* fir_channel_sequence_str[] = {
    "basic", "advanced"
};

enum rate_specification {frequency = 0, input_period, output_period};
static const char* fir_ratespecification_str[] = {
    "frequency_specification", "input_sample_period", "output_sample_period"
};

enum value_sign {value_signed = 0, value_unsigned};
static const char* fir_value_sign_str[] = {"signed", "unsigned"};

enum quantization {integer_coefficients = 0, quantize_only, maximize_dynamic_range};
static const char* fir_quantization_str[] = {
    "integer_coefficients", "quantize_only", "maximize_dynamic_range"
};

enum coeff_structure {inferred = 0, non_symmetric, symmetric, negative_symmetric, half_band, hilbert};
static const char* fir_coeff_struct_str[] = {
    "inferred", "non_symmetric", "symmetric",
    "negative_symmetric", "half_band", "hilbert"
};

enum output_rounding_mode {full_precision = 0, truncate_lsbs, non_symmetric_rounding_down,
                           non_symmetric_rounding_up, symmetric_rounding_to_zero,
                           symmetric_rounding_to_infinity, convergent_rounding_to_even,
                           convergent_rounding_to_odd};
static const char* fir_output_rounding_mode_str[] = {
    "full_precision", "truncate_lsbs", "non_symmetric_rounding_down",
    "non_symmetric_rounding_up", "symmetric_rounding_to_zero",
    "symmetric_rounding_to_infinity", "convergent_rounding_to_even",
    "convergent_rounding_to_odd"
};

enum filter_arch {systolic_multiply_accumulate = 0, transpose_multiply_accumulate};
static const char* fir_filter_arch_str[] = {
    "systolic_multiply_accumulate", "transpose_multiply_accumulate"
};

enum optimization_goal {area = 0, speed};
static const char* fir_opt_goal_str[] = {"area", "speed"};

enum config_sync_mode {on_vector = 0, on_packet};
static const char* fir_s_config_sync_mode_str[] = {"on_vector", "on_packet"};

enum config_method {single = 0, by_channel};
static const char* fir_s_config_method_str[] = {"single", "by_channel"};

//This prime number list is used to test whether 2 not equal and less than 512 numbers are coprim or not.
//Since they are not equal, the max common prime number need to be tried should be 251 which <= 512 / 2.
//Otherwise, the two test numbers must be coprim.
static const unsigned primeNums[TABLE_SIZE] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 
    233, 239, 241, 251};

#ifndef OVERRIDE
#define OVERRIDE(Param) static constexpr decltype(Default::Param) Param
#endif

struct params_t {
    using Default = params_t;
    static const unsigned input_width = 16; // old
    static const unsigned data_width = 16; // new
    static const unsigned input_fractional_bits = 0; // old
    static const unsigned data_fract_width = 0; // new
    static const unsigned output_width = 24;
    static const unsigned output_fractional_bits = 0; // old
    static const unsigned output_fract_width = 0; // new
    static const unsigned coeff_width = 16;
    static const unsigned coeff_fractional_bits = 0; // old
    static const unsigned coeff_fract_width = 0; // new
    static const unsigned config_width = 8;
    static const unsigned num_coeffs = 21;
    static const unsigned coeff_sets = 1;
    static const unsigned input_length = num_coeffs; // old
    static const unsigned data_length = num_coeffs; // new
    static const unsigned output_length = num_coeffs;
    static const unsigned num_channels = 1;

    static const unsigned total_num_coeff = num_coeffs * coeff_sets;
    static const double coeff_vec[];
    static const bool reloadable = false;
    static const unsigned filter_type = single_rate;
    static const unsigned rate_change = integer;
    static const unsigned interp_rate = 1;
    static const unsigned decim_rate = 1;
    static const unsigned zero_pack_factor = 1;
    static const unsigned chan_seq = basic;
    static const unsigned rate_specification = input_period;
#ifndef __GXX_EXPERIMENTAL_CXX0X__
    static const double sample_period = 1;

    static const double sample_frequency = 0.001;
#else
    static constexpr double sample_period = 1;
    static constexpr double sample_frequency = 0.001;
#endif
    static const unsigned quantization = integer_coefficients;
    static const bool best_precision = false;
    static const unsigned coeff_structure = non_symmetric;
    static const unsigned output_rounding_mode = full_precision;
    static const unsigned filter_arch = systolic_multiply_accumulate;
    static const unsigned optimization_goal = area;
    static const unsigned inter_column_pipe_length = 4;
    static const unsigned column_config = 1;
    static const unsigned config_sync_mode = on_vector;
    static const unsigned config_method = single;
    static const unsigned coeff_padding = 0;

    static const unsigned num_paths = 1;
    static const unsigned data_sign = value_signed;
    static const unsigned coeff_sign = value_signed;
};

template<bool _COND>
int constexpr my_assert() {
    static_assert(_COND, "Both old and new parameter name used for hls::ip_fir::params_t");
    return 0;
}
#ifdef __SYNTHESIS__
#define HLS_FIR_CHECK(new_name, old_name, def_val) \
    (_CONFIG_T::new_name != def_val ? _CONFIG_T::new_name : _CONFIG_T::old_name)
#else
#define HLS_FIR_CHECK(new_name, old_name, def_val) \
    (hls::ip_fir::my_assert<_CONFIG_T::new_name == def_val || _CONFIG_T::old_name == def_val>, \
    _CONFIG_T::new_name != def_val ? _CONFIG_T::new_name : _CONFIG_T::old_name)
#endif

// Redefinitions for new options
#define _CONFIG_T_coeff_fractional_bits \
  HLS_FIR_CHECK(coeff_fract_width, coeff_fractional_bits, 0) 
#define _CONFIG_T_input_width \
  HLS_FIR_CHECK(data_width, input_width, 16) 
#define _CONFIG_T_input_fractional_bits \
  HLS_FIR_CHECK(data_fract_width, input_fractional_bits, 0) 
#define _CONFIG_T_output_fractional_bits \
  HLS_FIR_CHECK(output_fract_width, output_fractional_bits, 0) 
#define _CONFIG_T_input_length \
  HLS_FIR_CHECK(data_length, input_length, ip_fir::params_t::num_coeffs) 


constexpr unsigned calcInFactor(double sample_period, int filter_type)
{
    unsigned factor = 1;
    if (sample_period >= 1.0)
       return factor;
    if (filter_type < hls::ip_fir::single_rate || filter_type > hls::ip_fir::decimation)
       return factor;
    factor = FLOOR(1.0/sample_period);
    return factor;
}

constexpr unsigned calcOutFactor(double sample_period, int filter_type, int rate_type, unsigned interp_rate, unsigned decim_rate)
{
    
    unsigned factor = 1;
    unsigned ssr_factor = 1;
    if (sample_period >= 1.0)
        ssr_factor = 1;
    else if (filter_type < hls::ip_fir::single_rate || filter_type > hls::ip_fir::decimation)
        ssr_factor = 1;
    else
        ssr_factor = FLOOR(1.0/sample_period);

    if (filter_type == hls::ip_fir::single_rate)
        factor = ssr_factor;
    else if (filter_type == hls::ip_fir::interpolation)
    {
        if (rate_type == hls::ip_fir::integer)
            factor = interp_rate * ssr_factor;
        else if (rate_type == hls::ip_fir::fixed_fractional)
            factor = CEIL((1.0 * interp_rate) / decim_rate) * ssr_factor;
    }
    else if (filter_type == hls::ip_fir::decimation)
    {
        if (rate_type == hls::ip_fir::integer)
            factor = CEIL((1.0 * ssr_factor) / decim_rate);
        // decimtion fraction rate FIR does not support SSR
        else if (rate_type == hls::ip_fir::fixed_fractional)
            factor = CEIL((1.0 * interp_rate) / decim_rate);
    }

    return factor;
}

#ifndef AESL_SYN
//---------------------------------------------------------------------------------------------------------------------
// Example message handler
static void msg_print(void* handle, int error, const char* msg)
{
    printf("%s\n",msg);
}
#endif
} // namespace hls::ip_fir

using namespace std;

template<typename _CONFIG_T>
class FIR {
private:
    static const unsigned input_axi_width = ((_CONFIG_T_input_width+7)>>3)<<3;    
    static const unsigned output_axi_width = ((_CONFIG_T::output_width+7)>>3)<<3;    
    static const unsigned coeff_axi_width = ((_CONFIG_T::coeff_width+7)>>3)<<3;    

    static constexpr unsigned ssr_rate = 
        (_CONFIG_T::sample_period >= 1.0) ? 
            1 
            : 
            ((_CONFIG_T::filter_type < hls::ip_fir::single_rate || _CONFIG_T::filter_type > hls::ip_fir::decimation) ? 
                1 
                : 
                FLOOR(1.0 / _CONFIG_T::sample_period)
            );
    static constexpr unsigned input_grouping_factor = ssr_rate;
    static constexpr unsigned output_decoupling_factor = 
        (_CONFIG_T::filter_type == hls::ip_fir::single_rate) ? 
            ssr_rate 
            : 
            ((_CONFIG_T::filter_type == hls::ip_fir::interpolation) ? 
                ((_CONFIG_T::rate_change == hls::ip_fir::integer) ? 
                    _CONFIG_T::interp_rate * ssr_rate
                    : 
                    ((_CONFIG_T::rate_change == hls::ip_fir::fixed_fractional) ? 
                        CEIL((1.0 * _CONFIG_T::interp_rate) / _CONFIG_T::decim_rate) * ssr_rate 
                        : 
                        1
                    )
                ) 
                : 
                ((_CONFIG_T::filter_type == hls::ip_fir::decimation) ? 
                    ((_CONFIG_T::rate_change == hls::ip_fir::integer) ? 
                        CEIL((1.0 * ssr_rate) / _CONFIG_T::decim_rate)
                        :
                        ((_CONFIG_T::rate_change == hls::ip_fir::fixed_fractional) ?
                            CEIL((1.0 * _CONFIG_T::interp_rate) / _CONFIG_T::decim_rate)
                            :
                            1
                        )
                    ) 
                    :
                    1
                )
            );

    static constexpr unsigned out_align_len = (_CONFIG_T::output_length * _CONFIG_T::num_channels) / output_decoupling_factor * output_decoupling_factor;
    static constexpr unsigned out_left_len = (_CONFIG_T::output_length * _CONFIG_T::num_channels) % output_decoupling_factor;

    typedef ap_fixed<input_axi_width, input_axi_width - _CONFIG_T_input_fractional_bits> in_data_t;
    typedef ap_fixed<output_axi_width, output_axi_width - _CONFIG_T_output_fractional_bits>  out_data_t;
    typedef ap_uint<_CONFIG_T::config_width> config_t;
    typedef ap_fixed<coeff_axi_width, coeff_axi_width - _CONFIG_T_coeff_fractional_bits> coeff_t;

#ifndef AESL_SYN
    //// Define array helper functions for types used
    //DEFINE_XIP_ARRAY(real);
    //DEFINE_XIP_ARRAY(complex);
    //DEFINE_XIP_ARRAY(uint);
    //DEFINE_XIP_ARRAY(mpz);
    //DEFINE_XIP_ARRAY(mpz_complex);

    //DEFINE_FIR_XIP_ARRAY(real);
    //DEFINE_FIR_XIP_ARRAY(mpz);
    //DEFINE_FIR_XIP_ARRAY(mpz_complex);

    xip_fir_v7_2* mFIR;
#endif

#ifndef AESL_SYN
    void printConfig(const xip_fir_v7_2_config* cfg)
    {
        printf("Configuration of %s:\n",cfg->name);
        printf("\tFilter       : ");
        if (cfg->filter_type == hls::ip_fir::single_rate || 
            cfg->filter_type == hls::ip_fir::hilbert_filter ) {
          printf("%s\n",hls::ip_fir::fir_filter_type_str[cfg->filter_type]);
        } else if ( cfg->filter_type == hls::ip_fir::interpolation ) {
          printf("%s by %d\n",hls::ip_fir::fir_filter_type_str[cfg->filter_type],cfg->zero_pack_factor);
        } else {
          printf("%s up by %d down by %d\n",hls::ip_fir::fir_filter_type_str[cfg->filter_type],cfg->interp_rate,cfg->decim_rate);
        }
        printf("\tCoefficients : %d ",cfg->coeff_sets);
        if ( cfg->is_halfband ) {
          printf("Halfband ");
        }
        if (cfg->reloadable) {
          printf("Reloadable ");
        }
        printf("coefficient set(s) of %d taps\n",cfg->num_coeffs);
        printf("\tData         : %d path(s) of %d %s channel(s)\n",cfg->num_paths,cfg->num_channels,hls::ip_fir::fir_channel_sequence_str[cfg->chan_seq]);
    }

    void gen_ip_inst()
    {

        xip_fir_v7_2_config fir_cnfg, check_cnfg;
        fir_cnfg.name =  "fir_compiler";

        fir_cnfg.coeff = &_CONFIG_T::coeff_vec[0];
        fir_cnfg.filter_type = _CONFIG_T::filter_type;
        fir_cnfg.rate_change = _CONFIG_T::rate_change;
        fir_cnfg.interp_rate = _CONFIG_T::interp_rate;
        fir_cnfg.decim_rate = _CONFIG_T::decim_rate;
        fir_cnfg.zero_pack_factor = _CONFIG_T::zero_pack_factor;
        fir_cnfg.num_channels = _CONFIG_T::num_channels;
        fir_cnfg.coeff_sets = _CONFIG_T::coeff_sets;
        fir_cnfg.num_coeffs = _CONFIG_T::num_coeffs;
        fir_cnfg.reloadable = _CONFIG_T::reloadable;
        fir_cnfg.quantization = _CONFIG_T::quantization;
        fir_cnfg.coeff_width = _CONFIG_T::coeff_width;
        fir_cnfg.coeff_fract_width = _CONFIG_T_coeff_fractional_bits;
        fir_cnfg.chan_seq = _CONFIG_T::chan_seq;
        fir_cnfg.data_width = _CONFIG_T_input_width;
        fir_cnfg.data_fract_width = _CONFIG_T_input_fractional_bits;
        fir_cnfg.output_rounding_mode = _CONFIG_T::output_rounding_mode;
        fir_cnfg.output_width = _CONFIG_T::output_width; 
        fir_cnfg.output_fract_width = _CONFIG_T_output_fractional_bits;
        fir_cnfg.config_method = _CONFIG_T::config_method;
        fir_cnfg.coeff_padding = _CONFIG_T::coeff_padding;
        fir_cnfg.is_halfband = (_CONFIG_T::coeff_structure == ip_fir::half_band);

        //FIXME: doesn't support the following params
        fir_cnfg.init_pattern = P4_3;
        fir_cnfg.num_paths = 1; 

        //Create filter instances
        mFIR = xip_fir_v7_2_create(&fir_cnfg, &ip_fir::msg_print, 0);
        if (!mFIR) {
            printf("Error creating instance %s\n",fir_cnfg.name);
            exit(1);
        } 
        xip_fir_v7_2_get_config (mFIR, &check_cnfg);
        if (check_cnfg.output_fract_width != _CONFIG_T_output_fractional_bits) {
            printf("Specified output fractional width %d is different from the required value %d\n",_CONFIG_T_output_fractional_bits, check_cnfg.output_fract_width);
            exit(1);
        }

        #ifdef _HLSCLIB_DEBUG_
        printConfig(&fir_cnfg);
        #endif
    }
#endif

    void insert_fpga_ip() 
    {
#ifdef AESL_SYN
        const unsigned ssr_rate = (_CONFIG_T::sample_period >= 1) ? 1 : FLOOR(1./_CONFIG_T::sample_period);
static_assert((_CONFIG_T_input_length % ssr_rate == 0),
  "Error: in a FIR instance, floor(1/sample_period) must divide evenly input_length");
        #pragma HLS inline
            __fpga_ip("Vivado_FIR",
                //"component_name", "fir_compiler_0",
                "gui_behaviour", "Coregen",
                "coefficientsource", "Vector",
                "coefficientvector", _CONFIG_T::coeff_vec,
                "coefficient_file", "no_coe_file_loaded",
                "coefficient_sets", _CONFIG_T::coeff_sets,
                "coefficient_reload", _CONFIG_T::reloadable,
                "filter_type", _CONFIG_T::filter_type,
                "rate_change_type", _CONFIG_T::rate_change,
                "interpolation_rate", _CONFIG_T::interp_rate,
                "decimation_rate", _CONFIG_T::decim_rate,
                "zero_pack_factor", _CONFIG_T::zero_pack_factor,
                "channel_sequence", _CONFIG_T::chan_seq,
                "number_channels", _CONFIG_T::num_channels,
                "select_pattern", "All",
                "pattern_list", "P4-0,P4-1,P4-2,P4-3,P4-4",
                "number_paths", _CONFIG_T::num_paths,
                "ratespecification", _CONFIG_T::rate_specification,
                "sampleperiod", _CONFIG_T::sample_period, 
                "sample_frequency", _CONFIG_T::sample_frequency,
                "clock_frequency", "300.0",
                "coefficient_sign", _CONFIG_T::coeff_sign,
                "quantization", _CONFIG_T::quantization,
                "coefficient_width", _CONFIG_T::coeff_width,
                "bestprecision", _CONFIG_T::best_precision,
                "coefficient_fractional_bits", _CONFIG_T_coeff_fractional_bits,
                "coefficient_structure", _CONFIG_T::coeff_structure,
                "data_sign", _CONFIG_T::data_sign,
                "data_width", _CONFIG_T_input_width,
                "data_fractional_bits", _CONFIG_T_input_fractional_bits,
                "output_rounding_mode", _CONFIG_T::output_rounding_mode,
                "output_width", _CONFIG_T::output_width,
                "filter_architecture", _CONFIG_T::filter_arch,
                "optimization_goal", _CONFIG_T::optimization_goal,
                "optimization_selection", "None",
                "optimization_list", "None",
                "data_buffer_type", "Automatic",
                "coefficient_buffer_type", "Automatic",
                "input_buffer_type", "Automatic",
                "output_buffer_type", "Automatic",
                "preference_for_other_storage", "Automatic",
                "multi_column_support", "Automatic",
                "inter_column_pipe_length", _CONFIG_T::inter_column_pipe_length,
                "columnconfig", _CONFIG_T::column_config,
                "data_has_tlast", "Packet_Framing",
                "m_data_has_tready", "true",
                "s_data_has_fifo", "true",
                "s_data_has_tuser", "Not_Required",
                "m_data_has_tuser", "Not_Required",
                "data_tuser_width", "1",
                "s_config_sync_mode", _CONFIG_T::config_sync_mode,
                "s_config_method", _CONFIG_T::config_method,
                "num_reload_slots", "1",
                "has_aclken", "true",
                "has_aresetn", "true",
                "reset_data_vector", "true",
                "gen_mif_from_spec", "false",
                "gen_mif_from_coe", "false",
                "reload_file", "no_coe_file_loaded",
                "gen_mif_files", "false",
                "displayreloadorder", "false",
                "passband_min", "0.0",
                "passband_max", "0.5",
                "stopband_min", "0.5",
                "stopband_max", "1.0",
                "filter_selection", "1"
            );
#endif
    }


#ifndef AESL_SYN
    enum sim_mode_t {dataonly, configonly, reloadable};    

    void run_sim (
        in_data_t in[_CONFIG_T_input_length * _CONFIG_T::num_channels],
        out_data_t out[_CONFIG_T::output_length * _CONFIG_T::num_channels],
        config_t config[_CONFIG_T::num_channels],
        coeff_t reload[_CONFIG_T::num_coeffs + ((_CONFIG_T::coeff_sets == 1) ? 0 : 1)],
        sim_mode_t mode)
    {
        //////////////////////////////////////////////
        // C level simulation models for hls::fir
        //////////////////////////////////////////////
        // Create input data packet
        xip_array_real* din = xip_array_real_create();
        xip_array_real_reserve_dim(din,3);
        din->dim_size = 3; // 3D array
        din->dim[0] = 1;
        din->dim[1] = _CONFIG_T::num_channels;
        din->dim[2] = _CONFIG_T_input_length;
        din->data_size = din->dim[0] * din->dim[1] * din->dim[2];
        if (xip_array_real_reserve_data(din,din->data_size) != XIP_STATUS_OK) {
            printf("Unable to reserve data!\n");
            return;
        }
        
        // Create output data packet
        //  - Automatically sized using xip_fir_v7_2_calc_size
        xip_array_real* fir_out = xip_array_real_create();
        xip_array_real_reserve_dim(fir_out,3);
        fir_out->dim_size = 3; // 3D array

        if(xip_fir_v7_2_calc_size(mFIR,din,fir_out,0)== XIP_STATUS_OK) {
            if (xip_array_real_reserve_data(fir_out,fir_out->data_size) != XIP_STATUS_OK) {
                printf("Unable to reserve data!\n");
                return;
            }
        } else {
            printf("Unable to calculate output date size\n");
            return;
        }

        //FIXME: check and promote msg
        assert(fir_out->data_size == _CONFIG_T::output_length * _CONFIG_T::num_channels);

        // Populate data in with an impulse
        // FIXME: assume path=1 and chan = 1
        for (unsigned idx = 0; idx < _CONFIG_T_input_length; ++idx)
        {
            for (unsigned chan = 0; chan < _CONFIG_T::num_channels; ++chan)
                xip_fir_v7_2_xip_array_real_set_chan(din, in[idx * _CONFIG_T::num_channels + chan], 0, chan, idx, P_BASIC);
        }

        #ifdef _HLSCLIB_DEBUG_
        std::cout << "s_sata" << std::endl;
        for (int i=0; i< din->data_size; i++)
            std::cout << " " << din->data[i] ;
        std::cout << std::endl;
        #endif

        // send new configuration
        xip_array_uint* fsel = 0; 
        if ((mode == configonly) || (mode == reloadable))
        {
            assert(_CONFIG_T::coeff_sets > 1 || _CONFIG_T::reloadable);
            // Create config packet
            xip_array_uint* fsel = xip_array_uint_create();
            xip_array_uint_reserve_dim(fsel,1);
            fsel->dim_size = 1;
            fsel->dim[0] = _CONFIG_T::num_channels;
            fsel->data_size = fsel->dim[0];
            if (xip_array_uint_reserve_data(fsel,fsel->data_size) != XIP_STATUS_OK) {
                printf("Unable to reserve data!\n");
                return;
            }

            xip_fir_v7_2_cnfg_packet cnfg;
            cnfg.fsel = fsel;
            for (unsigned i = 0; i < cnfg.fsel->data_size; ++i)
                cnfg.fsel->data[i] = config[i].to_int();

            // Send config data
            if (xip_fir_v7_2_config_send(mFIR, &cnfg) != XIP_STATUS_OK) {
                printf("Error sending config packet\n");
                return;
            }

        #ifdef _HLSCLIB_DEBUG_
            std::cout << "Config packet: " ;
            for (int i = 0; i < cnfg.fsel->data_size; ++i)
                std::cout << " " << cnfg.fsel->data[i];
            std::cout << std::endl; 
        #endif
        }

        xip_fir_v7_2_rld_packet rld;
        // send reloaded coefficients
        if (mode == reloadable)
		{
            assert(_CONFIG_T::reloadable);
            if (_CONFIG_T::coeff_sets == 1)
                rld.fsel = 0;
            else
                rld.fsel = reload[0];
            rld.coeff = xip_array_real_create();
            xip_array_real_reserve_dim(rld.coeff,1);
            rld.coeff->dim_size=1;
            rld.coeff->dim[0]=_CONFIG_T::num_coeffs;
            rld.coeff->data_size = rld.coeff->dim[0];
            if (xip_array_real_reserve_data(rld.coeff,rld.coeff->data_size) != XIP_STATUS_OK) {
                printf("Unable to reserve coeff!\n");
                return;
            }

            // Copy coefficients into reload packet
            int coeff_i;
            bool isAllZero = true;
            int coeff_offset = (_CONFIG_T::coeff_sets == 1) ? 0 : 1;
            for (coeff_i= 0; coeff_i < _CONFIG_T::num_coeffs; coeff_i++) { 
                rld.coeff->data[coeff_i] = (xip_real)(reload[coeff_i + coeff_offset]); 
                isAllZero &= (reload[coeff_i] == 0);
            }

            // Send reload data
            if (!isAllZero) {
                if ( xip_fir_v7_2_reload_send(mFIR, &rld) != XIP_STATUS_OK) {
                    printf("Error sending reload packet\n");
                    return;
                }

			#ifdef _HLSCLIB_DEBUG_
                std::cout << "Reload packet: ";
                if (_CONFIG_T::coeff_sets > 1)
                    std::cout << "fsel = " << rld.fsel << "\t; new coeff : ";
                for (int i = 0; i < rld.coeff->data_size; ++i)
                    std::cout << " " << rld.coeff->data[i];
                std::cout << std::endl;
            #endif
            }
        }           
        
        // Send input data and filter
        if ( xip_fir_v7_2_data_send(mFIR,din)!= XIP_STATUS_OK) {
            printf("Error sending data\n");
            return;
        } 

        // Retrieve filtered data
        if (xip_fir_v7_2_data_get(mFIR,fir_out,0) != XIP_STATUS_OK) {
            printf("Error getting data\n");
            return;
        }

        // FIXME: assume path=1 and chan = 1
        for (unsigned idx = 0; idx < _CONFIG_T::output_length; ++idx)
        {
            for (unsigned chan = 0; chan < _CONFIG_T::num_channels; ++chan)
            {
                xip_real val;
                xip_fir_v7_2_xip_array_real_get_chan(fir_out, &val, 0, chan, idx, P_BASIC);
                out[idx * _CONFIG_T::num_channels+ chan] = (out_data_t)val; 
            }
        }

        //DEBUG
		#ifdef _HLSCLIB_DEBUG_
        std::cout << "m_sata" << std::endl;
        for (int i=0; i< fir_out->data_size; i++)
            std::cout << " " << fir_out->data[i] ;
        std::cout << std::endl;
        #endif

        xip_array_real_destroy(din);
        xip_array_real_destroy(fir_out);
        if (fsel) xip_array_uint_destroy(fsel);
        if (mode == reloadable) xip_array_real_destroy(rld.coeff);
    }
#endif

    bool isCoprime(unsigned a, unsigned b)
    {
        bool res = true;

        for(int i=0;i<TABLE_SIZE;i++)
        {
            if (b < hls::ip_fir::primeNums[i])
                break;
            else if (b % hls::ip_fir::primeNums[i] == 0 && a % hls::ip_fir::primeNums[i] == 0)
            {
                res = false;
                break;
            }
        }
        
        return res;
    }

    void validateFactor()
    {
        bool violate = false;
        // bitwidth checker
        //static_assert(_CONFIG_T_input_width % 8 == 0 && "Error: FIR input_width must be a signed multiple of 8 bits");
        //static_assert(_CONFIG_T::output_width % 8 == 0 && "Error: FIR output_width must be a signed multiple of 8 bits");
        
        // data length checker
        int diff = _CONFIG_T_input_length % ssr_rate;
        if (diff) 
        {
            printf("Error: in a FIR instance, SSR rate(=floor(1/sample_period)) must divide evenly input_length, but %d %% %d == %d. Synthesis will fail!\n", 
                _CONFIG_T_input_length, ssr_rate, diff);
            violate = true;
        }

        diff = _CONFIG_T::output_length % output_decoupling_factor;
        if (diff)
        {
            printf("Error: In a FIR instance, output_length must be divided evenly by output_decoupling_factor. But user set %d %% %d = %d. RTL execution will hang.\n",_CONFIG_T::output_length,output_decoupling_factor,diff);
            if (_CONFIG_T::filter_type == hls::ip_fir::single_rate)
                printf("    For single rate FIR, output_decoupling_factor = ssr_rate.\n");
            else if (_CONFIG_T::filter_type == hls::ip_fir::interpolation)
            {
                if (_CONFIG_T::rate_change == hls::ip_fir::integer)
                    printf("    For interpolation FIR, output_decoupling_factor = interp_rate * ssr_rate\n");
                else if (_CONFIG_T::rate_change == hls::ip_fir::fixed_fractional)
                    printf("    For interpolation FIR, output_decoupling_factor = ceil(interp_rate / decim_rate) * ssr_rate.\n");
            }
            else if (_CONFIG_T::filter_type == hls::ip_fir::decimation)
            {
                if (_CONFIG_T::rate_change == hls::ip_fir::integer)
                    printf("    For decimation FIR, output_decoupling_factor = ceil(ssr_rate / decim_rate).\n");
                else if (_CONFIG_T::rate_change == hls::ip_fir::fixed_fractional)
                    printf("    For decimation FIR, output_decoupling_factor = ceil(interp_rate / decim_rate).\n");
            }
            violate = true;
        }

        // param checker
        if (_CONFIG_T::filter_type == hls::ip_fir::single_rate)
        {
            if (ssr_rate > 256)
            {
                printf("Error! For single rate FIR, SSR rate set value '%d' is out of valid range [1,256].\n",ssr_rate);
                violate = true;
            }
            if (_CONFIG_T::interp_rate != 1)
            {
                printf("Error! For single rate FIR, interpolation rate must be 1. Set value '%d'.\n",_CONFIG_T::interp_rate);
                violate = true;
            }
            if (_CONFIG_T::decim_rate != 1)
            {
                printf("Error! For single rate FIR, decimation rate must be 1. Set value '%d'.\n",_CONFIG_T::decim_rate);
                violate = true;
            }
        }
        else if (_CONFIG_T::filter_type == hls::ip_fir::interpolation)
        {
            if (_CONFIG_T::rate_change == hls::ip_fir::integer)
            {
                if (ssr_rate > 256 / _CONFIG_T::interp_rate)
                {
                    printf("Error! For interpolation FIR using integer type rate, SSR rate set value '%d' is out of valid range [1,256/_CONFIG_T::interp_rate].\n",ssr_rate);
                    violate = true;
                }
                if (_CONFIG_T::interp_rate > 1024 || _CONFIG_T::interp_rate < 2)
                {
                    printf("Error! For interpolation FIR using integer type rate, interpolation rate set value '%d' is out of valid range [2,1024].\n",_CONFIG_T::interp_rate);
                    violate = true;
                }
                if (_CONFIG_T::decim_rate != 1)
                {
                    printf("Error! For interpolation FIR using integer type rate, decimation rate must be 1. Set value '%d'.\n",_CONFIG_T::decim_rate);
                    violate = true;
                }
            }
            else if (_CONFIG_T::rate_change == hls::ip_fir::fixed_fractional)
            {
                if (ssr_rate > 256 / _CONFIG_T::interp_rate)
                {
                    printf("Error! For interpolation FIR using fraction type rate, SSR rate set value '%d' is out of valid range [1,256/_CONFIG_T::interp_rate].\n",ssr_rate);
                    violate = true;
                }
                if (_CONFIG_T::interp_rate > 512 || _CONFIG_T::interp_rate < 3)
                {
                    printf("Error! For interpolation FIR using fraction type rate, interpolation rate set value '%d' is out of valid range [3,512].\n",_CONFIG_T::interp_rate);
                    violate = true;
                }
                if (_CONFIG_T::decim_rate >= _CONFIG_T::interp_rate || _CONFIG_T::decim_rate < 2)
                {
                    printf("Error! For interpolation FIR using franction type rate, decimation rate set value '%d' is out of valid range [2,_CONFIG_T::interp_rate).\n",_CONFIG_T::decim_rate);
                    violate = true;
                }
                if (!isCoprime(_CONFIG_T::interp_rate,_CONFIG_T::decim_rate))
                {
                    printf("Error! For interpolation FIR using franction type rate, decimation and interpolation rate must be coprime.Set value are '%d' & '%d'.\n",_CONFIG_T::decim_rate,_CONFIG_T::interp_rate);
                    violate = true;
                }
            }
            else
            {
                printf("Error! For interpolation FIR, interpolation rate type can be only 'integer' or 'fraction'.\n");
                violate = true;
            }
        }
        else if (_CONFIG_T::filter_type == hls::ip_fir::decimation)
        {
            if (_CONFIG_T::rate_change == hls::ip_fir::integer)
            {
                if (ssr_rate > 256 / _CONFIG_T::decim_rate)
                {
                    printf("Error! For decimation FIR using integer type rate, SSR rate set value '%d' is out of valid range [1,256/_CONFIG_T::decim_rate].\n",ssr_rate);
                    violate = true;
                }
                if (_CONFIG_T::decim_rate > 1024 || _CONFIG_T::decim_rate < 2)
                {
                    printf("Error! For decimation FIR using integer type rate, decimation rate set value '%d' is out of valid range [2,1024].\n",_CONFIG_T::decim_rate);
                    violate = true;
                }
                if (_CONFIG_T::interp_rate != 1)
                {
                    printf("Error! For decimation FIR using integer type rate, interpolation rate must be 1. Set value '%d'.\n",_CONFIG_T::interp_rate);
                    violate = true;
                }
            }
            // SSR mode for fraction rate decimation type FIR is not supported by vivado (till HLS Ver. 2025.2)
            else if (_CONFIG_T::rate_change == hls::ip_fir::fixed_fractional)
            {
                if (ssr_rate != 1)
                {
                    printf("Error! For decimation FIR using fraction type rate, SSR mode isn't supported. SSR rate must be 1. Set value '%d'.\n",ssr_rate);
                    violate = true;
                }
                if (_CONFIG_T::decim_rate > 512 || _CONFIG_T::decim_rate < 3)
                {
                    printf("Error! For decimation FIR using fraction type rate, decimation rate set value '%d' is out of valid range [3,512].\n",_CONFIG_T::decim_rate);
                    violate = true;
                }
                if (_CONFIG_T::decim_rate <= _CONFIG_T::interp_rate || _CONFIG_T::interp_rate < 2)
                {
                    printf("Error! For decimation FIR using franction type rate, interpolation rate set value '%d' is out of valid range [2,_CONFIG_T::decim_rate).\n",_CONFIG_T::interp_rate);
                    violate = true;
                }
                if (!isCoprime(_CONFIG_T::interp_rate,_CONFIG_T::decim_rate))
                {
                    printf("Error! For decimation FIR using franction type rate, decimation and interpolation rate must be coprime.Set value are '%d' & '%d'.\n",_CONFIG_T::decim_rate,_CONFIG_T::interp_rate);
                    violate = true;
                }
            }
            else
            {
                printf("Error! For decimation FIR, decimation rate type can be only 'integer' or 'fraction'.\n");
                violate = true;
            }
        }

        #ifdef _HLSCLIB_DEBUG_
        //input_grouping_factor = hls::ip_fir::calcInFactor(_CONFIG_T::sample_period,_CONFIG_T::filter_type);
        //output_decoupling_factor = hls::ip_fir::calcOutFactor(_CONFIG_T::sample_period,_CONFIG_T::filter_type,
        //                            _CONFIG_T::rate_change,_CONFIG_T::interp_rate,_CONFIG_T::decim_rate);
        printf("User set SSR rate is %d\n",ssr_rate);
        printf("calculated input grouping factor is %d\n",input_grouping_factor);
        printf("calculated output decoupling factor is %d\n",output_decoupling_factor);
        #endif

        if (violate)
            exit(1);
    }

    public:
        FIR()
#ifndef AESL_SYN
        : mFIR(0)
#endif
        {
#ifndef AESL_SYN
            validateFactor();
            gen_ip_inst();
#endif
        }

        ~FIR()
        {
            #ifdef AESL_SYN
            #pragma HLS inline 
            #else
            xip_fir_v7_2_destroy(mFIR);
            #endif
        }

        //////////////////////////////////////////////////////
        // APIs for FIR without config channel 
        //////////////////////////////////////////////////////
        void ssr_copy_fir (
            in_data_t in_V[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            in_data_t out_V[_CONFIG_T_input_length * _CONFIG_T::num_channels]
        )
        {
            #pragma HLS inline off 
            for (int i = 0; i < _CONFIG_T_input_length * _CONFIG_T::num_channels; ++i) {
                #pragma HLS pipeline II=1
                //#pragma HLS unroll factor=hls::ip_fir::calcInFactor(_CONFIG_T::sample_period,_CONFIG_T::filter_type)
                #pragma HLS unroll factor=input_grouping_factor
                out_V[i] = in_V[i];
            }
        }
        
        void ssr_run_fir(
            in_data_t in_V[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            out_data_t out_V[_CONFIG_T::output_length * _CONFIG_T::num_channels]
        )
        {
        #ifdef AESL_SYN
            #pragma HLS array_reshape cyclic variable=in_V factor=input_grouping_factor
            #pragma HLS array_reshape cyclic variable=out_V factor=output_decoupling_factor
            #pragma HLS stream variable=in_V
            #pragma HLS stream variable=out_V
            #pragma HLS inline off 
            insert_fpga_ip();
            //this function will be replaced by vivado IP.
            //following code is used to keep port from optimiztion.
            for (int i = 0; i < output_decoupling_factor; ++i) {
                #pragma HLS unroll 
                out_V[i] = in_V[0];
            }
        #else
            coeff_t reload_coeff[_CONFIG_T::num_coeffs];
            for (unsigned int i = 0; i < _CONFIG_T::num_coeffs; i++) 
                reload_coeff[i] = 0;
            config_t config[_CONFIG_T::num_channels] = {0};
            run_sim(in_V, out_V, config, reload_coeff, dataonly);
        #endif
        }

        void ssr_out_fir(
            out_data_t in_V[_CONFIG_T::output_length * _CONFIG_T::num_channels],
            out_data_t out_V[_CONFIG_T::output_length * _CONFIG_T::num_channels]
        )
        {
            #pragma HLS inline off 
            for (int i = 0; i < out_align_len; ++i) {
                //#pragma HLS unroll factor=hls::ip_fir::calcOutFactor(_CONFIG_T::sample_period,_CONFIG_T::filter_type,_CONFIG_T::rate_change,_CONFIG_T::interp_rate,_CONFIG_T::decim_rate)
                #pragma HLS unroll factor=output_decoupling_factor
                out_V[i] = in_V[i];
            }
            for (int i = 0; i < out_left_len; ++i)
            {
                #pragma HLS unroll
                out_V[i+out_align_len] = in_V[i+out_align_len];
            }
        }

        void run(
            in_data_t in_V[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            out_data_t out_V[_CONFIG_T::output_length * _CONFIG_T::num_channels]
        )
        {
        #ifdef AESL_SYN
            if constexpr (input_grouping_factor > 1 || output_decoupling_factor > 1)
            {
                in_data_t in_V1[_CONFIG_T_input_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                out_data_t out_V1[_CONFIG_T::output_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                #pragma HLS stream variable=in_V1 
                #pragma HLS stream variable=out_V1 
                #pragma HLS array_reshape cyclic variable=in_V1 factor=input_grouping_factor
                #pragma HLS array_reshape cyclic variable=out_V1 factor=output_decoupling_factor
                #pragma HLS dataflow
                ssr_copy_fir(in_V, in_V1);
                ssr_run_fir(in_V1,out_V1);
                ssr_out_fir(out_V1,out_V);
            }
            else
            {
                #pragma HLS inline off 
                #pragma HLS stream variable=in_V
                #pragma HLS stream variable=out_V
                insert_fpga_ip();
                for (int i = 0; i < _CONFIG_T::output_length; ++i) {
                    out_V[i] = in_V[i];
                }
            }
        #else
            coeff_t reload_coeff[_CONFIG_T::num_coeffs];
            for (unsigned int i = 0; i < _CONFIG_T::num_coeffs; i++) 
                reload_coeff[i] = 0;
            config_t config[_CONFIG_T::num_channels] = {0};
            run_sim(in_V, out_V, config, reload_coeff, dataonly);
        #endif
        }

        void ssr_copy_fir(
            hls::stream<in_data_t> &in_V,
            in_data_t out_V[_CONFIG_T_input_length * _CONFIG_T::num_channels]
        )
        {
            #pragma HLS inline off 
            for (int i = 0; i < _CONFIG_T_input_length * _CONFIG_T::num_channels; ++i) {
            //#pragma HLS unroll factor=hls::ip_fir::calcInFactor(_CONFIG_T::sample_period,_CONFIG_T::filter_type)
            #pragma HLS unroll factor=input_grouping_factor
                out_V[i] = in_V.read();
            }
        }

        void ssr_out_fir(
            out_data_t in_V[_CONFIG_T::output_length * _CONFIG_T::num_channels],
            hls::stream<out_data_t> &out_V
        )
        {
            #pragma HLS inline off 
            for (int i = 0; i < out_align_len; ++i) {
            //#pragma HLS unroll factor=hls::ip_fir::calcOutFactor(_CONFIG_T::sample_period,_CONFIG_T::filter_type,_CONFIG_T::rate_change,_CONFIG_T::interp_rate,_CONFIG_T::decim_rate)
            #pragma HLS unroll factor=output_decoupling_factor
                out_V.write(in_V[i]);
            }
            for (int i = 0; i < out_left_len; ++i)
            {
                #pragma HLS unroll
                out_V.write(in_V[i+out_align_len]);
            }
        }
        
        void run(
            hls::stream<in_data_t> &in_V,
            hls::stream<out_data_t> &out_V
        )
        {
        #ifdef AESL_SYN
            if constexpr (input_grouping_factor > 1 || output_decoupling_factor > 1)
            {
                in_data_t in_V1[_CONFIG_T_input_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                out_data_t out_V1[_CONFIG_T::output_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                #pragma HLS stream variable=in_V1
                #pragma HLS stream variable=out_V1
                #pragma HLS array_reshape cyclic variable=in_V1 factor=input_grouping_factor
                #pragma HLS array_reshape cyclic variable=out_V1 factor=output_decoupling_factor
                #pragma HLS dataflow
                ssr_copy_fir(in_V, in_V1);
                ssr_run_fir(in_V1,out_V1);
                ssr_out_fir(out_V1,out_V);
            }
            else
            {
                #pragma HLS inline off
                insert_fpga_ip();
                for (int i = 0; i < _CONFIG_T::output_length; ++i)
                    out_V.write(in_V.read());
            }
        #else
            in_data_t in_V1[_CONFIG_T_input_length * _CONFIG_T::num_channels];
            out_data_t out_V1[_CONFIG_T::output_length * _CONFIG_T::num_channels];
            coeff_t reload_coeff[_CONFIG_T::num_coeffs];
            config_t config[_CONFIG_T::num_channels] = {0};

            for (unsigned int i = 0; i < _CONFIG_T_input_length * _CONFIG_T::num_channels; i++) 
                in_V1[i] = in_V.read();
            for (unsigned int i = 0; i < _CONFIG_T::num_coeffs; i++) 
                reload_coeff[i] = 0;
            run_sim(in_V1, out_V1, config, reload_coeff, dataonly);
            for (unsigned int i = 0; i < _CONFIG_T::output_length * _CONFIG_T::num_channels; i++) 
                out_V.write(out_V1[i]);
        #endif
        }

        //////////////////////////////////////////////////////
        // APIs for FIR with config channel 
        // 'out_fir()' shares with without config version.
        //////////////////////////////////////////////////////
        void ssr_copy_fir(
            in_data_t in_V[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            in_data_t in_V1[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            config_t config_V[_CONFIG_T::num_channels],
            config_t config_V1[_CONFIG_T::num_channels])
        {
            #pragma HLS inline off 
            for (int j = 0; j < _CONFIG_T::num_channels; ++j) {
                #pragma HLS pipeline II=1
                config_V1[j] = config_V[j];
            }
#ifndef USE_FENCE
            ap_wait();
#else
            hls::fence({config_V1},{in_V1});
#endif

            for (int i = 0; i < _CONFIG_T::num_channels * _CONFIG_T_input_length; ++i) {
                #pragma HLS pipeline II=1
                //#pragma HLS unroll factor=hls::ip_fir::calcInFactor(_CONFIG_T::sample_period,_CONFIG_T::filter_type)
                #pragma HLS unroll factor=input_grouping_factor
                in_V1[i] = in_V[i];
            }
#ifndef USE_FENCE
            ap_wait();
#endif
        }

        void ssr_run_fir(
            in_data_t in_V[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            out_data_t out_V[_CONFIG_T::output_length * _CONFIG_T::num_channels],
            config_t config_V[_CONFIG_T::num_channels])
        {
        #ifdef AESL_SYN
            #pragma HLS inline off 
            #pragma HLS array_reshape cyclic variable=in_V factor=input_grouping_factor
            #pragma HLS array_reshape cyclic variable=out_V factor=output_decoupling_factor
            #pragma HLS stream variable=in_V
            #pragma HLS stream variable=out_V
            #pragma HLS stream variable=config_V

            insert_fpga_ip();
            // The code below is unused; it just prevents the FE from deleting the block's I/O
            bool dummy = false;
            for (int j = 0; j < _CONFIG_T::num_channels; ++j) 
            {
                dummy |= (config_V[j] != 0);
            }
            if (dummy)
            {
                for (int i = 0; i < output_decoupling_factor; ++i) 
                {
                    //#pragma HLS unroll factor=hls::ip_fir::calcOutFactor(_CONFIG_T::sample_period,_CONFIG_T::filter_type,_CONFIG_T::rate_change,_CONFIG_T::interp_rate,_CONFIG_T::decim_rate)
                    #pragma HLS unroll
                    out_V[i] = in_V[0];
                }
            }
        #else
            coeff_t reload_coeff[_CONFIG_T::num_coeffs];
            for (unsigned int i = 0; i < _CONFIG_T::num_coeffs; i++) 
                reload_coeff[i] = 0;
            run_sim(in_V, out_V, config_V, reload_coeff, configonly);
        #endif
        }

        void copy_fir(
            in_data_t in_V[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            in_data_t in_V1[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            config_t config_V[_CONFIG_T::num_channels],
            config_t config_V1[_CONFIG_T::num_channels])
        {
            #pragma HLS inline off 
            for (int j = 0; j < _CONFIG_T::num_channels; ++j) {
                #pragma HLS pipeline II=1
                config_V1[j] = config_V[j];
            }
            ap_wait();

            for (int i = 0; i < _CONFIG_T::num_channels * _CONFIG_T_input_length; ++i) {
                #pragma HLS pipeline II=1
                in_V1[i] = in_V[i];
            }
            ap_wait();
        }

        void run_fir(
            in_data_t in_V[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            out_data_t out_V[_CONFIG_T::output_length * _CONFIG_T::num_channels],
            config_t config_V[_CONFIG_T::num_channels])
        {
            #pragma HLS inline off 
            insert_fpga_ip();
            // The code below is unused; it just prevents the FE from deleting the block's I/O
            bool dummy = false;
            for (int j = 0; j < _CONFIG_T::num_channels; ++j) 
            {
                dummy |= (config_V[j] != 0);
            }
            if (dummy)
            {
                for (int i = 0; i < _CONFIG_T::num_channels * _CONFIG_T::output_length; ++i) 
                {
                    out_V[i] = in_V[i];
                }
            }
        }

        void run(
            in_data_t in_V[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            out_data_t out_V[_CONFIG_T::output_length * _CONFIG_T::num_channels],
            config_t config_V[_CONFIG_T::num_channels])
        {
        #ifdef AESL_SYN
            if constexpr (input_grouping_factor > 1 || output_decoupling_factor > 1)
            {
                config_t config_V1[_CONFIG_T::num_channels];
                #pragma HLS stream variable=config_V1

                in_data_t in_V1[_CONFIG_T_input_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                out_data_t out_V1[_CONFIG_T::output_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                #pragma HLS stream variable=in_V1
                #pragma HLS stream variable=out_V1
                #pragma HLS array_reshape cyclic variable=in_V1 factor=input_grouping_factor
                #pragma HLS array_reshape cyclic variable=out_V1 factor=output_decoupling_factor
                #pragma HLS dataflow
                ssr_copy_fir(in_V, in_V1, config_V, config_V1);
                ssr_run_fir(in_V1, out_V1, config_V1);
                ssr_out_fir(out_V1,out_V);
            }
            else
            {
                #pragma HLS stream variable=in_V
                #pragma HLS stream variable=out_V
                in_data_t in_V1[_CONFIG_T_input_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                #pragma HLS stream variable=in_V1
                config_t config_V1[_CONFIG_T::num_channels];
                #pragma HLS stream variable=config_V1

                #pragma HLS dataflow
                copy_fir(in_V, in_V1, config_V, config_V1);
                run_fir(in_V1, out_V, config_V1);
            }
        #else
            coeff_t reload_coeff[_CONFIG_T::num_coeffs];
            for (unsigned int i = 0; i < _CONFIG_T::num_coeffs; i++) 
                reload_coeff[i] = 0;
            run_sim(in_V, out_V, config_V, reload_coeff, configonly);
        #endif
        }

        void ssr_copy_fir(
            hls::stream<in_data_t> &in_V,
            in_data_t in_V1[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            hls::stream<config_t> &config_V,
            config_t config_V1[_CONFIG_T::num_channels])
        {
            #pragma HLS inline off 
            for (int j = 0; j < _CONFIG_T::num_channels; ++j) {
                #pragma HLS pipeline II=1
                config_V1[j] = config_V.read();
            }
#ifndef USE_FENCE
            ap_wait();
#else
            hls::fence({config_V1},{in_V1});
#endif

            for (int i = 0; i < _CONFIG_T::num_channels * _CONFIG_T_input_length; ++i) {
                #pragma HLS pipeline II=1
                //#pragma HLS unroll factor=hls::ip_fir::calcInFactor(_CONFIG_T::sample_period,_CONFIG_T::filter_type)
                #pragma HLS unroll factor=input_grouping_factor
                in_V1[i] = in_V.read();
            }
#ifndef USE_FENCE
            ap_wait();
#endif
        }

        void copy_fir(
            hls::stream<in_data_t> &in_V,
            in_data_t in_V1[_CONFIG_T::input_length * _CONFIG_T::num_channels],
            hls::stream<config_t> &config_V,
            config_t config_V1[_CONFIG_T::num_channels])
        {
            #pragma HLS inline off 
            for (int j = 0; j < _CONFIG_T::num_channels; ++j) {
                #pragma HLS pipeline II=1
                config_V1[j] = config_V.read();
            }
            ap_wait();

            for (int i = 0; i < _CONFIG_T::num_channels * _CONFIG_T::input_length; ++i) {
                #pragma HLS pipeline II=1
                in_V1[i] = in_V.read();
            }
            ap_wait();
        }

        void run(
            hls::stream<in_data_t> &in_V,
            hls::stream<out_data_t> &out_V,
            hls::stream<config_t> &config_V)
        {
        #ifdef AESL_SYN
            if constexpr (input_grouping_factor > 1 || output_decoupling_factor > 1)
            {
                config_t config_V1[_CONFIG_T::num_channels];
                #pragma HLS stream variable=config_V1

                in_data_t in_V1[_CONFIG_T_input_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                out_data_t out_V1[_CONFIG_T::output_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                #pragma HLS stream variable=in_V1
                #pragma HLS stream variable=out_V1
                #pragma HLS array_reshape cyclic variable=in_V1 factor=input_grouping_factor
                #pragma HLS array_reshape cyclic variable=out_V1 factor=output_decoupling_factor

                #pragma HLS dataflow
                ssr_copy_fir(in_V, in_V1, config_V, config_V1);
                ssr_run_fir(in_V1, out_V1, config_V1);
                ssr_out_fir(out_V1,out_V);
            }
            else
            {
                #pragma HLS inline off
                insert_fpga_ip();
                bool dummy = false;
                for (int j = 0; j < _CONFIG_T::num_channels; ++j) {
                    dummy |= (config_V.read() != 0);
                }
                if (dummy)
                {
                    for (int i = 0; i < _CONFIG_T::num_channels * _CONFIG_T::output_length; ++i) {
                        out_V.write(in_V.read());
                    }
                }
            }
        #else
            in_data_t in_V1[_CONFIG_T_input_length * _CONFIG_T::num_channels];
            out_data_t out_V1[_CONFIG_T::output_length * _CONFIG_T::num_channels];
            config_t config_V1[_CONFIG_T::num_channels];
            coeff_t reload_coeff[_CONFIG_T::num_coeffs];

            for (unsigned int i = 0; i < _CONFIG_T_input_length * _CONFIG_T::num_channels; i++) 
                in_V1[i] = in_V.read();
            for (unsigned int i = 0; i < _CONFIG_T::num_channels; i++) 
                config_V1[i] = config_V.read();
            for (unsigned int i = 0; i < _CONFIG_T::num_coeffs; i++) 
                reload_coeff[i] = 0;
            run_sim(in_V1, out_V1, config_V1, reload_coeff, configonly);
            for (unsigned int i = 0; i < _CONFIG_T::output_length * _CONFIG_T::num_channels; i++) 
                out_V.write(out_V1[i]);
        #endif
        }


        //////////////////////////////////////////////////////
        // APIs for FIR with config channel reloadable
        // 'out_fir()' shares with without config version.
        // 'copy_fir()' shares with with config channel version.
        //////////////////////////////////////////////////////
        void ssr_run_fir(
            in_data_t in_V[_CONFIG_T_input_length * _CONFIG_T::num_channels],
            out_data_t out_V[_CONFIG_T::output_length * _CONFIG_T::num_channels],
            config_t config_V[_CONFIG_T::num_channels],
            coeff_t reload_V[_CONFIG_T::num_coeffs + ((_CONFIG_T::coeff_sets == 1) ? 0 : 1)])
        {
        #ifdef AESL_SYN
            #pragma HLS array_reshape cyclic variable=in_V factor=input_grouping_factor
            #pragma HLS array_reshape cyclic variable=out_V factor=output_decoupling_factor
            #pragma HLS stream variable=in_V
            #pragma HLS stream variable=out_V
            #pragma HLS stream variable=config_V
            #pragma HLS inline off
            insert_fpga_ip();
            //this function will be replaced by vivado IP.
            //following code is used to keep port from optimiztion.
            if (*config_V)
                for (int i = 0; i < output_decoupling_factor; ++i)
                {
                    #pragma HLS unroll 
                    out_V[i] = in_V[0] + reload_V[i & _CONFIG_T::num_coeffs];
                }
        #else
            run_sim(in_V, out_V, config_V, reload_V, reloadable);
        #endif
        }

        void run_fir(
            in_data_t in_V[_CONFIG_T::input_length * _CONFIG_T::num_channels],
            out_data_t out_V[_CONFIG_T::output_length * _CONFIG_T::num_channels],
            config_t config_V[_CONFIG_T::num_channels],
            coeff_t reload_V[_CONFIG_T::num_coeffs + ((_CONFIG_T::coeff_sets == 1) ? 0 : 1)])
        {
            #pragma HLS inline off
            insert_fpga_ip();
            if (*config_V)
                for (int i = 0; i < _CONFIG_T::output_length * _CONFIG_T::num_channels; ++i)
                    out_V[i] = in_V[i] + reload_V[i & _CONFIG_T::num_coeffs];
        }

        void run(
            in_data_t in_V[_CONFIG_T::input_length * _CONFIG_T::num_channels],
            out_data_t out_V[_CONFIG_T::output_length * _CONFIG_T::num_channels],
            config_t config_V[_CONFIG_T::num_channels],
            coeff_t reload_V[_CONFIG_T::num_coeffs + ((_CONFIG_T::coeff_sets == 1) ? 0 : 1)])
        {
        #ifdef AESL_SYN
            if constexpr (input_grouping_factor > 1 || output_decoupling_factor > 1)
            {
                config_t config_V1[_CONFIG_T::num_channels];
                #pragma HLS stream variable=config_V1
                in_data_t in_V1[_CONFIG_T::input_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                out_data_t out_V1[_CONFIG_T::output_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                #pragma HLS stream variable=in_V1
                #pragma HLS stream variable=out_V1
                #pragma HLS array_reshape cyclic variable=in_V1 factor=input_grouping_factor
                #pragma HLS array_reshape cyclic variable=out_V1 factor=output_decoupling_factor

                #pragma HLS dataflow
                ssr_copy_fir(in_V, in_V1, config_V, config_V1);
                ssr_run_fir(in_V1, out_V1, config_V1, reload_V);
                ssr_out_fir(out_V1, out_V);
            }
            else
            {
                #pragma HLS stream variable=in_V
                #pragma HLS stream variable=out_V
                config_t config_V1[_CONFIG_T::num_channels];
                #pragma HLS stream variable=config_V1
                in_data_t in_V1[_CONFIG_T::input_length * _CONFIG_T::num_channels] __attribute__((no_ctor));
                #pragma HLS stream variable=in_V1

                #pragma HLS dataflow
                copy_fir(in_V, in_V1, config_V, config_V1);
                run_fir(in_V1, out_V, config_V1, reload_V);
            }
        #else
            run_sim(in_V, out_V, config_V, reload_V, reloadable);
        #endif
        }
};

} // namespace hls

#endif // __cplusplus
#endif // X_HLS_FIR_H


