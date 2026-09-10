#include "mac.h"

void mac(hls::stream<data_t> &in, hls::stream<acc_t> &out, const data_t coef[N_TAPS]) {
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
#pragma HLS ARRAY_PARTITION variable=coef complete dim=1

    acc_t acc = 0;
    for (int i = 0; i < N_TAPS; i++) {
#pragma HLS PIPELINE II=1
        data_t x = in.read();
        acc += x * coef[i];
    }
    out.write(acc);
}
