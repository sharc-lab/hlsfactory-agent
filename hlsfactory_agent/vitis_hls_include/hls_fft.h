// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.

// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.

// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689

#ifndef X_HLS_FFT_H
#define X_HLS_FFT_H

/*
 * This file contains a C++ model of hls::fft.
 * It defines Vivado_HLS synthesis model.
 */
#ifndef __cplusplus
#error C++ is required to include this header file
#else

#include <ap_int.h> 
#include <complex>
#include <cassert>
#include <hls_stream.h> 
#include <hls_vector.h> 
#include <stdint.h> 
#ifndef __SYNTHESIS__
#include <math.h>
#endif

#ifndef AESL_SYN 
#include <iostream>
#include <fft/xfft_v9_1_bitacc_cmodel.h>
#endif

#ifndef __SYNTHESIS__
#include <iostream>
#include <fft/xfft_v9_1_bitacc_cmodel.h> 
#endif

#ifndef _RESHAPE_CALLER
#define _RESHAPE_CALLER 1
#endif

namespace hls {

#ifdef AESL_SYN 
#include <etc/autopilot_ssdm_op.h>
#endif

enum fft_T0_t { inlined };
enum fft_T1_t { streamed };
enum fft_T2_t { nonblocking };
enum fft_T3_t { wrapped };

namespace ip_fft {

#ifdef __SYNTHESIS__
#ifndef AP_INLINE
#define AP_INLINE inline __attribute__((always_inline))
#endif
#else
#ifndef AP_INLINE
#define AP_INLINE inline
#endif
#endif // __SYNTHESIS__

static const char* fftErrChkHead = "ERROR:hls::fft ";

enum ordering {bit_reversed_order = 0, natural_order};
enum scaling {scaled = 0, unscaled, block_floating_point};
enum arch {
	radix_4_burst_io = 0, radix_2_burst_io,
	pipelined_streaming_io, radix_2_lite_burst_io
};
enum rounding {truncation = 0, convergent_rounding};
enum mem { block_ram = 0, distributed_ram };
enum opt {
	use_luts = 0, use_mults_resources,
	use_mults_performance, use_xtremedsp_slices
};
enum type { fixed_point = 0, floating_point, native_floating_point };
static const char* fft_data_format_str[] = {"fixed_point", "floating_point", "native_floating_point"};
enum ssr {ssr_1=1, ssr_2=2, ssr_4=4, ssr_8=8, ssr_16=16, ssr_32=32, ssr_64=64};

#ifndef OVERRIDE
#define OVERRIDE(Param) static constexpr decltype(Default::Param) Param
#endif

template<bool _COND>
int constexpr my_assert() {
    static_assert(_COND, "Both old and new parameter name used for hls::ip_fft::params_t");
    return 0;
}
#ifdef __SYNTHESIS__
#define HLS_FFT_CHECK(new_name, old_name, def_val) \
    ((_CONFIG_T::new_name) != (def_val) ? (_CONFIG_T::new_name) : (_CONFIG_T::old_name))
#else
#define HLS_FFT_CHECK(new_name, old_name, def_val) \
    (hls::ip_fft::my_assert<(_CONFIG_T::new_name) == (def_val) || (_CONFIG_T::old_name) == (def_val)>, \
    ((_CONFIG_T::new_name) != (def_val) ? (_CONFIG_T::new_name) : (_CONFIG_T::old_name)))
#endif

// Redefinitions for new options
#define _CONFIG_T_max_nfft \
  HLS_FFT_CHECK(log2_transform_length, max_nfft, 10) 
#define _CONFIG_T_has_nfft \
  HLS_FFT_CHECK(run_time_configurable_transform_length, has_nfft,false)
#define _CONFIG_T_arch_opt \
  HLS_FFT_CHECK(implementation_options, arch_opt, ip_fft::pipelined_streaming_io)
#define _CONFIG_T_scaling_opt \
  HLS_FFT_CHECK(scaling_options, scaling_opt, ip_fft::scaled)
#define _CONFIG_T_rounding_opt \
  HLS_FFT_CHECK(rounding_modes, rounding_opt, ip_fft::truncation)
#define _CONFIG_T_ordering_opt \
  HLS_FFT_CHECK(output_ordering, ordering_opt, ip_fft::bit_reversed_order)
#define _CONFIG_T_mem_data \
  HLS_FFT_CHECK(memory_options_data, mem_data, ip_fft::block_ram)
#define _CONFIG_T_mem_phase_factors \
  HLS_FFT_CHECK(memory_options_phase_factors, mem_phase_factors, ip_fft::block_ram)
#define _CONFIG_T_mem_reorder \
  HLS_FFT_CHECK(memory_options_reorder, mem_reorder, ip_fft::block_ram)
#define _CONFIG_T_mem_hybrid \
  HLS_FFT_CHECK(memory_options_hybrid, mem_hybrid, false)
#define _CONFIG_T_super_sample_rate \
  HLS_FFT_CHECK(super_sample_rates, super_sample_rate, ip_fft::ssr_1)
#define _CONFIG_T_stages_block_ram \
  HLS_FFT_CHECK(number_of_stages_using_block_ram_for_data_and_phase_factors, stages_block_ram, ((_CONFIG_T::log2_transform_length < 10) ? 1 : (_CONFIG_T::log2_transform_length - 9)))

// Provides defaults for SSR==1, overridden by ssr_params_t for SSR>1
struct params_t
{
    using Default = params_t;

	static const unsigned input_width = 16;
	static const unsigned output_width = 16;
	static const unsigned status_width = 8;	
	static const unsigned config_width = 16;
	static const unsigned max_nfft = 10; // old
    static const unsigned log2_transform_length = 10; // new
	static const bool has_nfft = false; // old
	static const bool run_time_configurable_transform_length = false; // new
	static const unsigned channels = 1;
	static const unsigned arch_opt = pipelined_streaming_io; // old
	static const unsigned implementation_options = pipelined_streaming_io; // new
	static const unsigned phase_factor_width = 16;
	static const unsigned ordering_opt = bit_reversed_order; // old
	static const unsigned output_ordering = bit_reversed_order; // new
	static const bool ovflo = true;
	static const unsigned scaling_opt = scaled; // old
	static const unsigned scaling_options = scaled; // new
	static const unsigned rounding_opt = truncation; // old
	static const unsigned rounding_modes = truncation; // new
	static const unsigned mem_data = block_ram; // old
	static const unsigned memory_options_data = block_ram; // new
	static const unsigned mem_phase_factors = block_ram; // old
	static const unsigned memory_options_phase_factors = block_ram; // new
	static const unsigned mem_reorder = block_ram; // old
	static const unsigned memory_options_reorder = block_ram; // new
	static const unsigned stages_block_ram = ((max_nfft < 10) ? 1 : (max_nfft - 9)); // old
	static const unsigned number_of_stages_using_block_ram_for_data_and_phase_factors = ((log2_transform_length < 10) ? 1 : (log2_transform_length - 9)); // new
	static const bool mem_hybrid = false; // old
	static const bool memory_options_hybrid = false; // new
	static const unsigned complex_mult_type = use_mults_resources;
	static const unsigned butterfly_type = use_luts;
	static const unsigned super_sample_rate = ssr_1; // old
	static const unsigned super_sample_rates = ssr_1; // new
	static const bool use_native_float = false;
    static const bool systolicfft_inv = false; // Only for SSR>1

	//not supported params:
	static const bool xk_index = false;
	static const bool cyclic_prefix_insertion = false;
};

// Provides defaults for SSR>1
struct ssr_params_t: params_t
{
    using Default = params_t;

	static const unsigned phase_factor_width = 32;
	static const unsigned output_ordering = natural_order; // new version ONLY
	static const bool ovflo = false;
	static const unsigned complex_mult_type = use_mults_performance;
	static const unsigned butterfly_type = use_xtremedsp_slices;
};

template <typename _CONFIG_T>
struct config_t
{
	config_t() {
	}

	ap_uint<_CONFIG_T::config_width> data;
	// Check _CONFIG_T::config_width
	AP_INLINE void checkBitWidth(ip_fft::type data_type = ip_fft::fixed_point)
	{
#ifndef AESL_SYN
		const unsigned max_nfft = _CONFIG_T_max_nfft;
		const unsigned nfft_bits = _CONFIG_T_has_nfft ? 8 : 0; // Padding to 8 bits
		const unsigned cp_len_bits = _CONFIG_T::cyclic_prefix_insertion ? (((max_nfft + 7) >> 3) << 3) : 0; // Padding
		const unsigned ch_bits = _CONFIG_T::channels;
		const unsigned arch = _CONFIG_T_arch_opt;
		const unsigned tmp_bits = (arch == unsigned(ip_fft::pipelined_streaming_io) || arch == unsigned(ip_fft::radix_4_burst_io)) ? ((max_nfft+1)>>1) * 2 : 2 * max_nfft;
		//Temporarily set floating point type to always generate scaling due to bugs in FFT IP
		const bool need_scaling = (data_type == ip_fft::floating_point) ? true : (_CONFIG_T_scaling_opt == unsigned(ip_fft::scaled));
		const unsigned sch_bits = need_scaling ? tmp_bits : 0;
		const unsigned config_bits = (sch_bits + ch_bits) * _CONFIG_T::channels + cp_len_bits + nfft_bits;
		const unsigned config_width = ((config_bits + 7) >> 3) << 3; // padding
		if (_CONFIG_T::config_width != config_width)
		{
			std::cerr << ip_fft::fftErrChkHead << "Config channel width = " << (int)_CONFIG_T::config_width
					<< " is illegal." << std::endl;
			std::cerr << "Correct width is " << config_width << ". Please refer to FFT IP in Vivado GUI for details" << std::endl;
			exit(1);
		}
#endif
	}

	AP_INLINE void checkCpLen(bool cp_len_enable)
	{
#ifndef AESL_SYN
		if (cp_len_enable == 0)
		{
			std::cerr << fftErrChkHead << "FFT_CYCLIC_PREFIX_INSERTION = false."
					<< " It's invalid to access cp_len field."
					<< std::endl;
			exit(1);
		}
#endif
	}

	AP_INLINE void checkSch(unsigned scaling_opt)
	{
#ifndef AESL_SYN
		if (scaling_opt != unsigned(scaled))
		{
			std::cerr << fftErrChkHead << "FFT_SCALING != scaled."
					<< " It's invalid to access scaling_sch field."
					<< std::endl;
			exit(1);
		}
#endif
	}

	AP_INLINE void setNfft(unsigned nfft)
	{
		//checkBitWidth();
		if (_CONFIG_T_has_nfft) {
			data.range(7, 0) = nfft;
#ifndef AESL_SYN
			if (nfft > _CONFIG_T_max_nfft)
				std::cerr << "Warning: the nfft value is larger than max_nfft. The behavior of the FFT is undefined." << std::endl;
#endif
		} else if (nfft != _CONFIG_T_max_nfft) {
#ifndef AESL_SYN
			std::cerr << "Warning: As has_nnft is set to false, the nfft value cannot be changed. The max_nfft value is used in this case." << std::endl;
#endif
		}
	}
	AP_INLINE unsigned getNfft()
	{
		//checkBitWidth();
		if (_CONFIG_T_has_nfft)
			return data.range(7, 0);
		else
			return _CONFIG_T_max_nfft;
	}
	AP_INLINE unsigned getNfft() const
	{
		//checkBitWidth();
		if (_CONFIG_T_has_nfft)
			return data.range(7, 0);
		else
			return _CONFIG_T_max_nfft;
	}

	AP_INLINE void setCpLen(unsigned cp_len)
	{
		//checkBitWidth();
		checkCpLen(_CONFIG_T::cyclic_prefix_insertion);
		unsigned max_nfft = _CONFIG_T_max_nfft;
		unsigned nfft_bits = _CONFIG_T_has_nfft ? 8 : 0; // Padding to 8 bits
		unsigned cp_len_bits = _CONFIG_T::cyclic_prefix_insertion ? (((max_nfft + 7) >> 3) << 3) : 0; // Padding
		data.range(cp_len_bits+nfft_bits-1, nfft_bits) = cp_len;
	}
	AP_INLINE unsigned getCpLen()
	{
		//checkBitWidth();
		checkCpLen(_CONFIG_T::cyclic_prefix_insertion);
		unsigned ret = 0;
		unsigned max_nfft = _CONFIG_T_max_nfft;
		unsigned nfft_bits = _CONFIG_T_has_nfft ? 8 : 0; // Padding to 8 bits
		unsigned cp_len_bits = _CONFIG_T::cyclic_prefix_insertion ? (((max_nfft + 7) >> 3) << 3) : 0; // Padding
		ret = data.range(cp_len_bits+nfft_bits-1, nfft_bits);
		return ret;
	}
	AP_INLINE unsigned getCpLen() const
	{
		//checkBitWidth();
		checkCpLen(_CONFIG_T::cyclic_prefix_insertion);
		unsigned ret = 0;
		unsigned max_nfft = _CONFIG_T_max_nfft;
		unsigned nfft_bits = _CONFIG_T_has_nfft ? 8 : 0; // Padding to 8 bits
		unsigned cp_len_bits = _CONFIG_T::cyclic_prefix_insertion ? (((max_nfft + 7) >> 3) << 3) : 0; // Padding
		ret = data.range(cp_len_bits+nfft_bits-1, nfft_bits);
		return ret;
	}

	AP_INLINE void setDir(bool dir, unsigned ch = 0)
	{
		unsigned max_nfft = _CONFIG_T_max_nfft;
		unsigned nfft_bits = _CONFIG_T_has_nfft ? 8 : 0; // Padding to 8 bits
		unsigned cp_len_bits = _CONFIG_T::cyclic_prefix_insertion ? (((max_nfft + 7) >> 3) << 3) : 0; // Padding
		unsigned ch_lo = cp_len_bits + nfft_bits;
		unsigned ch_bits = 1;
		data.range(ch_bits*(ch+1)+ch_lo-1, ch_bits*ch+ch_lo) = dir;
	}
	AP_INLINE unsigned getDir(unsigned ch = 0)
	{
		unsigned max_nfft = _CONFIG_T_max_nfft;
		unsigned nfft_bits = _CONFIG_T_has_nfft ? 8 : 0; // Padding to 8 bits
		unsigned cp_len_bits = _CONFIG_T::cyclic_prefix_insertion ? (((max_nfft + 7) >> 3) << 3) : 0; // Padding
		unsigned ch_lo = cp_len_bits + nfft_bits;
		unsigned ch_bits = 1;
		return data.range(ch_bits*(ch+1)+ch_lo-1, ch_bits*ch+ch_lo);
	}
	AP_INLINE unsigned getDir(unsigned ch = 0) const
	{
		unsigned max_nfft = _CONFIG_T_max_nfft;
		unsigned nfft_bits = _CONFIG_T_has_nfft ? 8 : 0; // Padding to 8 bits
		unsigned cp_len_bits = _CONFIG_T::cyclic_prefix_insertion ? (((max_nfft + 7) >> 3) << 3) : 0; // Padding
		unsigned ch_lo = cp_len_bits + nfft_bits;
		unsigned ch_bits = 1;
		return data.range(ch_bits*(ch+1)+ch_lo-1, ch_bits*ch+ch_lo);
	}

