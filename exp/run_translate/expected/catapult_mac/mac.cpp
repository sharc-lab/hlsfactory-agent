#include "mac.h"

#pragma hls_design top
void mac(ac_channel<data_t> &in, ac_channel<acc_t> &out, const data_t coef[N_TAPS]) {
    acc_t acc = 0;
#pragma hls_pipeline_init_interval 1
    for (int i = 0; i < N_TAPS; i++) {
        data_t x = in.read();
        acc += x * coef[i];
    }
    out.write(acc);
}
