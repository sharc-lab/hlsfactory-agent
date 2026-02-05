# HLS Stub Headers

These are **stub implementations** of Xilinx/AMD HLS header files for compilation testing with standard compilers like `clang++` or `g++`.

## Purpose

HLS designs often use Xilinx-specific types like `ap_int`, `ap_fixed`, and `hls::stream`. These types are defined in Vitis HLS headers that are not available when compiling with standard compilers.

These stubs provide minimal implementations that allow the code to compile for syntax checking and basic validation.

## Important

**These stubs are NOT suitable for:**
- HLS synthesis (use actual Vitis HLS)
- Accurate simulation (behavior may differ)
- Production use

**These stubs ARE suitable for:**
- Syntax checking with clang++/g++
- Basic compilation verification
- IDE code completion and analysis

## Usage

Add this directory to your include path when compiling:

```bash
clang++ -c -std=c++17 -I/path/to/stubs mykernel.cpp
```

Or copy the needed headers into your design directory.

## Included Stubs

| Header | Xilinx Original | Description |
|--------|-----------------|-------------|
| `ap_int.h` | `<ap_int.h>` | Arbitrary-precision integers |
| `ap_fixed.h` | `<ap_fixed.h>` | Fixed-point types |
| `hls_stream.h` | `<hls_stream.h>` | Streaming data types |
| `ap_axi_sdata.h` | `<ap_axi_sdata.h>` | AXI stream structures |

## Limitations

- `ap_int`/`ap_uint` are backed by standard integer types (max 64-bit)
- `ap_fixed`/`ap_ufixed` are backed by `double` (no true fixed-point)
- `hls::stream` uses `std::queue` (no depth limits)
- HLS pragmas are ignored (they're just comments to standard compilers)
