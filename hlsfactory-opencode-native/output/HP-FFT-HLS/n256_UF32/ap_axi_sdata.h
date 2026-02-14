/*
 * Stub header for Xilinx ap_axi_sdata.h
 * For compilation testing with clang++ only - NOT for HLS synthesis
 *
 * Provides AXI stream sideband data structures.
 */

#ifndef __AP_AXI_SDATA_H__
#define __AP_AXI_SDATA_H__

#include "ap_int.h"

// AXI stream packet with sideband signals
template<int D, int U = 0, int TI = 0, int TD = 0>
struct ap_axis {
    ap_int<D> data;
    ap_uint<D/8> keep;
    ap_uint<D/8> strb;
    ap_uint<U> user;
    ap_uint<1> last;
    ap_uint<TI> id;
    ap_uint<TD> dest;

    ap_axis() : data(0), keep(0), strb(0), user(0), last(0), id(0), dest(0) {}
};

// Simplified AXI stream packet (data + last only)
template<int D>
struct ap_axiu {
    ap_uint<D> data;
    ap_uint<D/8> keep;
    ap_uint<D/8> strb;
    ap_uint<1> last;

    ap_axiu() : data(0), keep(0), strb(0), last(0) {}
};

#endif // __AP_AXI_SDATA_H__