	AP_INLINE void setSch(unsigned sch, unsigned ch = 0)
	{
		//checkBitWidth();
		checkSch(_CONFIG_T_scaling_opt);
		unsigned max_nfft = _CONFIG_T_max_nfft;
		unsigned nfft_bits = _CONFIG_T_has_nfft ? 8 : 0; // Padding to 8 bits
		unsigned cp_len_bits = _CONFIG_T::cyclic_prefix_insertion ? (((max_nfft + 7) >> 3) << 3) : 0; // Padding
		unsigned ch_lo = cp_len_bits + nfft_bits;
		unsigned ch_bits = 1;
		unsigned arch = _CONFIG_T_arch_opt;
		unsigned tmp_bits = (arch == unsigned(pipelined_streaming_io) || arch == unsigned(radix_4_burst_io)) ? ((max_nfft+1)>>1) * 2 : 2 * max_nfft;
		unsigned sch_bits = (_CONFIG_T_scaling_opt == unsigned(scaled)) ? tmp_bits : 0;
		unsigned sch_lo = ch_lo + _CONFIG_T::channels * ch_bits;
		data.range(sch_bits*(ch+1)+sch_lo-1, sch_bits*ch+sch_lo) = sch;
	}
	AP_INLINE unsigned getSch(unsigned ch = 0)
	{
		//checkBitWidth();
		checkSch(_CONFIG_T_scaling_opt);
		unsigned max_nfft = _CONFIG_T_max_nfft;
		unsigned nfft_bits = _CONFIG_T_has_nfft ? 8 : 0; // Padding to 8 bits
		unsigned cp_len_bits = _CONFIG_T::cyclic_prefix_insertion ? (((max_nfft + 7) >> 3) << 3) : 0; // Padding
		unsigned ch_lo = cp_len_bits + nfft_bits;
		unsigned ch_bits = 1;
		unsigned arch = _CONFIG_T_arch_opt;
		unsigned tmp_bits = (arch == unsigned(pipelined_streaming_io) || arch == unsigned(radix_4_burst_io)) ? ((max_nfft+1)>>1) * 2 : 2 * max_nfft;
		unsigned sch_bits = (_CONFIG_T_scaling_opt == unsigned(scaled)) ? tmp_bits : 0;
		unsigned sch_lo = ch_lo + _CONFIG_T::channels * ch_bits;
		return data.range(sch_bits*(ch+1)+sch_lo-1, sch_bits*ch+sch_lo);
	}
	AP_INLINE unsigned getSch(unsigned ch = 0) const
	{
		//checkBitWidth();
		checkSch(_CONFIG_T_scaling_opt);
		unsigned max_nfft = _CONFIG_T_max_nfft;
		unsigned nfft_bits = _CONFIG_T_has_nfft ? 8 : 0; // Padding to 8 bits
		unsigned cp_len_bits = _CONFIG_T::cyclic_prefix_insertion ? (((max_nfft + 7) >> 3) << 3) : 0; // Padding
		unsigned ch_lo = cp_len_bits + nfft_bits;
		unsigned ch_bits = 1;
		unsigned arch = _CONFIG_T_arch_opt;
		unsigned tmp_bits = (arch == unsigned(pipelined_streaming_io) || arch == unsigned(radix_4_burst_io)) ? ((max_nfft+1)>>1) * 2 : 2 * max_nfft;
		unsigned sch_bits = (_CONFIG_T_scaling_opt == unsigned(scaled)) ? tmp_bits : 0;
		unsigned sch_lo = ch_lo + _CONFIG_T::channels * ch_bits;
		return data.range(sch_bits*(ch+1)+sch_lo-1, sch_bits*ch+sch_lo);
	}
};

template<typename _CONFIG_T>
struct status_t
{
	typedef ap_uint<_CONFIG_T::status_width> status_data_t;
	status_data_t data;


	// Check _CONFIG_T::status_width
	AP_INLINE void checkBitWidth()
	{
#ifndef AESL_SYN
		const bool has_ovflo = _CONFIG_T::ovflo && (_CONFIG_T_scaling_opt == unsigned(ip_fft::scaled));
		const unsigned blk_exp_bits = (_CONFIG_T_scaling_opt == unsigned(ip_fft::block_floating_point)) ? 8 : 0; // padding to 8 bits
		const unsigned ovflo_bits = has_ovflo ? 1 : 0;
		const unsigned status_bits = (blk_exp_bits + ovflo_bits) * _CONFIG_T::channels;
		const unsigned status_width = (status_bits == 0) ? 8 : ((status_bits + 7) >> 3) << 3; // padding
		if (_CONFIG_T::status_width != status_width)
		{
			std::cerr << ip_fft::fftErrChkHead << "Status channel width = " << (int)_CONFIG_T::status_width
					<< " is illegal." << std::endl;
			exit(1);
		}
#endif
	}

	AP_INLINE void checkBlkExp(unsigned scaling_opt)
	{
#ifndef AESL_SYN
		if (scaling_opt != unsigned(block_floating_point))
		{
			std::cerr << fftErrChkHead << "FFT_SCALING != block_floating_point."
					<< " It's invalid to access BLK_EXP field."
					<< std::endl;
			exit(1);
		}
#endif
	}

	AP_INLINE void checkOvflo(bool has_ovflo)
	{
#ifndef AESL_SYN
		if (!has_ovflo)
		{
			std::cerr << fftErrChkHead
					<< "Current configuration disables over flow field,"
					<< " it's invalid to access OVFLO field."
					<< std::endl;
			exit(1);
		}
#endif
	}

	AP_INLINE void setBlkExp(status_data_t exp)
	{
		checkBitWidth();
		checkBlkExp(_CONFIG_T_scaling_opt);
		data = exp;
	}
	AP_INLINE unsigned getBlkExp(unsigned ch = 0)
	{
		checkBitWidth();
		unsigned blk_exp_bits = (_CONFIG_T_scaling_opt == unsigned(block_floating_point)) ? 8 : 0; // padding to 8 bits
		checkBlkExp(_CONFIG_T_scaling_opt);
		return data.range(blk_exp_bits*(ch+1)-1, blk_exp_bits*ch);
	}
	AP_INLINE unsigned getBlkExp(unsigned ch = 0) const
	{
		checkBitWidth();
		unsigned blk_exp_bits = (_CONFIG_T_scaling_opt == unsigned(block_floating_point)) ? 8 : 0; // padding to 8 bits
		checkBlkExp(_CONFIG_T_scaling_opt);
		return data.range(blk_exp_bits*(ch+1)-1, blk_exp_bits*ch);
	}

