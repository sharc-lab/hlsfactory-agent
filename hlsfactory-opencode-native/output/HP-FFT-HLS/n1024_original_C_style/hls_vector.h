#ifndef HLS_VECTOR_H
#define HLS_VECTOR_H

// HLS Vector library stub for compilation
// This is a minimal stub to allow clang compilation

namespace hls {

template<typename T, int N>
class vector {
public:
    T data[N];
    
    vector() {}
    
    T& operator[](int i) {
        return data[i];
    }
    
    const T& operator[](int i) const {
        return data[i];
    }
};

} // namespace hls

#endif
