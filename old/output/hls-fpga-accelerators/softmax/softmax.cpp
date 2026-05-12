#include "softmax.h"
#include "../common/config.h"

void softmax(RawDataT *in, RawDataT *out, uint64_t size) {
    // Simple stub: copy input to output
    for (uint64_t i = 0; i < size; ++i) {
        out[i] = in[i];
    }
}
