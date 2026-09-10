#include <cmath>
#include <cstdio>

#include "mac.h"

int main() {
    const double coef_d[N_TAPS] = {0.5, -0.25, 1.0, 0.125, -1.5, 0.75, 2.0, -0.0625};
    const double in_d[N_TAPS] = {1.0, 2.0, -3.0, 4.5, 0.25, -1.75, 8.0, 3.125};

    data_t coef[N_TAPS];
    ac_channel<data_t> in;
    ac_channel<acc_t> out;

    double expected = 0.0;
    for (int i = 0; i < N_TAPS; i++) {
        coef[i] = coef_d[i];
        in.write(data_t(in_d[i]));
        expected += in_d[i] * coef_d[i];
    }

    mac(in, out, coef);

    acc_t got = out.read();
    double got_d = got.to_double();
    double err = std::fabs(got_d - expected);
    std::printf("expected=%f got=%f err=%f\n", expected, got_d, err);
    if (err > 1e-3) {
        std::printf("FAIL\n");
        return 1;
    }
    std::printf("PASS\n");
    return 0;
}