	AP_INLINE void setOvflo(status_data_t ovflo)
	{
		checkBitWidth();
		bool has_ovflo = _CONFIG_T::ovflo && (_CONFIG_T_scaling_opt == unsigned(scaled));
		checkOvflo(has_ovflo);
		data = ovflo;
	}
	AP_INLINE unsigned getOvflo(unsigned ch = 0)
	{
		checkBitWidth();
		bool has_ovflo = _CONFIG_T::ovflo && (_CONFIG_T_scaling_opt == unsigned(scaled));
		unsigned ovflo_bits = has_ovflo ? 1 : 0;
		checkOvflo(has_ovflo);
		return data.range(ovflo_bits*(ch+1)-1, ovflo_bits*ch);
	}
	AP_INLINE unsigned getOvflo(unsigned ch = 0) const
	{
		checkBitWidth();
		bool has_ovflo = _CONFIG_T::ovflo && (_CONFIG_T_scaling_opt == unsigned(scaled));
		unsigned ovflo_bits = has_ovflo ? 1 : 0;
		checkOvflo(has_ovflo);
		return data.range(ovflo_bits*(ch+1)-1, ovflo_bits*ch);
	}
};

} // namespace hls::ip_fft

template<
typename _CONFIG_T,
char _FFT_INPUT_WIDTH,
char _FFT_OUTPUT_WIDTH,
typename _FFT_INPUT_T,
typename _FFT_OUTPUT_T,
int _FFT_LENGTH,
char _FFT_CHANNELS,
ip_fft::type _FFT_DATA_FORMAT,
char _FFT_SUPER_SAMPLE_RATE
>
AP_INLINE void fft_core(
		std::complex<_FFT_INPUT_T> xn[_FFT_CHANNELS][_FFT_LENGTH],
		std::complex<_FFT_OUTPUT_T> xk[_FFT_CHANNELS][_FFT_LENGTH],
		ip_fft::status_t<_CONFIG_T>* status,
		ip_fft::config_t<_CONFIG_T>* config_ch)
{

#ifdef AESL_SYN

//////////////////////////////////////////////
// C level synthesis models for hls::fft
	//////////////////////////////////////////////

#pragma HLS inline
	__fpga_ip("Vivado_FFT",
			//"component_name", "xfft_0",
			"channels", _FFT_CHANNELS,
			"transform_length", 1 << _CONFIG_T_max_nfft,
			"implementation_options", _CONFIG_T_arch_opt,
			"run_time_configurable_transform_length", _CONFIG_T_has_nfft,
			"data_format", ip_fft::fft_data_format_str[_FFT_DATA_FORMAT],
			"input_width", _FFT_INPUT_WIDTH,
			"output_width", _FFT_OUTPUT_WIDTH,
			"phase_factor_width", _CONFIG_T::phase_factor_width,
			"scaling_options", _CONFIG_T_scaling_opt,
			"rounding_modes", _CONFIG_T_rounding_opt,
			"aclken", "true",
			"aresetn", "true",
			"ovflo", _CONFIG_T::ovflo,
			"xk_index", _CONFIG_T::xk_index,
			"throttle_scheme", "nonrealtime",
			"output_ordering", _CONFIG_T_ordering_opt,
			"cyclic_prefix_insertion", _CONFIG_T::cyclic_prefix_insertion,
			"memory_options_data", _CONFIG_T_mem_data,
			"memory_options_phase_factors", _CONFIG_T_mem_phase_factors,
			"memory_options_reorder", _CONFIG_T_mem_reorder,
			"number_of_stages_using_block_ram_for_data_and_phase_factors", _CONFIG_T_stages_block_ram,
			"memory_options_hybrid", _CONFIG_T_mem_hybrid,
			"complex_mult_type", _CONFIG_T::complex_mult_type,
			"butterfly_type", _CONFIG_T::butterfly_type,
			"systolicfft_inv", _CONFIG_T::systolicfft_inv,
			"super_sample_rates", _FFT_SUPER_SAMPLE_RATE
	);
	ip_fft::status_t<_CONFIG_T> status_t;
	ip_fft::config_t<_CONFIG_T> config_ch_t = *config_ch;

	bool has_scaling_sch =  config_ch_t.getSch();
	bool has_direction = config_ch_t.getDir();

	if ( has_direction || has_scaling_sch )
		for (int i = 0; i < _FFT_LENGTH; ++i)
		{
#pragma HLS unroll factor=_FFT_SUPER_SAMPLE_RATE skip_exit_check
			for (int c = 0; c < _FFT_CHANNELS; ++c)
			{
#pragma HLS unroll 
				xk[c][i] = xn[c][i];
			}
		}

	status_t.data = config_ch_t.getDir();
	*status = status_t;
#else

	//////////////////////////////////////////////
	// C level simulation models for hls::fft
	//////////////////////////////////////////////

	// Declare the C model IO structures
	xilinx_ip_xfft_v9_1_generics  generics;
	xilinx_ip_xfft_v9_1_state    *state;
	xilinx_ip_xfft_v9_1_inputs    inputs;
	xilinx_ip_xfft_v9_1_outputs   outputs;

	// Log2 of FFT length
	int fft_length = _FFT_LENGTH;
	int NFFT = 0;
	if (_CONFIG_T_has_nfft)
		NFFT = config_ch->getNfft();
	else
		NFFT = _CONFIG_T_max_nfft;

	const int samples =  1 << NFFT;

	///////////// IP parameters legality checking /////////////

	// Check _CONFIG_T::config_width
	config_ch->checkBitWidth(_FFT_DATA_FORMAT);

	// Check _CONFIG_T::status_width
	status->checkBitWidth();

	// Check ip parameters
	if (_CONFIG_T::channels < 1 || _CONFIG_T::channels > 12)
	{
		std::cerr << ip_fft::fftErrChkHead << "Channels = " << (int)_CONFIG_T::channels
				<< " is illegal. It should be from 1 to 12."
				<< std::endl;
		exit(1);
	}

	if (_CONFIG_T_max_nfft < 3 || _CONFIG_T_max_nfft > 16)
	{
		std::cerr << ip_fft::fftErrChkHead << "NFFT_MAX = " << (int)_CONFIG_T_max_nfft
				<< " is illegal. It should be from 3 to 16."
				<< std::endl;
		exit(1);
	}

	unsigned length = _FFT_LENGTH;
	if (!_CONFIG_T_has_nfft)
	{
		if (_FFT_LENGTH != (1 << _CONFIG_T_max_nfft))
		{
			std::cerr << ip_fft::fftErrChkHead << "_FFT_LENGTH = " << (int)_FFT_LENGTH
					<< " is illegal. Log2(_FFT_LENGTH) should equal to NFFT_MAX when run-time configurable length is disabled."
					<< std::endl;
			exit(1);
		}
	}
	else if (length & (length - 1))
	{
		std::cerr << ip_fft::fftErrChkHead << "_FFT_LENGTH = " << (int)_FFT_LENGTH
				<< " is illegal. It should be the integer power of 2."
				<< std::endl;
		exit(1);
	}
	else if (NFFT < 3 || NFFT > 16)
	{
		std::cerr << ip_fft::fftErrChkHead << "_FFT_LENGTH = " << (int)_FFT_LENGTH
				<< " is illegal. Log2(_FFT_LENGTH) should be from 3 to 16."
				<< std::endl;
		exit(1);
	}
	else if (NFFT > _CONFIG_T_max_nfft)
	{
		std::cerr << ip_fft::fftErrChkHead << "_FFT_LENGTH = " << (int)_FFT_LENGTH
				<< " is illegal. Log2(_FFT_LENGTH) should be less than or equal to NFFT_MAX."
				<< std::endl;
		exit(1);
	}
#if 0
	else if (NFFT != config_ch->getNfft())
	{
		std::cerr << ip_fft::fftErrChkHead << "_FFT_LENGTH = " << (int)_FFT_LENGTH
				<< " is illegal. Log2(_FFT_LENGTH) should equal to NFFT field of configure channel."
				<< std::endl;
		exit(1);
	}
#endif

	if ((_FFT_INPUT_WIDTH < 8) || (_FFT_INPUT_WIDTH > 40))
	{
		std::cerr << ip_fft::fftErrChkHead << "_FFT_INPUT_WIDTH = " << (int)_FFT_INPUT_WIDTH
				<< " is illegal. It should be 8,16,24,32,40."
				<< std::endl;
		exit(1);
	}

	if (_CONFIG_T_scaling_opt == ip_fft::unscaled && _FFT_DATA_FORMAT != ip_fft::floating_point)
	{
		unsigned golden = _FFT_INPUT_WIDTH + _CONFIG_T_max_nfft + 1;
		golden = ((golden + 7) >> 3) << 3;
		if (_FFT_OUTPUT_WIDTH != golden)
		{
			std::cerr << ip_fft::fftErrChkHead << "_FFT_OUTPUT_WIDTH = " << (int)_FFT_OUTPUT_WIDTH
					<< " is illegal with unscaled arithmetic. It should be input_width+nfft_max+1."
					<< std::endl;
			exit(1);
		}
	}
	else if (_FFT_OUTPUT_WIDTH != _FFT_INPUT_WIDTH)
	{
		std::cerr << ip_fft::fftErrChkHead << "_FFT_OUTPUT_WIDTH = " << (int)_FFT_OUTPUT_WIDTH
				<< " is illegal. It should be the same as input_width."
				<< std::endl;
		exit(1);
	}

	if (_CONFIG_T::channels > 1 && _CONFIG_T_arch_opt == ip_fft::pipelined_streaming_io)
	{
		std::cerr << ip_fft::fftErrChkHead << "_FFT_CHANNELS = " << (int)_CONFIG_T::channels << " and FFT_ARCH = pipelined_streaming_io"
				<< " is illegal. pipelined_streaming_io architecture is not supported when channels is bigger than 1."
				<< std::endl;
		exit(1);
	}

	if (_CONFIG_T::channels > 1 && _FFT_DATA_FORMAT == ip_fft::floating_point)
	{
		std::cerr << ip_fft::fftErrChkHead << "_FFT_CHANNELS = " << (int)_CONFIG_T::channels
				<< " is illegal with floating point data format. Floating point data format only supports 1 channel."
				<< std::endl;
		exit(1);
	}

	if(_FFT_DATA_FORMAT != ip_fft::floating_point){
		if(_CONFIG_T_super_sample_rate > ip_fft::ssr_4){	// SSR = 1/2/4 is available for both fixed point and native floating point
			std::cerr << ip_fft::fftErrChkHead << "FFT_SUPER_SAMPLE_RATE = " << (int)_CONFIG_T_super_sample_rate
					<< " is illegal with fixed point data format. Should use floating type."
					<< std::endl;
			exit(1);
		}
	}

	if (_FFT_DATA_FORMAT == ip_fft::floating_point)
	{
		if(!_CONFIG_T::use_native_float){
			if(_CONFIG_T_super_sample_rate > ip_fft::ssr_1){
				std::cerr << ip_fft::fftErrChkHead << "FFT_SUPER_SAMPLE_RATE = " << (int)_CONFIG_T_super_sample_rate
						<< " should be used with setting use_native_float=1."
						<< std::endl;
				exit(1);
			}			
			if (_CONFIG_T::phase_factor_width != 24 && _CONFIG_T::phase_factor_width != 25)
			{
				std::cerr << ip_fft::fftErrChkHead << "FFT_PHASE_FACTOR_WIDTH = " << (int)_CONFIG_T::phase_factor_width
						<< " is illegal with floating point data format. It should be 24 or 25."
						<< std::endl;
				exit(1);
			}
		}
		else {
			// native float specific checks
			if(_CONFIG_T_has_nfft != 0)
			{
				std::cerr << ip_fft::fftErrChkHead << "FFT_HAS_NFFT = " << (int)_CONFIG_T_has_nfft
						<< " is illegal with native floating point data format. It should be 0."
						<< std::endl;
				exit(1);
			}
			if(_CONFIG_T::phase_factor_width != 32)
			{
				std::cerr << ip_fft::fftErrChkHead << "FFT_PHASE_FACTOR_WIDTH = " << (int)_CONFIG_T::phase_factor_width
						<< " is illegal with native floating point data format. It should be 32."
						<< std::endl;
				exit(1);
			}
			if(_CONFIG_T_ordering_opt != ip_fft::natural_order)
			{
				std::cerr << ip_fft::fftErrChkHead << "FFT_ORDERING_OPT = bit_reversed_order" 
						<< " is illegal with native floating point data format. It should be natural_order."
						<< std::endl;
				exit(1);
			}
			if(_CONFIG_T::ovflo != false)
			{
				std::cerr << ip_fft::fftErrChkHead << "FFT_OVFLO = " << _CONFIG_T::ovflo
						<< " is illegal with native floating point data format. It should be false."
						<< std::endl;
				exit(1);
			}
			if(_CONFIG_T::complex_mult_type != ip_fft::use_mults_performance)
			{
				std::cerr << ip_fft::fftErrChkHead << "FFT_COMPLEX_MULT_TYPE"
						<< " is illegal with native floating point data format. It should be use_mults_performance."
						<< std::endl;
				exit(1);
			}
			if(_CONFIG_T::butterfly_type != ip_fft::use_xtremedsp_slices)
			{
				std::cerr << ip_fft::fftErrChkHead << "FFT_BUTTERFLY_TYPE"
						<< " is illegal with native floating point data format. It should be use_xtremedsp_slices."
						<< std::endl;
				exit(1);
			}
			if(_CONFIG_T_super_sample_rate == ip_fft::ssr_1)
			{
				std::cerr << ip_fft::fftErrChkHead << "FFT_SUPER_SAMPLE_RATE = 1"
						<< " is illegal with native floating point data format. It should be 2/4/8/16/32/64."
						<< std::endl;
				exit(1);
			}
			if(_FFT_LENGTH < 4*_CONFIG_T_super_sample_rate)
			{
				std::cerr << ip_fft::fftErrChkHead << "FFT_MAX_NFFT = " << _FFT_LENGTH 
						<< " is illegal with current super sample rate setting. It should be no less than four times super sample rate."
						<< std::endl;
				exit(1);
			}
		}
	}
	else if (_CONFIG_T::phase_factor_width < 8 || _CONFIG_T::phase_factor_width > 34)
	{
		std::cerr << ip_fft::fftErrChkHead << "FFT_PHASE_FACTOR_WIDTH = " << (int)_CONFIG_T::phase_factor_width
				<< " is illegal. It should be from 8 to 34."
				<< std::endl;
		exit(1);
	}

	//////////////////////////////////////////////////////////

	// Build up the C model generics structure
	generics.C_NFFT_MAX      = _CONFIG_T_max_nfft;
	generics.C_ARCH          = _CONFIG_T_arch_opt+1;
	generics.C_HAS_NFFT      = _CONFIG_T_has_nfft;
	generics.C_INPUT_WIDTH   = _FFT_INPUT_WIDTH;
	generics.C_TWIDDLE_WIDTH = _CONFIG_T::phase_factor_width;
	generics.C_HAS_SCALING   = _CONFIG_T_scaling_opt == ip_fft::unscaled ? 0 : 1;
	generics.C_HAS_BFP       = _CONFIG_T_scaling_opt == ip_fft::block_floating_point ? 1 : 0;
	generics.C_HAS_ROUNDING  = _CONFIG_T_rounding_opt;
	generics.C_USE_FLT_PT    = _FFT_DATA_FORMAT == ip_fft::floating_point ? (_CONFIG_T::use_native_float)? 2 : 1 : 0;
	generics.C_NSSR 		 = _FFT_SUPER_SAMPLE_RATE;
	generics.C_SYSTOLICFFT_INV = _CONFIG_T::systolicfft_inv;

	// Create an FFT state object
	state = xilinx_ip_xfft_v9_1_create_state(generics);

	int stages = 0;
	if(generics.C_NSSR == 1){
		if ((generics.C_ARCH == 2) || (generics.C_ARCH == 4))  // radix-2
			stages = NFFT;
		else  // radix-4 or radix-22
			stages = (NFFT+1)/2;
	} else {
		stages = NFFT;
	}

	double* xn_re       = (double*) calloc(samples, sizeof(double));
	double* xn_im       = (double*) calloc(samples, sizeof(double));
	int*    scaling_sch = (int*)    calloc(stages, sizeof(int));
	double* xk_re       = (double*) calloc(samples, sizeof(double));
	double* xk_im       = (double*) calloc(samples, sizeof(double));

	// Check the memory was allocated successfully for all arrays
	if (xn_re == NULL || xn_im == NULL || scaling_sch == NULL || xk_re == NULL || xk_im == NULL)
	{
		std::cerr << "Couldn't allocate memory for input and output data arrays - dying" << std::endl;
		exit(3);
	}

	ap_uint<_CONFIG_T::status_width> overflow = 0;
	ap_uint<_CONFIG_T::status_width> blkexp = 0;
	for (int c = 0; c < _FFT_CHANNELS; ++c)
	{
		// Set pointers in input and output structures
		inputs.xn_re       = xn_re;
		inputs.xn_im       = xn_im;
		inputs.scaling_sch = scaling_sch;
		outputs.xk_re      = xk_re;
		outputs.xk_im      = xk_im;

		// Store in inputs structure
		inputs.nfft = NFFT;
		// config data
		inputs.direction = config_ch->getDir(c);
		unsigned scaling = 0;
		if (_CONFIG_T_scaling_opt == ip_fft::scaled)
			scaling = config_ch->getSch(c);
		for (int i = 0; i < stages; i++)
		{
			inputs.scaling_sch[i] = scaling & 0x3;
			scaling >>= 2;
		}
		inputs.scaling_sch_size = stages;
		for (int i = 0; i < samples ; i++)
		{
			std::complex<_FFT_INPUT_T> din = xn[c][i];
			inputs.xn_re[i] = (double)din.real();
			inputs.xn_im[i] = (double)din.imag();
#ifdef _HLSCLIB_FFT_DEBUG_
std::cout << "xn[" << c << "][" << i << "]: xn_re = " << inputs .xn_re[i] <<
		" xn_im = " <<  inputs.xn_im[i] << std::endl;
#endif
		}
		inputs.xn_re_size = samples;
		inputs.xn_im_size = samples;

		// Set sizes of output structure arrays
		outputs.xk_re_size    = samples;
		outputs.xk_im_size    = samples;

//#define _HLSCLIB_FFT_DEBUG_
#ifdef _HLSCLIB_FFT_DEBUG_
		///////////////////////////////////////////////////////////////////////////////
		/// Debug
		std::cout << "About to call the C model with:" << std::endl;
		std::cout << "Generics:" << std::endl;
		std::cout << "  C_NFFT_MAX = "        << generics.C_NFFT_MAX << std::endl;
		std::cout << "  C_ARCH = "            << generics.C_ARCH << std::endl;
		std::cout << "  C_HAS_NFFT = "        << generics.C_HAS_NFFT << std::endl;
		std::cout << "  C_INPUT_WIDTH = "     << generics.C_INPUT_WIDTH << std::endl;
		std::cout << "  C_TWIDDLE_WIDTH = "   << generics.C_TWIDDLE_WIDTH << std::endl;
		std::cout << "  C_HAS_SCALING = "     << generics.C_HAS_SCALING << std::endl;
		std::cout << "  C_HAS_BFP = "         << generics.C_HAS_BFP << std::endl;
		std::cout << "  C_HAS_ROUNDING = "    << generics.C_HAS_ROUNDING << std::endl;
		std::cout << "  C_USE_FLT_PT = "      << generics.C_USE_FLT_PT << std::endl;
		std::cout << "  C_NSSR = "            << generics.C_NSSR << std::endl;
		std::cout << "  C_SYSTOLICFFT_INV = " << generics.C_SYSTOLICFFT_INV << std::endl;

		std::cout << "Inputs structure:" << std::endl;
		std::cout << "  nfft = " << inputs.nfft << std::endl;
		printf("  xn_re[0] = %e\n",inputs.xn_re[0]);
		std::cout << "  xn_re_size = " << inputs.xn_re_size << std::endl;
		printf("  xn_im[0] = %e\n",inputs.xn_im[0]);
		std::cout << "  xn_im_size = " << inputs.xn_im_size << std::endl;

		for (int i = stages - 1; i >= 0; --i)
			std::cout << "  scaling_sch[" << i << "] = " << inputs.scaling_sch[i] << std::endl;

		std::cout << "  scaling_sch_size = " << inputs.scaling_sch_size << std::endl;
		std::cout << "  direction = " << inputs.direction << std::endl;

		std::cout << "Outputs structure:" << std::endl;
		std::cout << "  xk_re_size = " << outputs.xk_re_size << std::endl;
		std::cout << "  xk_im_size = " << outputs.xk_im_size << std::endl;

		// Run the C model to generate output data
		std::cout << "Running the C model..." << std::endl;
		///////////////////////////////////////////////////////////////////////////////
#endif

		int result = 0;
		result = xilinx_ip_xfft_v9_1_bitacc_simulate(state, inputs, &outputs);
		if (result != 0)
		{
			std::cerr << "An error occurred when simulating the FFT core: return code " << result << std::endl;
			exit(4);
		}

		// Output data
		for (int i = 0; i < samples; i++)
		{
			std::complex<_FFT_OUTPUT_T> dout;
			unsigned addr_reverse = 0;
			for (int k = 0; k < NFFT; ++k)
			{
				addr_reverse <<= 1;
				addr_reverse |= (i >> k) & 0x1;
			}
			unsigned addr = i;
			if (_CONFIG_T_ordering_opt == ip_fft::bit_reversed_order)
				addr = addr_reverse;
			dout = std::complex<_FFT_OUTPUT_T> (outputs.xk_re[addr], outputs.xk_im[addr]);
			xk[c][i] = dout;
#ifdef _HLSCLIB_FFT_DEBUG_
std::cout << "xk[" << i << "][" << i << "]: xk_re = " << outputs.xk_re[addr] <<
		" xk_im = " <<  outputs.xk_im[addr] << std::endl;
#endif
		}

		// Status
		if (_CONFIG_T_scaling_opt == ip_fft::block_floating_point)
			blkexp.range(c*8+7, c*8) = outputs.blk_exp;
		else if (_CONFIG_T::ovflo && (_CONFIG_T_scaling_opt == ip_fft::scaled))
			overflow.range(c, c) = outputs.overflow;
	}

	// Status
	if (_CONFIG_T_scaling_opt == ip_fft::block_floating_point)
		status->setBlkExp(blkexp);
	else if (_CONFIG_T::ovflo && (_CONFIG_T_scaling_opt == ip_fft::scaled))
		status->setOvflo(overflow);

	// Release memory used for input and output arrays
	free(xn_re);
	free(xn_im);
	free(scaling_sch);
	free(xk_re);
	free(xk_im);

	// Destroy FFT state to free up memory
	xilinx_ip_xfft_v9_1_destroy_state(state);
#endif

} // End of fft_core


template<
typename _CONFIG_T,
char _FFT_INPUT_WIDTH,
char _FFT_OUTPUT_WIDTH,
typename _FFT_INPUT_T,
typename _FFT_OUTPUT_T,
int _FFT_LENGTH,
char _FFT_CHANNELS,
ip_fft::type _FFT_DATA_FORMAT,
char _FFT_SUPER_SAMPLE_RATE
>
void fft_core(
		std::complex<_FFT_INPUT_T> xn[_FFT_LENGTH],
		std::complex<_FFT_OUTPUT_T> xk[_FFT_LENGTH],
		ip_fft::status_t<_CONFIG_T>* status,
		ip_fft::config_t<_CONFIG_T>* config_ch)
{
#ifdef AESL_SYN
#pragma HLS inline

	__fpga_ip("Vivado_FFT",
			//"component_name", "xfft_0",
			"channels", _FFT_CHANNELS,
			"transform_length", _FFT_LENGTH,
			"implementation_options", _CONFIG_T_arch_opt,
			"run_time_configurable_transform_length", _CONFIG_T_has_nfft,
			"data_format", ip_fft::fft_data_format_str[_FFT_DATA_FORMAT],
			"input_width", _FFT_INPUT_WIDTH,
			"output_width", _FFT_OUTPUT_WIDTH,
			"phase_factor_width", _CONFIG_T::phase_factor_width,
			"scaling_options", _CONFIG_T_scaling_opt,
			"rounding_modes", _CONFIG_T_rounding_opt,
			"aclken", "true",
			"aresetn", "true",
			"ovflo", _CONFIG_T::ovflo,
			"xk_index", _CONFIG_T::xk_index,
			"throttle_scheme", "nonrealtime",
			"output_ordering", _CONFIG_T_ordering_opt,
			"cyclic_prefix_insertion", _CONFIG_T::cyclic_prefix_insertion,
			"memory_options_data", _CONFIG_T_mem_data,
			"memory_options_phase_factors", _CONFIG_T_mem_phase_factors,
			"memory_options_reorder", _CONFIG_T_mem_reorder,
			"number_of_stages_using_block_ram_for_data_and_phase_factors", _CONFIG_T_stages_block_ram,
			"memory_options_hybrid", _CONFIG_T_mem_hybrid,
			"complex_mult_type", _CONFIG_T::complex_mult_type,
			"butterfly_type", _CONFIG_T::butterfly_type,
			"systolicfft_inv", _CONFIG_T::systolicfft_inv,
			"super_sample_rates", _FFT_SUPER_SAMPLE_RATE
	);
	ip_fft::status_t<_CONFIG_T> status_t;
	ip_fft::config_t<_CONFIG_T> config_ch_t = *config_ch;

	bool has_scaling_sch =  config_ch_t.getSch();
	bool has_direction = config_ch_t.getDir();

	if ( has_direction || has_scaling_sch )
		for (int i = 0; i < _FFT_LENGTH; ++i)
		{
#pragma HLS unroll factor=_FFT_SUPER_SAMPLE_RATE skip_exit_check
			xk[i] = xn[i];
		}

	status_t.data = config_ch_t.getDir();
	*status = status_t;

#else
	std::complex<_FFT_INPUT_T> xn_multi_chan [1][_FFT_LENGTH];
	std::complex<_FFT_OUTPUT_T> xk_multi_chan [1][_FFT_LENGTH];

	for(int i=0; i< _FFT_LENGTH; i++)
		xn_multi_chan[0][i] = xn[i];

	fft_core<
	_CONFIG_T,
	_FFT_INPUT_WIDTH,
	_FFT_OUTPUT_WIDTH,
	_FFT_INPUT_T,
	_FFT_OUTPUT_T,
	_FFT_LENGTH,
	1,
	_FFT_DATA_FORMAT,
	_FFT_SUPER_SAMPLE_RATE
	>(xn_multi_chan, xk_multi_chan, status, config_ch);

	for(int i=0; i< _FFT_LENGTH; i++)
		xk[i] = xk_multi_chan[0][i];
#endif
}

#ifdef __SYNTHESIS__
// 1-channel, fixed-point, streaming
template<
typename _CONFIG_T
>
void fft_syn(
		hls::stream<std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > > &xn,
		hls::stream<std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > > &xk,
		hls::stream<ip_fft::status_t<_CONFIG_T> > &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T> > &config_ch_data_V)
{
#pragma HLS inline  

	__fpga_ip("Vivado_FFT",
			//"component_name", "xfft_0",
			"channels", 1,
			"transform_length", 1 << _CONFIG_T_max_nfft,
			"implementation_options", _CONFIG_T_arch_opt,
			"run_time_configurable_transform_length", _CONFIG_T_has_nfft,
			"data_format", ip_fft::fft_data_format_str[ip_fft::fixed_point],
			"input_width", _CONFIG_T::input_width,
			"output_width", _CONFIG_T::output_width,
			"phase_factor_width", _CONFIG_T::phase_factor_width,
			"scaling_options", _CONFIG_T_scaling_opt,
			"rounding_modes", _CONFIG_T_rounding_opt,
			"aclken", "true",
			"aresetn", "true",
			"ovflo", _CONFIG_T::ovflo,
			"xk_index", _CONFIG_T::xk_index,
			"throttle_scheme", "nonrealtime",
			"output_ordering", _CONFIG_T_ordering_opt,
			"cyclic_prefix_insertion", _CONFIG_T::cyclic_prefix_insertion,
			"memory_options_data", _CONFIG_T_mem_data,
			"memory_options_phase_factors", _CONFIG_T_mem_phase_factors,
			"memory_options_reorder", _CONFIG_T_mem_reorder,
			"number_of_stages_using_block_ram_for_data_and_phase_factors", _CONFIG_T_stages_block_ram,
			"memory_options_hybrid", _CONFIG_T_mem_hybrid,
			"complex_mult_type", _CONFIG_T::complex_mult_type,
			"butterfly_type", _CONFIG_T::butterfly_type,
			"systolicfft_inv", _CONFIG_T::systolicfft_inv,
			"super_sample_rates", _CONFIG_T_super_sample_rate
	);


	ip_fft::config_t<_CONFIG_T> config_tmp = config_ch_data_V.read();
	bool has_scaling_sch =  config_tmp.getSch();
	bool has_direction = config_tmp.getDir();
	int _FFT_LENGTH = 1 << _CONFIG_T_max_nfft;
	if ( has_direction || has_scaling_sch )
		for (int i = 0; i < _FFT_LENGTH; ++i)
		{
			xk.write(xn.read());
		}

	ip_fft::status_t<_CONFIG_T> status_tmp;
	status_tmp.data = config_tmp.getDir();
	status_data_V.write(status_tmp);

} // End of 1-channel, fixed-point, streaming

// 1-channel, fixed-point, streaming with SSR>1:
template<typename _CONFIG_T>
void fft_syn(
		hls::stream<hls::vector<std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> >, _CONFIG_T_super_sample_rate> > &xn,
		hls::stream<hls::vector<std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> >, _CONFIG_T_super_sample_rate> > &xk,
		hls::stream<ip_fft::status_t<_CONFIG_T> > &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T> > &config_ch_data_V)
{
#pragma HLS inline 
	__fpga_ip("Vivado_FFT",
			//"component_name", "xfft_0",
			"channels", 1,
			"transform_length", 1 << _CONFIG_T_max_nfft,
			"implementation_options", _CONFIG_T_arch_opt,
			"run_time_configurable_transform_length", _CONFIG_T_has_nfft,
			"data_format", ip_fft::fft_data_format_str[ip_fft::fixed_point],
			"input_width", _CONFIG_T::input_width,
			"output_width", _CONFIG_T::output_width,
			"phase_factor_width", _CONFIG_T::phase_factor_width,
			"scaling_options", _CONFIG_T_scaling_opt,
			"rounding_modes", _CONFIG_T_rounding_opt,
			"aclken", "true",
			"aresetn", "true",
			"ovflo", _CONFIG_T::ovflo,
			"xk_index", _CONFIG_T::xk_index,
			"throttle_scheme", "nonrealtime",
			"output_ordering", _CONFIG_T_ordering_opt,
			"cyclic_prefix_insertion", _CONFIG_T::cyclic_prefix_insertion,
			"memory_options_data", _CONFIG_T_mem_data,
			"memory_options_phase_factors", _CONFIG_T_mem_phase_factors,
			"memory_options_reorder", _CONFIG_T_mem_reorder,
			"number_of_stages_using_block_ram_for_data_and_phase_factors", _CONFIG_T_stages_block_ram,
			"memory_options_hybrid", _CONFIG_T_mem_hybrid,
			"complex_mult_type", _CONFIG_T::complex_mult_type,
			"butterfly_type", _CONFIG_T::butterfly_type,
			"systolicfft_inv", _CONFIG_T::systolicfft_inv,
			"super_sample_rates", _CONFIG_T_super_sample_rate 
	);

	ip_fft::config_t<_CONFIG_T> config_tmp = config_ch_data_V.read();
	bool has_scaling_sch =  config_tmp.getSch();
	bool has_direction = config_tmp.getDir();
	int _FFT_LENGTH = 1 << _CONFIG_T_max_nfft;
	if ( has_direction || has_scaling_sch )
		for (int i = 0; i < _FFT_LENGTH; ++i)
		{
			xk.write(xn.read());
		}
	ip_fft::status_t<_CONFIG_T> status_tmp;
	status_tmp.data = config_tmp.getDir();
	status_data_V.write(status_tmp);
}
// End of 1-channel, fixed-point, streaming with SSR>1

#endif

#ifdef __SYNTHESIS__
// 1-channel, fixed-point, partial streaming
template<
typename _CONFIG_T
>
void fft_syn(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::status_t<_CONFIG_T> > &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T> > &config_ch_data_V)
{
#pragma HLS inline off

	__fpga_ip("Vivado_FFT",
			//"component_name", "xfft_0",
			"channels", 1,
			"transform_length", 1 << _CONFIG_T_max_nfft,
			"implementation_options", _CONFIG_T_arch_opt,
			"run_time_configurable_transform_length", _CONFIG_T_has_nfft,
			"data_format", ip_fft::fft_data_format_str[ip_fft::fixed_point],
			"input_width", _CONFIG_T::input_width,
			"output_width", _CONFIG_T::output_width,
			"phase_factor_width", _CONFIG_T::phase_factor_width,
			"scaling_options", _CONFIG_T_scaling_opt,
			"rounding_modes", _CONFIG_T_rounding_opt,
			"aclken", "true",
			"aresetn", "true",
			"ovflo", _CONFIG_T::ovflo,
			"xk_index", _CONFIG_T::xk_index,
			"throttle_scheme", "nonrealtime",
			"output_ordering", _CONFIG_T_ordering_opt,
			"cyclic_prefix_insertion", _CONFIG_T::cyclic_prefix_insertion,
			"memory_options_data", _CONFIG_T_mem_data,
			"memory_options_phase_factors", _CONFIG_T_mem_phase_factors,
			"memory_options_reorder", _CONFIG_T_mem_reorder,
			"number_of_stages_using_block_ram_for_data_and_phase_factors", _CONFIG_T_stages_block_ram,
			"memory_options_hybrid", _CONFIG_T_mem_hybrid,
			"complex_mult_type", _CONFIG_T::complex_mult_type,
			"butterfly_type", _CONFIG_T::butterfly_type,
			"systolicfft_inv", _CONFIG_T::systolicfft_inv,
			"super_sample_rates", _CONFIG_T_super_sample_rate
	);


	ip_fft::config_t<_CONFIG_T> config_tmp = config_ch_data_V.read();
	bool has_scaling_sch =  config_tmp.getSch();
	bool has_direction = config_tmp.getDir();
	int _FFT_LENGTH = 1 << _CONFIG_T_max_nfft;
	if ( has_direction || has_scaling_sch )
		for (int i = 0; i < _FFT_LENGTH; ++i)
		{
#pragma HLS unroll factor=_CONFIG_T_super_sample_rate skip_exit_check
			xk[i] = xn[i];
		}

	ip_fft::status_t<_CONFIG_T> status_tmp;
	status_tmp.data = config_tmp.getDir();
	status_data_V.write(status_tmp);

} // End of 1-channel, fixed-point, partial streaming
#endif

#ifdef __SYNTHESIS__
// multi-channels, fixed-point, partial streaming
template<
typename _CONFIG_T
>
void fft_syn(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::status_t<_CONFIG_T> > &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T> > &config_ch_data_V)
{
#pragma HLS inline off

	__fpga_ip("Vivado_FFT",
			//"component_name", "xfft_0",
			"channels", _CONFIG_T::channels,
			"transform_length", 1 << _CONFIG_T_max_nfft,
			"implementation_options", _CONFIG_T_arch_opt,
			"run_time_configurable_transform_length", _CONFIG_T_has_nfft,
			"data_format", ip_fft::fft_data_format_str[ip_fft::fixed_point],
			"input_width", _CONFIG_T::input_width,
			"output_width", _CONFIG_T::output_width,
			"phase_factor_width", _CONFIG_T::phase_factor_width,
			"scaling_options", _CONFIG_T_scaling_opt,
			"rounding_modes", _CONFIG_T_rounding_opt,
			"aclken", "true",
			"aresetn", "true",
			"ovflo", _CONFIG_T::ovflo,
			"xk_index", _CONFIG_T::xk_index,
			"throttle_scheme", "nonrealtime",
			"output_ordering", _CONFIG_T_ordering_opt,
			"cyclic_prefix_insertion", _CONFIG_T::cyclic_prefix_insertion,
			"memory_options_data", _CONFIG_T_mem_data,
			"memory_options_phase_factors", _CONFIG_T_mem_phase_factors,
			"memory_options_reorder", _CONFIG_T_mem_reorder,
			"number_of_stages_using_block_ram_for_data_and_phase_factors", _CONFIG_T_stages_block_ram,
			"memory_options_hybrid", _CONFIG_T_mem_hybrid,
			"complex_mult_type", _CONFIG_T::complex_mult_type,
			"butterfly_type", _CONFIG_T::butterfly_type,
			"systolicfft_inv", _CONFIG_T::systolicfft_inv, 
			"super_sample_rates", _CONFIG_T_super_sample_rate
	);

	ip_fft::config_t<_CONFIG_T> config_tmp = config_ch_data_V.read();
	bool has_scaling_sch =  config_tmp.getSch();
	bool has_direction = config_tmp.getDir();
	int _FFT_LENGTH = 1 << _CONFIG_T_max_nfft;
	int _FFT_CHANNELS = _CONFIG_T::channels;
	if ( has_direction || has_scaling_sch )
		for (int i = 0; i < _FFT_LENGTH; ++i)
		{
#pragma HLS unroll factor=_CONFIG_T_super_sample_rate skip_exit_check
			for (int c = 0; c < _FFT_CHANNELS; ++c)
			{
#pragma HLS unroll 
				xk[c][i] = xn[c][i];
			}
		}

	ip_fft::status_t<_CONFIG_T> status_tmp;
	status_tmp.data = config_tmp.getDir();
	status_data_V.write(status_tmp);

} // End of multi-channels, fixed-point, partial streaming
#endif

// 1-channel, fixed-point
template<
typename _CONFIG_T
>
void fft_sim(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T> *status,
		ip_fft::config_t<_CONFIG_T> *config_ch)
{
	fft_core<
	_CONFIG_T,
	_CONFIG_T::input_width,
	_CONFIG_T::output_width,
	ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1>,
	ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1>,
	1 << _CONFIG_T_max_nfft,
	1,
	ip_fft::fixed_point,
	_CONFIG_T_super_sample_rate
	>(xn, xk, status, config_ch);
} // End of 1-channel, fixed-point

template<
typename _CONFIG_T
>
void data_copy_from_ap_fix_to_ap_uint(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[1 << _CONFIG_T_max_nfft],
		ap_uint<((_CONFIG_T::input_width+7)/8)*8*2> xn_cp[1 << _CONFIG_T_max_nfft]) {

	for (unsigned i = 0; i < (1 << _CONFIG_T_max_nfft); i++) {
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn_tmp = xn[i];
		ap_uint<((_CONFIG_T::input_width+7)/8)*8*2> xn_cp_tmp;
		xn_cp_tmp(((_CONFIG_T::input_width+7)/8)*8 - 1, 0) = xn_tmp.real().range(((_CONFIG_T::input_width+7)/8)*8 - 1, 0);
		xn_cp_tmp(((_CONFIG_T::input_width+7)/8)*8*2 - 1, ((_CONFIG_T::input_width+7)/8)*8) = xn_tmp.imag().range(((_CONFIG_T::input_width+7)/8)*8 - 1, 0);
		xn_cp[i] = xn_cp_tmp;
	}
}

template<
typename _CONFIG_T
>
void data_copy_from_ap_uint_to_ap_fixed(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xk[1 << _CONFIG_T_max_nfft],
		ap_uint<((_CONFIG_T::input_width+7)/8)*8*2> xk_cp[1 << _CONFIG_T_max_nfft]) {
	for (unsigned i = 0; i < (1 << _CONFIG_T_max_nfft); i++) {
		ap_uint<((_CONFIG_T::input_width+7)/8)*8*2> xk_cp_tmp = xk_cp[i];
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, 1> > xk_tmp;
		ap_fixed<((_CONFIG_T::output_width+7)/8)*8, 1> tmp;
		tmp.range(((_CONFIG_T::output_width+7)/8)*8 - 1, 0) = xk_cp_tmp.range(((_CONFIG_T::output_width+7)/8)*8 - 1, 0);
		xk_tmp.real(tmp);
		tmp.range(((_CONFIG_T::output_width+7)/8)*8 - 1, 0) = xk_cp_tmp.range(((_CONFIG_T::output_width+7)/8)*8*2 - 1, ((_CONFIG_T::output_width+7)/8)*8);
		xk_tmp.imag(tmp);
		xk[i] = xk_tmp;
	}
}

// 1-channel, fixed-point, streaming, not inlined
template<
typename _CONFIG_T
>
void  fft(
		hls::stream<hls::vector<std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> >,_CONFIG_T_super_sample_rate> > &xn,
		hls::stream<hls::vector<std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> >, _CONFIG_T_super_sample_rate> > &xk,
		hls::stream<ip_fft::status_t<_CONFIG_T> > &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T> > &config_ch_data_V)
{
#ifdef __SYNTHESIS__
#pragma HLS inline off

	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // inlined
#else
	std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn_a[1 << _CONFIG_T_max_nfft];
	std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk_a[1 << _CONFIG_T_max_nfft];
	ip_fft::status_t<_CONFIG_T> status;
	ip_fft::config_t<_CONFIG_T> config;

	config_ch_data_V.read(config);
	unsigned int bound = _CONFIG_T_has_nfft ? config.getNfft() : _CONFIG_T_max_nfft;
	for (unsigned int i = 0; i < 1 << bound; i += _CONFIG_T_super_sample_rate) {
        hls::vector<std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> >,_CONFIG_T_super_sample_rate> in = xn.read();
        for (unsigned int j = 0; j < _CONFIG_T_super_sample_rate; j++) {
            xn_a[i + j] = in[j];
        }
    }
	fft_sim<_CONFIG_T>(xn_a, xk_a, &status, &config);
	for (unsigned int i = 0; i < 1 << bound; i += _CONFIG_T_super_sample_rate) {
        hls::vector<std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> >, _CONFIG_T_super_sample_rate> out;
        for (unsigned int j = 0; j < _CONFIG_T_super_sample_rate; j++) {
            out[j] = xk_a[i + j];
        }
		xk.write(out);
    }
	status_data_V.write(status);
#endif
}

// 1-channel, fixed-point, streaming, not inlined
template<
typename _CONFIG_T
>
void  fft(
		hls::stream<std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > > &xn,
		hls::stream<std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > > &xk,
		hls::stream<ip_fft::status_t<_CONFIG_T> > &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T> > &config_ch_data_V)
{
#ifdef __SYNTHESIS__
#pragma HLS inline off

	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // inlined
#else
	std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn_a[1 << _CONFIG_T_max_nfft];
	std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk_a[1 << _CONFIG_T_max_nfft];
	ip_fft::status_t<_CONFIG_T> status;
	ip_fft::config_t<_CONFIG_T> config;

	config_ch_data_V.read(config);
	unsigned int bound = _CONFIG_T_has_nfft ? config.getNfft() : _CONFIG_T_max_nfft;
	for (unsigned int i = 0; i < 1 << bound; i++)
		xn_a[i] = xn.read();
	fft_sim<_CONFIG_T>(xn_a, xk_a, &status, &config);
	for (unsigned int i = 0; i < 1 << bound; i++)
		xk.write(xk_a[i]);
	status_data_V.write(status);
#endif
}

// 1-channel, fixed-point, streaming, non-blocking for simulation
// FIXME Add SSSR version
template<
typename _CONFIG_T, fft_T2_t _TAG
>
void  fft(
		hls::stream<std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > > &xn,
		hls::stream<std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > > &xk,
		hls::stream<ip_fft::status_t<_CONFIG_T> > &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T> > &config_ch_data_V)
{
#ifdef __SYNTHESIS__
#pragma HLS inline off

	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // inlined
#else
	std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn_a[1 << _CONFIG_T_max_nfft];
	std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk_a[1 << _CONFIG_T_max_nfft];
	ip_fft::status_t<_CONFIG_T> status;
	static ip_fft::config_t<_CONFIG_T> config;

	config_ch_data_V.read_nb(config);
	unsigned int bound = _CONFIG_T_has_nfft ? config.getNfft() : _CONFIG_T_max_nfft;
	for (unsigned int i = 0; i < 1 << bound; i++)
		xn_a[i] = xn.read();
	fft_sim<_CONFIG_T>(xn_a, xk_a, &status, &config);
	for (unsigned int i = 0; i < 1 << bound; i++)
		xk.write(xk_a[i]);
	status_data_V.write(status);
#endif
}

// 1-channel, fixed-point, arrays + streams
template<
typename _CONFIG_T
>
void fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V)
{
#ifdef __SYNTHESIS__
#pragma HLS inline
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // not inlined
#else
	ip_fft::status_t<_CONFIG_T> status;
	ip_fft::config_t<_CONFIG_T> config;

	config_ch_data_V.read(config);
	fft_sim<_CONFIG_T>(xn, xk, &status, &config);
	status_data_V.write(status);
#endif
}

