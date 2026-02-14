#ifndef HLS_STREAMOFBLOCKS_H
#define HLS_STREAMOFBLOCKS_H

// HLS Stream of Blocks library stub for compilation
// This is a minimal stub to allow clang compilation

#include "hls_stream.h"

namespace hls {

template<typename T>
class stream_of_blocks {
public:
    stream<T> strm;
    
    void write(T val) {
        strm.write(val);
    }
    
    T read() {
        return strm.read();
    }
};

} // namespace hls

#endif
