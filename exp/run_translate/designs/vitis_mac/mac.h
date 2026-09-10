#ifndef MAC_H
#define MAC_H

#include "ap_fixed.h"
#include "hls_stream.h"

#define N_TAPS 8

typedef ap_fixed<16, 8> data_t;
typedef ap_fixed<32, 16> acc_t;

void mac(hls::stream<data_t> &in, hls::stream<acc_t> &out, const data_t coef[N_TAPS]);

#endif