// 1-channel, fixed-point, arrays + streams, non blocking for simulation
template<
typename _CONFIG_T, fft_T2_t _TAG
>
void fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V)
{
#ifdef __SYNTHESIS__
#pragma HLS inline
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // not inlined
#else
	ip_fft::status_t<_CONFIG_T> status;
	static ip_fft::config_t<_CONFIG_T> config;

	config_ch_data_V.read_nb(config);
	fft_sim<_CONFIG_T>(xn, xk, &status, &config);
	status_data_V.write(status);
#endif
}


// 1-channel, fixed-point, arrays + scalars, inlined
template<
typename _CONFIG_T, fft_T0_t _TAG
>
void fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T> *status,
		ip_fft::config_t<_CONFIG_T> *config_ch)
{
#ifdef __SYNTHESIS__
#pragma HLS inline
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn 
#pragma HLS stream variable=xk 
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
	hls::stream<ip_fft::config_t<_CONFIG_T>, 2> config_ch_data_V;
	hls::stream<ip_fft::status_t<_CONFIG_T>, 2> status_data_V;
	config_ch_data_V.write(*config_ch);
	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // not inlined
	*status = status_data_V.read();
#else
	fft_sim<_CONFIG_T>(xn, xk, status, config_ch);
#endif
}


// 1-channel, fixed-point, arrays + scalars, not inlined
template<
typename _CONFIG_T
>
void fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T> *status,
		ip_fft::config_t<_CONFIG_T> *config_ch)
{
#ifdef __SYNTHESIS__
#pragma HLS dataflow
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
	hls::stream<ip_fft::config_t<_CONFIG_T>, 2> config_ch_data_V;
	hls::stream<ip_fft::status_t<_CONFIG_T>, 2> status_data_V;
	config_ch_data_V.write(*config_ch);
	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // not inlined
	*status = status_data_V.read();
#else
	fft_sim<_CONFIG_T>(xn, xk, status, config_ch);
#endif
}

