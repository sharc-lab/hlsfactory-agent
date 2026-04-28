#include "rmsnorm.h"
#include "../common/config.h"

void rmsnorm(RawDataT *in, RawDataT *out, uint64_t size) {
    for (uint64_t i = 0; i < size; ++i) {
        out[i] = in[i];
    }
}
