#ifndef HLSLIB_XILINX_OPENCL_H
#define HLSLIB_XILINX_OPENCL_H

#include <memory>

namespace hlslib {
namespace ocl {

template <typename T, std::size_t Alignment>
using AlignedAllocator = std::allocator<T>;

}  // namespace ocl
}  // namespace hlslib

#endif
