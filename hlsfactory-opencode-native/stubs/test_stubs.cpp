#include "ap_int.h"
#include "ap_fixed.h"
#include "hls_stream.h"

void test_kernel(ap_uint<32> in, ap_uint<32>& out, hls::stream<ap_int<16>>& s) {
    ap_fixed<16,8> fixed_val = 3.14;
    out = in + 1;
    s.write(42);
}

int main() {
    ap_uint<32> a = 10, b;
    hls::stream<ap_int<16>> stream;
    test_kernel(a, b, stream);
    return 0;
}