#ifdef __SYNTHESIS__

template<
typename _CONFIG_T
>
void fft_copy(hls::stream<ip_fft::status_t<_CONFIG_T> > &status_data_V, ip_fft::status_t<_CONFIG_T> *status, bool &flag) {

	if (flag)		    *status = status_data_V.read();
}

template<
typename _CONFIG_T
>
void fft_wrapper(std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::config_t<_CONFIG_T> > &config_ch_data_V,
		hls::stream<ip_fft::status_t<_CONFIG_T> > &status_data_V, bool &flag) {
#pragma HLS dataflow
        fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // not inlined
        flag = true;

}

#endif

// 1-channel, fixed-point, arrays + scalars, with extra wrapper 
template<
typename _CONFIG_T, fft_T3_t _TAG
>
void fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T> *status,
		ip_fft::config_t<_CONFIG_T> *config_ch)
{
#ifdef __SYNTHESIS__
#pragma HLS dataflow
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
	hls::stream<ip_fft::status_t<_CONFIG_T>, 2> status_data_V;
	hls::stream<ip_fft::config_t<_CONFIG_T>, 2> config_ch_data_V;
	bool flag;
#pragma HLS STREAM variable=flag type=pipo
	config_ch_data_V.write(*config_ch);
	fft_wrapper<_CONFIG_T>(xn, xk, config_ch_data_V, status_data_V, flag); // not inlined
	fft_copy(status_data_V, status, flag);
#else
	fft_sim<_CONFIG_T>(xn, xk, status, config_ch);
#endif
}

// 1-channel, fixed-point, 1D arrays + scalar-to-stream, not inlined
template<
typename _CONFIG_T, fft_T1_t _TAG
>
void       fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T> *status_data_V,
		ip_fft::config_t<_CONFIG_T> *config_ch_data_V)
{

#ifdef __SYNTHESIS__
#pragma HLS inline off
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS stream variable=status_data_V
#pragma HLS stream variable=config_ch_data_V
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
	fft_core<
	_CONFIG_T,
	_CONFIG_T::input_width,
	_CONFIG_T::output_width,
	ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1>,
	ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1>,
	1 << _CONFIG_T_max_nfft,
	1,
	ip_fft::fixed_point,
	_CONFIG_T_super_sample_rate>(xn, xk, status_data_V, config_ch_data_V); // inlined
#else
	fft_sim<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V);
#endif
} // End of 1-channel, fixed-point

