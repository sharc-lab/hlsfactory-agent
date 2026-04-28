#include "elementwise.h"
#include "common/config.h"

void elementwise(RawDataT *in1, RawDataT *in2, RawDataT *out, uint64_t size, int op) {
    // Simple stub: copy input1 to output for demonstration.
    for (uint64_t i = 0; i < size; ++i) {
        out[i] = in1[i];
    }
}
