#include "BitBlender.h"
#include <tapa.h>

// Dummy stream definitions (using tapa stubs)
int main() {
    // Create dummy streams; in real use, these would be connected to data.
    tapa::stream<LOAD_DTYPE> dummy_in;
    tapa::ostream<KEY_DTYPE> ks[8][2];
    // Fill dummy input with zeroes
    for (int i = 0; i < 10; ++i) {
        LOAD_DTYPE val;
        dummy_in.write(val);
    }
    // Call the HLS function (MAXSTM version)
    loadKey_MAXSTM(dummy_in,
        ks[0][0], ks[0][1],
        ks[1][0], ks[1][1],
        ks[2][0], ks[2][1],
        ks[3][0], ks[3][1],
        ks[4][0], ks[4][1],
        ks[5][0], ks[5][1],
        ks[6][0], ks[6][1],
        ks[7][0], ks[7][1],
        10);
    return 0;
}