// Multi-channels, fixed-point, 2D arrays + streams
template<
typename _CONFIG_T
>
void   fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8,
		((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V)
{
#ifdef __SYNTHESIS__
#pragma HLS inline
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS array_reshape dim=1 variable=xn
#pragma HLS array_reshape dim=1 variable=xk	
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape dim=2 cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape dim=2 cyclic variable=xk factor=_CONFIG_T_super_sample_rate
	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // not inlined
#else
	ip_fft::status_t<_CONFIG_T> status;
	ip_fft::config_t<_CONFIG_T> config;

	config_ch_data_V.read(config);

	fft_core<
	_CONFIG_T,
	_CONFIG_T::input_width,
	_CONFIG_T::output_width,
	ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1>,
	ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1>,
	1 << _CONFIG_T_max_nfft,
	_CONFIG_T::channels,
	ip_fft::fixed_point,
	_CONFIG_T_super_sample_rate
	>(xn, xk, &status, &config);
	status_data_V.write(status);

#endif
} 

// Multi-channels, fixed-point, 2D arrays + streams, non-blocking for simulation
template<
typename _CONFIG_T, fft_T2_t _TAG
>
void   fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8,
		((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V)
{
#ifdef __SYNTHESIS__
#pragma HLS inline
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS array_reshape dim=1 variable=xn
#pragma HLS array_reshape dim=1 variable=xk	
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=xk factor=_CONFIG_T_super_sample_rate
	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // not inlined
#else
	ip_fft::status_t<_CONFIG_T> status;
	static ip_fft::config_t<_CONFIG_T> config;

	config_ch_data_V.read_nb(config);

	fft_core<
	_CONFIG_T,
	_CONFIG_T::input_width,
	_CONFIG_T::output_width,
	ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1>,
	ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1>,
	1 << _CONFIG_T_max_nfft,
	_CONFIG_T::channels,
	ip_fft::fixed_point,
	_CONFIG_T_super_sample_rate
	>(xn, xk, &status, &config);
	status_data_V.write(status);

#endif
} 

// Multi-channels, fixed-point, 2D arrays + scalars, not inlined
template<
typename _CONFIG_T
>
void   fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8,
		((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T>* status,
		ip_fft::config_t<_CONFIG_T>* config_ch)
{
#ifdef __SYNTHESIS__
#pragma HLS dataflow
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS array_reshape dim=1 variable=xn
#pragma HLS array_reshape dim=1 variable=xk	
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=xk factor=_CONFIG_T_super_sample_rate
	hls::stream<ip_fft::config_t<_CONFIG_T>, 2> config_ch_data_V;
	hls::stream<ip_fft::status_t<_CONFIG_T>, 2> status_data_V;
	config_ch_data_V.write(*config_ch);
	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // not inlined
	*status = status_data_V.read();
#else
	fft_core<
	_CONFIG_T,
	_CONFIG_T::input_width,
	_CONFIG_T::output_width,
	ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1>,
	ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1>,
	1 << _CONFIG_T_max_nfft,
	_CONFIG_T::channels,
	ip_fft::fixed_point,
	_CONFIG_T_super_sample_rate
	>(xn, xk, status, config_ch);
#endif
} 

// Multi-channels, fixed-point, 2D arrays + scalars, inlined
template<
typename _CONFIG_T, fft_T0_t _TAG
>
void   fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8,
		((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T>* status,
		ip_fft::config_t<_CONFIG_T>* config_ch)
{
#ifdef __SYNTHESIS__
#pragma HLS inline
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS array_reshape dim=1 variable=xn
#pragma HLS array_reshape dim=1 variable=xk	
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=xk factor=_CONFIG_T_super_sample_rate
	hls::stream<ip_fft::config_t<_CONFIG_T>, 2> config_ch_data_V;
	hls::stream<ip_fft::status_t<_CONFIG_T>, 2> status_data_V;
	config_ch_data_V.write(*config_ch);
	fft_syn<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V); // not inlined
	*status = status_data_V.read();
#else
	fft_core<
	_CONFIG_T,
	_CONFIG_T::input_width,
	_CONFIG_T::output_width,
	ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1>,
	ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1>,
	1 << _CONFIG_T_max_nfft,
	_CONFIG_T::channels,
	ip_fft::fixed_point,
	_CONFIG_T_super_sample_rate
	>(xn, xk, status, config_ch);
#endif
}

// Multi-channels, fixed-point, 2D arrays + scalar-to-stream, not inlined 
template<
typename _CONFIG_T, fft_T1_t _TAG
>
void   fft(
		std::complex<ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1> > xn[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		std::complex<ap_fixed<((_CONFIG_T::output_width+7)/8)*8,
		((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1> > xk[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T>* status_data_V,
		ip_fft::config_t<_CONFIG_T>* config_ch_data_V)
{
#pragma HLS inline off
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS array_reshape dim=1 variable=xn
#pragma HLS array_reshape dim=1 variable=xk	
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=xk factor=_CONFIG_T_super_sample_rate
#pragma HLS stream variable=status_data_V
#pragma HLS stream variable=config_ch_data_V
        
	fft_core<
	_CONFIG_T,
	_CONFIG_T::input_width,
	_CONFIG_T::output_width,
	ap_fixed<((_CONFIG_T::input_width+7)/8)*8, 1>,
	ap_fixed<((_CONFIG_T::output_width+7)/8)*8, ((_CONFIG_T::output_width+7)/8)*8-_CONFIG_T::input_width+1>,
	1 << _CONFIG_T_max_nfft,
	_CONFIG_T::channels,
	ip_fft::fixed_point,
	_CONFIG_T_super_sample_rate
	>(xn, xk, status_data_V, config_ch_data_V); // inlined

} // End of multi-channels, fixed-point


union U {
	float f;
	unsigned i;
};

template <typename _CONFIG_T>
static void
data_copy_from_float_to_int64(std::complex<float> xn[1 << _CONFIG_T_max_nfft],
		uint64_t xn_cp[1 << _CONFIG_T_max_nfft]) {
	for (unsigned i = 0; i < (1 << _CONFIG_T_max_nfft); i++) {
		std::complex<float> xn_tmp = xn[i];
		U u;
		u.f = xn_tmp.real();
		uint64_t xn_cp_tmp = u.i;
		u.f = xn_tmp.imag();
		xn_cp_tmp |= (((uint64_t)u.i) << 32);
		xn_cp[i] = xn_cp_tmp;
	}
}

template <typename _CONFIG_T>
static void
data_copy_from_int64_to_float(std::complex<float> xk[1 << _CONFIG_T_max_nfft],
		uint64_t xk_cp[1 << _CONFIG_T_max_nfft]) {
	for (unsigned i = 0; i < (1 << _CONFIG_T_max_nfft); i++) {
		uint64_t xk_cp_tmp = xk_cp[i];
		U u;
		u.i = unsigned(xk_cp_tmp);
		std::complex<float> xk_tmp;
		xk_tmp.real(u.f);
		u.i = unsigned(xk_cp_tmp >> 32);
		xk_tmp.imag(u.f);
		xk[i] = xk_tmp;
	}
}

#ifdef __SYNTHESIS__

// 1-channel, floating-point
template <typename _CONFIG_T, char _FFT_INPUT_WIDTH, char _FFT_OUTPUT_WIDTH,
int _FFT_LENGTH, char _FFT_CHANNELS, ip_fft::type _FFT_DATA_FORMAT>
void fft_syn(std::complex<float> xn[1 << _CONFIG_T_max_nfft],
		std::complex<float> xk[1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V) {
#pragma HLS inline off


__fpga_ip("Vivado_FFT",
		//"component_name", "xfft_0",
		"channels", _FFT_CHANNELS,
		"transform_length", _FFT_LENGTH,
		"implementation_options", _CONFIG_T_arch_opt,
		"run_time_configurable_transform_length", _CONFIG_T_has_nfft,
		"data_format", ip_fft::fft_data_format_str[_FFT_DATA_FORMAT],
		"input_width", _FFT_INPUT_WIDTH,
		"output_width", _FFT_OUTPUT_WIDTH,
		"phase_factor_width", _CONFIG_T::phase_factor_width,
		"scaling_options", _CONFIG_T_scaling_opt,
		"rounding_modes", _CONFIG_T_rounding_opt,
		"aclken", "true",
		"aresetn", "true",
		"ovflo", _CONFIG_T::ovflo,
		"xk_index", _CONFIG_T::xk_index,
		"throttle_scheme", "nonrealtime",
		"output_ordering", _CONFIG_T_ordering_opt,
		"cyclic_prefix_insertion", _CONFIG_T::cyclic_prefix_insertion,
		"memory_options_data", _CONFIG_T_mem_data,
		"memory_options_phase_factors", _CONFIG_T_mem_phase_factors,
		"memory_options_reorder", _CONFIG_T_mem_reorder,
		"number_of_stages_using_block_ram_for_data_and_phase_factors", _CONFIG_T_stages_block_ram,
		"memory_options_hybrid", _CONFIG_T_mem_hybrid,
		"complex_mult_type", _CONFIG_T::complex_mult_type,
		"butterfly_type", _CONFIG_T::butterfly_type,
			"systolicfft_inv", _CONFIG_T::systolicfft_inv,
		"super_sample_rates", _CONFIG_T_super_sample_rate
);

ip_fft::config_t<_CONFIG_T> config_tmp = config_ch_data_V.read();
bool has_scaling_sch = config_tmp.getSch();
bool has_direction = config_tmp.getDir();

if (has_direction || has_scaling_sch)
	for (int i = 0; i < _FFT_LENGTH; ++i) {
#pragma HLS unroll factor=_CONFIG_T_super_sample_rate skip_exit_check
		xk[i] = xn[i];
	}

ip_fft::status_t<_CONFIG_T> status_tmp;
status_tmp.data = config_tmp.getDir();
status_data_V.write(status_tmp);
}


template <typename _CONFIG_T, char _FFT_INPUT_WIDTH, char _FFT_OUTPUT_WIDTH,
int _FFT_LENGTH, char _FFT_CHANNELS, ip_fft::type _FFT_DATA_FORMAT>
void fft_syn(hls::stream<std::complex<float>> &xn,
		hls::stream<std::complex<float>> &xk,
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V) {
#pragma HLS inline

__fpga_ip("Vivado_FFT",
		//"component_name", "xfft_0",
		"channels", _FFT_CHANNELS,
		"transform_length", _FFT_LENGTH,
		"implementation_options", _CONFIG_T_arch_opt,
		"run_time_configurable_transform_length", _CONFIG_T_has_nfft,
		"data_format", ip_fft::fft_data_format_str[_FFT_DATA_FORMAT],
		"input_width", _FFT_INPUT_WIDTH,
		"output_width", _FFT_OUTPUT_WIDTH,
		"phase_factor_width", _CONFIG_T::phase_factor_width,
		"scaling_options", _CONFIG_T_scaling_opt,
		"rounding_modes", _CONFIG_T_rounding_opt,
		"aclken", "true",
		"aresetn", "true",
		"ovflo", _CONFIG_T::ovflo,
		"xk_index", _CONFIG_T::xk_index,
		"throttle_scheme", "nonrealtime",
		"output_ordering", _CONFIG_T_ordering_opt,
		"cyclic_prefix_insertion", _CONFIG_T::cyclic_prefix_insertion,
		"memory_options_data", _CONFIG_T_mem_data,
		"memory_options_phase_factors", _CONFIG_T_mem_phase_factors,
		"memory_options_reorder", _CONFIG_T_mem_reorder,
		"number_of_stages_using_block_ram_for_data_and_phase_factors", _CONFIG_T_stages_block_ram,
		"memory_options_hybrid", _CONFIG_T_mem_hybrid,
		"complex_mult_type", _CONFIG_T::complex_mult_type,
		"butterfly_type", _CONFIG_T::butterfly_type,
			"systolicfft_inv", _CONFIG_T::systolicfft_inv,
		"super_sample_rates", _CONFIG_T_super_sample_rate 
);

ip_fft::config_t<_CONFIG_T> config_tmp = config_ch_data_V.read();
bool has_scaling_sch = config_tmp.getSch();
bool has_direction = config_tmp.getDir();

if (has_direction || has_scaling_sch)
	for (int i = 0; i < _FFT_LENGTH; ++i) {
		xk.write(xn.read());
	}

ip_fft::status_t<_CONFIG_T> status_tmp;
status_tmp.data = config_tmp.getDir();
status_data_V.write(status_tmp);
}


template <typename _CONFIG_T, char _FFT_INPUT_WIDTH, char _FFT_OUTPUT_WIDTH,
int _FFT_LENGTH, char _FFT_CHANNELS, ip_fft::type _FFT_DATA_FORMAT>
void fft_syn(hls::stream<hls::vector<std::complex<float>, _CONFIG_T_super_sample_rate>> &xn,
		hls::stream<hls::vector<std::complex<float>, _CONFIG_T_super_sample_rate>> &xk,
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V) {
#pragma HLS inline 

__fpga_ip("Vivado_FFT",
		//"component_name", "xfft_0",
		"channels", _FFT_CHANNELS,
		"transform_length", _FFT_LENGTH,
		"implementation_options", _CONFIG_T_arch_opt,
		"run_time_configurable_transform_length", _CONFIG_T_has_nfft,
		"data_format", ip_fft::fft_data_format_str[_FFT_DATA_FORMAT],
		"input_width", _FFT_INPUT_WIDTH,
		"output_width", _FFT_OUTPUT_WIDTH,
		"phase_factor_width", _CONFIG_T::phase_factor_width,
		"scaling_options", _CONFIG_T_scaling_opt,
		"rounding_modes", _CONFIG_T_rounding_opt,
		"aclken", "true",
		"aresetn", "true",
		"ovflo", _CONFIG_T::ovflo,
		"xk_index", _CONFIG_T::xk_index,
		"throttle_scheme", "nonrealtime",
		"output_ordering", _CONFIG_T_ordering_opt,
		"cyclic_prefix_insertion", _CONFIG_T::cyclic_prefix_insertion,
		"memory_options_data", _CONFIG_T_mem_data,
		"memory_options_phase_factors", _CONFIG_T_mem_phase_factors,
		"memory_options_reorder", _CONFIG_T_mem_reorder,
		"number_of_stages_using_block_ram_for_data_and_phase_factors", _CONFIG_T_stages_block_ram,
		"memory_options_hybrid", _CONFIG_T_mem_hybrid,
		"complex_mult_type", _CONFIG_T::complex_mult_type,
		"butterfly_type", _CONFIG_T::butterfly_type,
			"systolicfft_inv", _CONFIG_T::systolicfft_inv,
		"super_sample_rates", _CONFIG_T_super_sample_rate 
);

ip_fft::config_t<_CONFIG_T> config_tmp = config_ch_data_V.read();
bool has_scaling_sch = config_tmp.getSch();
bool has_direction = config_tmp.getDir();

if (has_direction || has_scaling_sch)
	for (int i = 0; i < (1 << _CONFIG_T_max_nfft ); ++i) {
		xk.write(xn.read());
	}

ip_fft::status_t<_CONFIG_T> status_tmp;
status_tmp.data = config_tmp.getDir();
status_data_V.write(status_tmp);
}


template<
typename _CONFIG_T
>
void fft_wrapper(std::complex<float> xn[1 << _CONFIG_T_max_nfft],
		std::complex<float> xk[1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::config_t<_CONFIG_T> > &config_ch_data_V,
		hls::stream<ip_fft::status_t<_CONFIG_T> > &status_data_V, bool &flag) {
#pragma HLS dataflow
        fft_syn<_CONFIG_T, 32, 32, 1 << _CONFIG_T_max_nfft, 1,
			_CONFIG_T::use_native_float ? ip_fft::native_floating_point : ip_fft::floating_point>(xn, xk, status_data_V, config_ch_data_V); // not inlined
        flag = true;
}

#endif

// 1-channel, floating-point
template <typename _CONFIG_T>
void fft_sim(std::complex<float> xn[1 << _CONFIG_T_max_nfft],
		std::complex<float> xk[1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T> *status,
		ip_fft::config_t<_CONFIG_T> *config_ch) {
	fft_core<
	_CONFIG_T,
	32,
	32,
	float,
	float,
	1 << _CONFIG_T_max_nfft,
	1,
	ip_fft::floating_point,
	_CONFIG_T_super_sample_rate
	>(xn, xk, status, config_ch);
} 

// 1-channel, floating-point, streaming
template <typename _CONFIG_T>
void fft(hls::stream<hls::vector<std::complex<float>, _CONFIG_T_super_sample_rate>> &xn,
		hls::stream<hls::vector<std::complex<float>, _CONFIG_T_super_sample_rate>> &xk,
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V) {
#pragma HLS inline off
#ifdef __SYNTHESIS__
	fft_syn<_CONFIG_T, 32, 32, 1 << _CONFIG_T_max_nfft, 1,
			_CONFIG_T::use_native_float ? ip_fft::native_floating_point : ip_fft::floating_point>(xn, xk, status_data_V, config_ch_data_V); // inlined
#else
	std::complex<float> xn_a[1 << _CONFIG_T_max_nfft];
	std::complex<float> xk_a[1 << _CONFIG_T_max_nfft];
	ip_fft::status_t<_CONFIG_T> status;
	ip_fft::config_t<_CONFIG_T> config;

	config = config_ch_data_V.read();
	unsigned int bound = _CONFIG_T_has_nfft ? config.getNfft() : _CONFIG_T_max_nfft;
	for (unsigned int i = 0; i < 1 << bound; i += _CONFIG_T_super_sample_rate) {
        hls::vector<std::complex<float>, _CONFIG_T_super_sample_rate> in = xn.read();
        for (unsigned int j = 0; j < _CONFIG_T_super_sample_rate; j++) {
            xn_a[i + j] = in[j];
        }
    }
	fft_sim<_CONFIG_T>(xn_a, xk_a, &status, &config);
	for (unsigned int i = 0; i < 1 << bound; i += _CONFIG_T_super_sample_rate) {
        hls::vector<std::complex<float>, _CONFIG_T_super_sample_rate> out;
        for (unsigned int j = 0; j < _CONFIG_T_super_sample_rate; j++) {
		    out[j] = xk_a[i + j];
        }
		xk.write(out);
    }
	status_data_V.write(status);
#endif
} 

// 1-channel, floating-point, streaming
template <typename _CONFIG_T>
void fft(hls::stream<std::complex<float>> &xn,
		hls::stream<std::complex<float>> &xk,
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V) {
#pragma HLS inline off
#ifdef __SYNTHESIS__
	fft_syn<_CONFIG_T, 32, 32, 1 << _CONFIG_T_max_nfft, 1,
			_CONFIG_T::use_native_float ? ip_fft::native_floating_point : ip_fft::floating_point>(xn, xk, status_data_V, config_ch_data_V); // inlined
#else
	std::complex<float> xn_a[1 << _CONFIG_T_max_nfft];
	std::complex<float> xk_a[1 << _CONFIG_T_max_nfft];
	ip_fft::status_t<_CONFIG_T> status;
	ip_fft::config_t<_CONFIG_T> config;

	config = config_ch_data_V.read();
	unsigned int bound = _CONFIG_T_has_nfft ? config.getNfft() : _CONFIG_T_max_nfft;
	for (unsigned int i = 0; i < 1 << bound; i++)
		xn_a[i] = xn.read();
	fft_sim<_CONFIG_T>(xn_a, xk_a, &status, &config);
	for (unsigned int i = 0; i < 1 << bound; i++)
		xk.write(xk_a[i]);
	status_data_V.write(status);
#endif
} 

// 1-channel, floating-point, streaming, non-blocking for simulation
// FIXME add SSR
template <typename _CONFIG_T, fft_T2_t _TAG>
void fft(hls::stream<std::complex<float>> &xn,
		hls::stream<std::complex<float>> &xk,
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V) {
#pragma HLS inline off
#ifdef __SYNTHESIS__
	fft_syn<_CONFIG_T, 32, 32, 1 << _CONFIG_T_max_nfft, 1,
			_CONFIG_T::use_native_float ? ip_fft::native_floating_point : ip_fft::floating_point>(xn, xk, status_data_V, config_ch_data_V); // inlined
#else
	std::complex<float> xn_a[1 << _CONFIG_T_max_nfft];
	std::complex<float> xk_a[1 << _CONFIG_T_max_nfft];
	ip_fft::status_t<_CONFIG_T> status;
	static ip_fft::config_t<_CONFIG_T> config;

	config_ch_data_V.read_nb(config);
	unsigned int bound = _CONFIG_T_has_nfft ? config.getNfft() : _CONFIG_T_max_nfft;
	for (unsigned int i = 0; i < 1 << bound; i++)
		xn_a[i] = xn.read();
	fft_sim<_CONFIG_T>(xn_a, xk_a, &status, &config);
	for (unsigned int i = 0; i < 1 << bound; i++)
		xk.write(xk_a[i]);
	status_data_V.write(status);
#endif
} 


// 1-channel, floating-point, 1D arrays + streams
template <typename _CONFIG_T>
void fft(std::complex<float> xn[1 << _CONFIG_T_max_nfft],
		std::complex<float> xk[1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V) {
#ifdef __SYNTHESIS__
#pragma HLS inline
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
    fft_syn<_CONFIG_T, 32, 32, 1 << _CONFIG_T_max_nfft, 1,_CONFIG_T::use_native_float ? ip_fft::native_floating_point : ip_fft::floating_point>(xn, xk, status_data_V, config_ch_data_V); // not inlined
#else
	ip_fft::status_t<_CONFIG_T> status;
	ip_fft::config_t<_CONFIG_T> config;

	config = config_ch_data_V.read();
	fft_sim<_CONFIG_T>(xn, xk, &status, &config);
	status_data_V.write(status);
#endif
} 


// 1-channel, floating-point, 1D arrays + streams, non-blocking for simulation
template <typename _CONFIG_T, fft_T2_t _TAG>
void fft(std::complex<float> xn[1 << _CONFIG_T_max_nfft],
		std::complex<float> xk[1 << _CONFIG_T_max_nfft],
		hls::stream<ip_fft::status_t<_CONFIG_T>> &status_data_V,
		hls::stream<ip_fft::config_t<_CONFIG_T>> &config_ch_data_V) {

#ifdef __SYNTHESIS__
#pragma HLS inline
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
	fft_syn<_CONFIG_T, 32, 32, 1 << _CONFIG_T_max_nfft, 1,
			_CONFIG_T::use_native_float ? ip_fft::native_floating_point : ip_fft::floating_point>(xn, xk, status_data_V, config_ch_data_V); // not inlined
#else
	ip_fft::status_t<_CONFIG_T> status;
	static ip_fft::config_t<_CONFIG_T> config;

	config_ch_data_V.read_nb(config);
	fft_sim<_CONFIG_T>(xn, xk, &status, &config);
	status_data_V.write(status);
#endif
} 


// 1-channel, floating-point, 1D arrays + scalars, not inlined
template <typename _CONFIG_T>
void fft(std::complex<float> xn[1 << _CONFIG_T_max_nfft],
		std::complex<float> xk[1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T> *status,
		ip_fft::config_t<_CONFIG_T> *config_ch) {

#ifdef __SYNTHESIS__
#pragma HLS dataflow
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
	hls::stream<ip_fft::config_t<_CONFIG_T>, 2> config_ch_data_V;
	hls::stream<ip_fft::status_t<_CONFIG_T>, 2> status_data_V;
	config_ch_data_V.write(*config_ch);
	fft_syn<_CONFIG_T, 32, 32, 1 << _CONFIG_T_max_nfft, 1,
			_CONFIG_T::use_native_float ? ip_fft::native_floating_point : ip_fft::floating_point>(xn, xk, status_data_V, config_ch_data_V); // not inlined
	*status = status_data_V.read();

#else
	fft_sim<_CONFIG_T>(xn, xk, status, config_ch);
#endif
} 

// 1-channel, floating-point, 1D arrays + scalars, inlined
template <typename _CONFIG_T, fft_T0_t _TAG>
void fft(std::complex<float> xn[1 << _CONFIG_T_max_nfft],
		std::complex<float> xk[1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T> *status,
		ip_fft::config_t<_CONFIG_T> *config_ch) {

#ifdef __SYNTHESIS__
#pragma HLS inline
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
	hls::stream<ip_fft::config_t<_CONFIG_T>, 2> config_ch_data_V;
	hls::stream<ip_fft::status_t<_CONFIG_T>, 2> status_data_V;
	config_ch_data_V.write(*config_ch);
	fft_syn<_CONFIG_T, 32, 32, 1 << _CONFIG_T_max_nfft, 1,
			_CONFIG_T::use_native_float ? ip_fft::native_floating_point : ip_fft::floating_point>(xn, xk, status_data_V, config_ch_data_V); // not inlined
	*status = status_data_V.read();

#else
	fft_sim<_CONFIG_T>(xn, xk, status, config_ch);
#endif
} 

// 1-channel, floating-point, 1D arrays + scalar-to-stream, not inlined
template <typename _CONFIG_T, fft_T1_t _TAG>
void fft(std::complex<float> xn[1 << _CONFIG_T_max_nfft],
		std::complex<float> xk[1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T> *status_data_V,
		ip_fft::config_t<_CONFIG_T> *config_ch_data_V) {

#ifdef __SYNTHESIS__
#pragma HLS inline off
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate
#pragma HLS stream variable=status_data_V
#pragma HLS stream variable=config_ch_data_V

	fft_core<_CONFIG_T, 32, 32, float, float, 1 << _CONFIG_T_max_nfft, 1, _CONFIG_T::use_native_float ? ip_fft::native_floating_point : ip_fft::floating_point, _CONFIG_T_super_sample_rate>(xn, xk, status_data_V, config_ch_data_V); // inlined
#else
	fft_sim<_CONFIG_T>(xn, xk, status_data_V, config_ch_data_V);
#endif
} 

// 1-channel, floating-point, arrays + scalars, with extra wrapper 
template <typename _CONFIG_T, fft_T3_t _TAG>
void fft(std::complex<float> xn[1 << _CONFIG_T_max_nfft],
		std::complex<float> xk[1 << _CONFIG_T_max_nfft],
		ip_fft::status_t<_CONFIG_T> *status,
		ip_fft::config_t<_CONFIG_T> *config_ch) {

#ifdef __SYNTHESIS__
#pragma HLS dataflow
#pragma HLS aggregate variable=xn
#pragma HLS aggregate variable=xk
#pragma HLS stream variable=xn
#pragma HLS stream variable=xk
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xn factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=xk factor=_CONFIG_T_super_sample_rate

	hls::stream<ip_fft::config_t<_CONFIG_T>, 2> config_ch_data_V;
	hls::stream<ip_fft::status_t<_CONFIG_T>, 2> status_data_V;
        bool flag;
#pragma HLS STREAM variable=flag type=pipo
	config_ch_data_V.write(*config_ch);
	fft_wrapper<_CONFIG_T>(xn, xk, config_ch_data_V, status_data_V, flag); // not inlined
        fft_copy(status_data_V, status, flag);
#else
	fft_sim<_CONFIG_T>(xn, xk, status, config_ch);
#endif
} // End of 1-channel, floating-point 


// input related functions
// set_config scalar 1 channel
template<typename _CONFIG_T>
void set_config(hls::ip_fft::config_t<_CONFIG_T> &config,
                bool fwd_inv = false,
                int scale_sch = -1,
                unsigned nfft = _CONFIG_T_max_nfft,
                int cp_len = -1) {
        hls::ip_fft::config_t<_CONFIG_T> config_tmp;
        config_tmp.setDir(fwd_inv);
        if (_CONFIG_T_has_nfft)
            config_tmp.setNfft(nfft);
        config_tmp.setSch(scale_sch >= 0 ? scale_sch : 0x2AA);
        if (cp_len >=0)
            config_tmp.setCpLen(cp_len);
        config = config_tmp;
}

// set_config scalar multi-channel
template<typename _CONFIG_T>
void set_config(hls::ip_fft::config_t<_CONFIG_T> &config,
                hls::vector<bool, _CONFIG_T::channels> &fwd_inv,
                hls::vector<int, _CONFIG_T::channels> &scale_sch,
                unsigned nfft = _CONFIG_T_max_nfft,
                int cp_len = -1) {
        hls::ip_fft::config_t<_CONFIG_T> config_tmp;
        if (_CONFIG_T_has_nfft)
            config_tmp.setNfft(nfft);
        if (cp_len >=0)
            config_tmp.setCpLen(cp_len);
        for (int i = 0; i < _CONFIG_T::channels; i++) {
#pragma HLS unroll
            config_tmp.setDir(fwd_inv[i], i);
            if (_CONFIG_T_scaling_opt == ip_fft::scaled)
                config_tmp.setSch(scale_sch[i], i);
        }
        config = config_tmp;
}

// set_data streaming 1 channel
template<typename _CONFIG_T,
         typename _DATA_IN_T>
void set_data(unsigned int nfft,
              hls::stream<std::complex<_DATA_IN_T>>& in,
              std::complex<_DATA_IN_T>  out[1 << _CONFIG_T_max_nfft]) {
        assert(nfft <= _CONFIG_T_max_nfft);
        unsigned length = 1 << nfft;
        assert(length <= 1 << _CONFIG_T_max_nfft);
        assert(length>0);
        
        for (int i=0; i<length; i++) {
#pragma HLS pipeline II=1 rewind style=flp
            out[i] = in.read();
        }
}

// set_data streaming SSR>1 1 channel
template<typename _CONFIG_T,
         typename _DATA_IN_T>
void set_data(unsigned int nfft,
              hls::stream<hls::vector<std::complex<_DATA_IN_T>, _CONFIG_T_super_sample_rate>>& in,
              std::complex<_DATA_IN_T>  out[1 << _CONFIG_T_max_nfft]) {
        assert(nfft <= _CONFIG_T_max_nfft);
        unsigned length = 1 << nfft;
        assert(length <= 1 << _CONFIG_T_max_nfft);
        assert(length>0);
        
        for (int i=0; i<length; i += _CONFIG_T_super_sample_rate) {
#pragma HLS pipeline II=1 rewind style=flp
                hls::vector<std::complex<_DATA_IN_T>, _CONFIG_T_super_sample_rate> tmp = in.read();
                for (int j = 0; j < _CONFIG_T_super_sample_rate; j++) {
                    out[i + j] = tmp[j];
                }
        }
}

// set_data array 1 channel
template<typename _CONFIG_T,
         typename _DATA_IN_T>
void set_data(unsigned int nfft,
              std::complex<_DATA_IN_T>  in[1 << _CONFIG_T_max_nfft], 
              std::complex<_DATA_IN_T>  out[1 << _CONFIG_T_max_nfft]) {
        assert(nfft <= _CONFIG_T_max_nfft);
        unsigned length = 1 << nfft;
        assert(length <= 1 << _CONFIG_T_max_nfft);
        assert(length>0);
        
        for (int i=0; i<length; i++) {
#pragma HLS pipeline II=1 rewind style=flp
#pragma HLS unroll factor=_CONFIG_T_super_sample_rate skip_exit_check
            out[i] = in[i];
        }
}

// set_data array multi-channel
template<typename _CONFIG_T,
         typename _DATA_IN_T>
void set_data(unsigned int nfft,
              std::complex<_DATA_IN_T>  in[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft], 
              std::complex<_DATA_IN_T>  out[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft]) {
        assert(nfft <= _CONFIG_T_max_nfft);
        unsigned length = 1 << nfft;
        assert(length <= 1 << _CONFIG_T_max_nfft);
        assert(length>0);
        
        for (int i=0; i<length; i++) {
#pragma HLS pipeline II=1 rewind style=flp
#pragma HLS unroll factor=_CONFIG_T_super_sample_rate skip_exit_check
            for (int k = 0; k < _CONFIG_T::channels; k++) {
                out[k][i] = in[k][i];
            }
        }
}

// inputdatamover streaming 1 channel
template<typename _CONFIG_T,
         typename _DATA_IN_T>
void inputdatamover(hls::stream<std::complex<_DATA_IN_T>>&  in,
                    std::complex<_DATA_IN_T>  out[1 << _CONFIG_T_max_nfft],
                    hls::ip_fft::config_t<_CONFIG_T> &config,
                    unsigned nfft = _CONFIG_T_max_nfft,
                    bool fwd_inv = false,
                    int scale_sch = -1,
                    int cp_len = -1) {
#pragma HLS dataflow
#pragma HLS inline
#pragma HLS STREAM variable=config type=pipo

    set_config<_CONFIG_T>(config, fwd_inv, scale_sch, nfft, cp_len);
	set_data<_CONFIG_T, _DATA_IN_T>(nfft, in, out);
}

// inputdatamover streaming SSR>1 1 channel
template<typename _CONFIG_T,
         typename _DATA_IN_T>
void inputdatamover(hls::stream<hls::vector<std::complex<_DATA_IN_T>, _CONFIG_T_super_sample_rate>>&  in,
                    std::complex<_DATA_IN_T>  out[1 << _CONFIG_T_max_nfft],
                    hls::ip_fft::config_t<_CONFIG_T> &config,
                    unsigned nfft = _CONFIG_T_max_nfft,
                    bool fwd_inv = false,
                    int scale_sch = -1,
                    int cp_len = -1) {
#pragma HLS dataflow
#pragma HLS inline
#pragma HLS STREAM variable=config type=pipo

    set_config<_CONFIG_T>(config, fwd_inv, scale_sch, nfft, cp_len);
	set_data<_CONFIG_T, _DATA_IN_T>(nfft, in, out);
}

// inputdatamover array 1 channel
template<typename _CONFIG_T,
         typename _DATA_IN_T>
void inputdatamover(std::complex<_DATA_IN_T>  in[1 << _CONFIG_T_max_nfft], 
                    std::complex<_DATA_IN_T>  out[1 << _CONFIG_T_max_nfft],
                    hls::ip_fft::config_t<_CONFIG_T> &config,
                    unsigned nfft = _CONFIG_T_max_nfft,
                    bool fwd_inv = false,
                    int scale_sch = -1,
                    int cp_len = -1) {
#pragma HLS dataflow
#pragma HLS inline
    set_config<_CONFIG_T>(config, fwd_inv, scale_sch, nfft, cp_len);
    set_data<_CONFIG_T, _DATA_IN_T>(nfft, in, out);
}

// inputdatamover array multi-channel
template<typename _CONFIG_T,
         typename _DATA_IN_T>
void inputdatamover(std::complex<_DATA_IN_T>  in[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft], 
                    std::complex<_DATA_IN_T>  out[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
                    hls::ip_fft::config_t<_CONFIG_T> &config,
                    hls::vector<bool, _CONFIG_T::channels> &fwd_inv,
                    hls::vector<int, _CONFIG_T::channels> &scale_sch,
                    unsigned nfft = _CONFIG_T_max_nfft,
                    int cp_len = -1) {
#pragma HLS dataflow
#pragma HLS inline
    set_config<_CONFIG_T>(config, fwd_inv, scale_sch, nfft, cp_len);
    set_data<_CONFIG_T, _DATA_IN_T>(nfft, in, out);
}

// output related functions
// get_status scalar 1 channel
template<typename _CONFIG_T>
void get_status(hls::ip_fft::status_t<_CONFIG_T> &status, 
                bool *ovflo = 0,
                unsigned *blk_exp = 0) {
    const bool has_ovflo = _CONFIG_T::ovflo && (_CONFIG_T_scaling_opt == unsigned(ip_fft::scaled));
    const bool has_blk_exp = _CONFIG_T_scaling_opt == unsigned(ip_fft::block_floating_point);
 #ifndef __SYNTHESIS__
	 assert((ovflo || !has_ovflo) && (blk_exp || !has_blk_exp)); 
 #endif
    if constexpr (has_ovflo)
        *ovflo = status.getOvflo();
    if constexpr (has_blk_exp)
        *blk_exp = status.getBlkExp();
}

// get_status scalar multi-channel
template<typename _CONFIG_T>
void get_status(hls::ip_fft::status_t<_CONFIG_T> &status, 
                hls::vector<bool, _CONFIG_T::channels> *ovflo = 0,
                hls::vector<unsigned, _CONFIG_T::channels> *blk_exp = 0) {
    const bool has_ovflo = _CONFIG_T::ovflo && (_CONFIG_T_scaling_opt == unsigned(ip_fft::scaled));
    const bool has_blk_exp = _CONFIG_T_scaling_opt == unsigned(ip_fft::block_floating_point);
 #ifndef __SYNTHESIS__
	 assert((ovflo || !has_ovflo) && (blk_exp || !has_blk_exp)); 
 #endif
    if constexpr (has_ovflo) {
        for (int i = 0; i < _CONFIG_T::channels; i++) {
        #pragma HLS unroll
            (*ovflo)[i] = status.getOvflo(i);
        }
    }
    if constexpr (has_blk_exp) {
        for (int i = 0; i < _CONFIG_T::channels; i++) {
        #pragma HLS unroll
            (*blk_exp)[i] = status.getBlkExp(i);
        }
    }
}

// get_data array 1 channel
template<typename _CONFIG_T,
         typename _DATA_OUT_T>
void get_data(std::complex<_DATA_OUT_T>  in[1 << _CONFIG_T_max_nfft], 
              std::complex<_DATA_OUT_T>  out[1 << _CONFIG_T_max_nfft],
              unsigned nfft = _CONFIG_T_max_nfft) {
        unsigned length = 1 << nfft;
        assert(nfft <= _CONFIG_T_max_nfft);
        assert(length <= 1 << _CONFIG_T_max_nfft);
        assert(length>0);
        for (int i=0; i<length; i++) {
#pragma HLS pipeline II=1 rewind style=flp
#pragma HLS unroll factor=_CONFIG_T_super_sample_rate skip_exit_check
                std::complex<_DATA_OUT_T> tmp = in[i];
                out[i] = tmp;
        }
}

// get_data streaming 1 channel
template<typename _CONFIG_T,
         typename _DATA_OUT_T>
void get_data(std::complex<_DATA_OUT_T>  in[1 << _CONFIG_T_max_nfft], 
              hls::stream<std::complex<_DATA_OUT_T>>&  out,
              unsigned nfft = _CONFIG_T_max_nfft) {
        unsigned length = 1 << nfft;
        assert(nfft <= _CONFIG_T_max_nfft);
        assert(length <= 1 << _CONFIG_T_max_nfft);
        assert(length>0);
        for (int i=0; i<length; i++) {
#pragma HLS pipeline II=1 rewind style=flp
                std::complex<_DATA_OUT_T> tmp = in[i];
                out.write(tmp);
        }
}

// get_data streaming SSR>1 1 channel
template<typename _CONFIG_T,
         typename _DATA_OUT_T>
void get_data(std::complex<_DATA_OUT_T>  in[1 << _CONFIG_T_max_nfft], 
              hls::stream<hls::vector<std::complex<_DATA_OUT_T>, _CONFIG_T_super_sample_rate>>&  out,
              unsigned nfft = _CONFIG_T_max_nfft) {
        unsigned length = 1 << nfft;
        assert(nfft <= _CONFIG_T_max_nfft);
        assert(length <= 1 << _CONFIG_T_max_nfft);
        assert(length>0);
        for (int i=0; i<length; i += _CONFIG_T_super_sample_rate) {
#pragma HLS pipeline II=1 rewind style=flp
                hls::vector<std::complex<_DATA_OUT_T>, _CONFIG_T_super_sample_rate> tmp;
                for (int j = 0; j < _CONFIG_T_super_sample_rate; j++) {
                    tmp[j] = in[i + j];
                }
                out.write(tmp);
        }
}

// get_data array multi-channel
template<typename _CONFIG_T,
         typename _DATA_OUT_T>
void get_data(std::complex<_DATA_OUT_T>  in[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft], 
              std::complex<_DATA_OUT_T>  out[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
              unsigned nfft = _CONFIG_T_max_nfft) {
        unsigned length = 1 << nfft;
        assert(nfft <= _CONFIG_T_max_nfft);
        assert(length <= 1 << _CONFIG_T_max_nfft);
        assert(length>0);
        for (int i=0; i<length; i++) {
#pragma HLS pipeline II=1 rewind style=flp
#pragma HLS unroll factor=_CONFIG_T_super_sample_rate skip_exit_check
            for (int k = 0; k < _CONFIG_T::channels; k++) {
                out[k][i] = in[k][i];
            }
        }
}

// outputdatamover streaming 1 channel
template<typename _CONFIG_T,
         typename _DATA_OUT_T>
void outputdatamover(std::complex<_DATA_OUT_T>  in[1 << _CONFIG_T_max_nfft], 
                     hls::stream<std::complex<_DATA_OUT_T>>&  out,
                     hls::ip_fft::status_t<_CONFIG_T>  &status,
                     bool *ovflo = 0,
                     unsigned *blk_exp = 0,
                     unsigned nfft = _CONFIG_T_max_nfft) {
#pragma HLS dataflow
#pragma HLS inline

        get_status<_CONFIG_T>(status, ovflo, blk_exp);
        get_data<_CONFIG_T, _DATA_OUT_T>(in, out, nfft);
}

// outputdatamover streaming SSR>1 1 channel
template<typename _CONFIG_T,
         typename _DATA_OUT_T>
void outputdatamover(std::complex<_DATA_OUT_T>  in[1 << _CONFIG_T_max_nfft], 
                     hls::stream<hls::vector<std::complex<_DATA_OUT_T>, _CONFIG_T_super_sample_rate>>&  out,
                     hls::ip_fft::status_t<_CONFIG_T>  &status,
                     bool *ovflo = 0,
                     unsigned *blk_exp = 0,
                     unsigned nfft = _CONFIG_T_max_nfft) {
#pragma HLS dataflow
#pragma HLS inline

        get_status<_CONFIG_T>(status, ovflo, blk_exp);
        get_data<_CONFIG_T, _DATA_OUT_T>(in, out, nfft);
}

// outputdatamover array 1 channel
template<typename _CONFIG_T,
         typename _DATA_OUT_T>
void outputdatamover(std::complex<_DATA_OUT_T>  in[1 << _CONFIG_T_max_nfft], 
                     std::complex<_DATA_OUT_T>  out[1 << _CONFIG_T_max_nfft],
                     hls::ip_fft::status_t<_CONFIG_T>  &status,
                     bool *ovflo = 0,
                     unsigned *blk_exp = 0,
                     unsigned nfft = _CONFIG_T_max_nfft) {
#pragma HLS dataflow
#pragma HLS inline

        get_status<_CONFIG_T>(status, ovflo, blk_exp);
        get_data<_CONFIG_T, _DATA_OUT_T>(in, out, nfft);
}

// outputdatamover array multi-channel
template<typename _CONFIG_T,
         typename _DATA_OUT_T>
void outputdatamover(std::complex<_DATA_OUT_T>  in[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft], 
                     std::complex<_DATA_OUT_T>  out[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
                     hls::ip_fft::status_t<_CONFIG_T>  &status,
                     hls::vector<bool, _CONFIG_T::channels> *ovflo = 0,
                     hls::vector<unsigned, _CONFIG_T::channels> *blk_exp = 0,
                     unsigned nfft = _CONFIG_T_max_nfft) {
#pragma HLS dataflow
#pragma HLS inline

        get_status<_CONFIG_T>(status, ovflo, blk_exp);
        get_data<_CONFIG_T, _DATA_OUT_T>(in, out, nfft);
}


// new API streaming 1 channel no nfft
template<typename _CONFIG_T =  hls::ip_fft::params_t,
         typename _DATA_IN_T,
         typename _DATA_OUT_T>
void fft(hls::stream<std::complex<_DATA_IN_T>>& in,
         hls::stream<std::complex<_DATA_OUT_T>>& out,
         bool fwd_inv = false, // direction
         int scale_sch = -1,
         int cp_len = -1,
         bool *ovflo = 0,
         unsigned *blk_exp = 0) {
#pragma HLS dataflow

    hls::ip_fft::config_t<_CONFIG_T> config;
    hls::ip_fft::status_t<_CONFIG_T>  status;
    std::complex<_DATA_IN_T>  xn[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
    std::complex<_DATA_OUT_T>  xk[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
#pragma HLS stream depth=8 variable=xn
#pragma HLS stream depth=8 variable=xk

    inputdatamover(in, xn, config, _CONFIG_T_max_nfft, fwd_inv, scale_sch, cp_len);
    fft<_CONFIG_T>(xn, xk, &status, &config);
    outputdatamover(xk, out, status, ovflo, blk_exp, _CONFIG_T_max_nfft);
}

// new API streaming 1 channel nfft
template<typename _CONFIG_T =  hls::ip_fft::params_t,
         typename _DATA_IN_T,
         typename _DATA_OUT_T>
void fft(unsigned nfft,
         hls::stream<std::complex<_DATA_IN_T>>& in,
         hls::stream<std::complex<_DATA_OUT_T>>& out,
         bool fwd_inv = false, // direction
         int scale_sch = -1,
         int cp_len = -1,
         bool *ovflo = 0,
         unsigned *blk_exp = 0) {
#pragma HLS dataflow

    hls::ip_fft::config_t<_CONFIG_T> config;
    hls::ip_fft::status_t<_CONFIG_T>  status;
    std::complex<_DATA_IN_T>  xn[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
    std::complex<_DATA_OUT_T>  xk[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
#pragma HLS stream depth=8 variable=xn
#pragma HLS stream depth=8 variable=xk

    inputdatamover(in, xn, config, nfft, fwd_inv, scale_sch, cp_len);
    fft<_CONFIG_T>(xn, xk, &status, &config);
    outputdatamover(xk, out, status, ovflo, blk_exp, nfft);
}

// new API streaming SSR>1 1 channel no nfft
template<typename _CONFIG_T =  hls::ip_fft::ssr_params_t,
         typename _DATA_IN_T,
         typename _DATA_OUT_T>
void fft(hls::stream<hls::vector<std::complex<_DATA_IN_T>, _CONFIG_T_super_sample_rate>>&  in,
         hls::stream<hls::vector<std::complex<_DATA_OUT_T>, _CONFIG_T_super_sample_rate>>&  out,
         bool fwd_inv = false, // direction
         int scale_sch = -1,
         int cp_len = -1,
         bool *ovflo = 0,
         unsigned *blk_exp = 0) {
#pragma HLS dataflow

    hls::ip_fft::config_t<_CONFIG_T> config;
    hls::ip_fft::status_t<_CONFIG_T>  status;
    std::complex<_DATA_IN_T>  xn[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
    std::complex<_DATA_OUT_T>  xk[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
#pragma HLS stream depth=8 variable=xn
#pragma HLS stream depth=8 variable=xk

    inputdatamover(in, xn, config, _CONFIG_T_max_nfft, fwd_inv, scale_sch, cp_len);
    fft<_CONFIG_T>(xn, xk, &status, &config);
    outputdatamover(xk, out, status, ovflo, blk_exp, _CONFIG_T_max_nfft);
}

// new API streaming SSR>1 1 channel nfft
template<typename _CONFIG_T =  hls::ip_fft::ssr_params_t,
         typename _DATA_IN_T,
         typename _DATA_OUT_T>
void fft(unsigned nfft,
         hls::stream<hls::vector<std::complex<_DATA_IN_T>, _CONFIG_T_super_sample_rate>>&  in,
         hls::stream<hls::vector<std::complex<_DATA_OUT_T>, _CONFIG_T_super_sample_rate>>&  out,
         bool fwd_inv = false, // direction
         int scale_sch = -1,
         int cp_len = -1,
         bool *ovflo = 0,
         unsigned *blk_exp = 0) {
#pragma HLS dataflow

    hls::ip_fft::config_t<_CONFIG_T> config;
    hls::ip_fft::status_t<_CONFIG_T>  status;
    std::complex<_DATA_IN_T>  xn[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
    std::complex<_DATA_OUT_T>  xk[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
#pragma HLS stream depth=8 variable=xn
#pragma HLS stream depth=8 variable=xk

    inputdatamover(in, xn, config, nfft, fwd_inv, scale_sch, cp_len);
    fft<_CONFIG_T>(xn, xk, &status, &config);
    outputdatamover(xk, out, status, ovflo, blk_exp, nfft);
}

// new API array 1 channel no nfft
template<typename _CONFIG_T =  hls::ip_fft::params_t,
         typename _DATA_IN_T,
         typename _DATA_OUT_T>
void fft(std::complex<_DATA_IN_T>  in[1 << _CONFIG_T_max_nfft], 
         std::complex<_DATA_OUT_T>  out[1 << _CONFIG_T_max_nfft],
         bool fwd_inv = false, // direction
         int scale_sch = -1,
         int cp_len = -1,
         bool *ovflo = 0,
         unsigned *blk_exp = 0) {
#if _RESHAPE_CALLER
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=in factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=out factor=_CONFIG_T_super_sample_rate
#endif
#pragma HLS dataflow

    hls::ip_fft::config_t<_CONFIG_T> config;
    hls::ip_fft::status_t<_CONFIG_T>  status;
    std::complex<_DATA_IN_T>  xn[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
    std::complex<_DATA_OUT_T>  xk[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
#pragma HLS stream depth=8 variable=xn
#pragma HLS stream depth=8 variable=xk

    inputdatamover(in, xn, config, _CONFIG_T_max_nfft, fwd_inv, scale_sch, cp_len);
    fft<_CONFIG_T>(xn, xk, &status, &config);
    outputdatamover(xk, out, status, ovflo, blk_exp, _CONFIG_T_max_nfft);
}

// new API array 1 channel nfft
template<typename _CONFIG_T =  hls::ip_fft::params_t,
         typename _DATA_IN_T,
         typename _DATA_OUT_T>
void fft(unsigned nfft,
         std::complex<_DATA_IN_T>  in[1 << _CONFIG_T_max_nfft], 
         std::complex<_DATA_OUT_T>  out[1 << _CONFIG_T_max_nfft],
         bool fwd_inv = false, // direction
         int scale_sch = -1,
         int cp_len = -1,
         bool *ovflo = 0,
         unsigned *blk_exp = 0) {
#if _RESHAPE_CALLER
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=in factor=_CONFIG_T_super_sample_rate
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic variable=out factor=_CONFIG_T_super_sample_rate
#endif
#pragma HLS dataflow

    hls::ip_fft::config_t<_CONFIG_T> config;
    hls::ip_fft::status_t<_CONFIG_T>  status;
    std::complex<_DATA_IN_T>  xn[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
    std::complex<_DATA_OUT_T>  xk[1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
#pragma HLS stream depth=8 variable=xn
#pragma HLS stream depth=8 variable=xk

    inputdatamover(in, xn, config, nfft, fwd_inv, scale_sch, cp_len);
    fft<_CONFIG_T>(xn, xk, &status, &config);
    outputdatamover(xk, out, status, ovflo, blk_exp, nfft);
}

// new API array multi-channel no nfft
template<typename _CONFIG_T =  hls::ip_fft::params_t,
         typename _DATA_IN_T,
         typename _DATA_OUT_T>
void fft(std::complex<_DATA_IN_T>  in[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft], 
         std::complex<_DATA_OUT_T>  out[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
         hls::vector<bool, _CONFIG_T::channels> &fwd_inv,
         hls::vector<int, _CONFIG_T::channels> &scale_sch,
         int cp_len = -1,
         hls::vector<bool, _CONFIG_T::channels> *ovflo = 0,
         hls::vector<unsigned, _CONFIG_T::channels> *blk_exp = 0) {
#if _RESHAPE_CALLER
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=in factor=_CONFIG_T_super_sample_rate
#endif
#pragma HLS array_reshape complete dim=1 variable=in
#if _RESHAPE_CALLER
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=out factor=_CONFIG_T_super_sample_rate
#endif
#pragma HLS array_reshape complete dim=1 variable=out
#pragma HLS dataflow

    hls::ip_fft::config_t<_CONFIG_T> config;
    hls::ip_fft::status_t<_CONFIG_T>  status;
    std::complex<_DATA_IN_T>  xn[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
    std::complex<_DATA_OUT_T>  xk[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
#pragma HLS stream depth=8 variable=xn
#pragma HLS stream depth=8 variable=xk

    inputdatamover(in, xn, config, fwd_inv, scale_sch, _CONFIG_T_max_nfft, cp_len);
    fft<_CONFIG_T>(xn, xk, &status, &config);
    outputdatamover(xk, out, status, ovflo, blk_exp, _CONFIG_T_max_nfft);
}

// new API array multi-channel nfft
template<typename _CONFIG_T =  hls::ip_fft::params_t,
         typename _DATA_IN_T,
         typename _DATA_OUT_T>
void fft(unsigned nfft,
         std::complex<_DATA_IN_T>  in[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft], 
         std::complex<_DATA_OUT_T>  out[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft],
         hls::vector<bool, _CONFIG_T::channels> &fwd_inv,
         hls::vector<int, _CONFIG_T::channels> &scale_sch,
         int cp_len = -1,
         hls::vector<bool, _CONFIG_T::channels> *ovflo = 0,
         hls::vector<unsigned, _CONFIG_T::channels> *blk_exp = 0) {
#if _RESHAPE_CALLER
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=in factor=_CONFIG_T_super_sample_rate
#endif
#pragma HLS array_reshape complete dim=1 variable=in
#if _RESHAPE_CALLER
#pragma HLS if (_CONFIG_T_super_sample_rate>1) array_reshape cyclic dim=2 variable=out factor=_CONFIG_T_super_sample_rate
#endif
#pragma HLS array_reshape complete dim=1 variable=out
#pragma HLS dataflow

    hls::ip_fft::config_t<_CONFIG_T> config;
    hls::ip_fft::status_t<_CONFIG_T>  status;
    std::complex<_DATA_IN_T>  xn[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
    std::complex<_DATA_OUT_T>  xk[_CONFIG_T::channels][1 << _CONFIG_T_max_nfft]  __attribute__((no_ctor));
#pragma HLS stream depth=8 variable=xn
#pragma HLS stream depth=8 variable=xk

    inputdatamover(in, xn, config, fwd_inv, scale_sch, nfft, cp_len);
    fft<_CONFIG_T>(xn, xk, &status, &config);
    outputdatamover(xk, out, status, ovflo, blk_exp, nfft);
}

} // namespace hls

#endif // __cplusplus
#endif // X_HLS_FFT_H


