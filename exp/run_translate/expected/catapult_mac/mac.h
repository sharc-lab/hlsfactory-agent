#ifndef MAC_H
#define MAC_H

#include "ac_fixed.h"
#include "ac_channel.h"

#define N_TAPS 8

typedef ac_fixed<16, 8, true> data_t;
typedef ac_fixed<32, 16, true> acc_t;

void mac(ac_channel<data_t> &in, ac_channel<acc_t> &out, const data_t coef[N_TAPS]);

#endif
