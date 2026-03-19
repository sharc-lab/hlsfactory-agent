#include "unary.h"
#include "../common/config.h"

void unary(RawDataT *in, RawDataT *out, uint64_t size, int op) {
    // Simple stub: copy input to output
    for (uint64_t i = 0; i < size; ++i) {
        out[i] = in[i];
    }
}
