# ORC Processing Design (orc_proc)

## Overview
This is the main top-level module for the FORC (FPGA ORC) accelerator. It orchestrates the complete ORC file processing pipeline including data reading, decompression, decoding, filtering, and writing.

## Architecture

The design consists of the following stages:
1. **Data Reading** - Memory-mapped to stream conversion
2. **ORC Decompression** - Zlib decompression of compressed ORC data
3. **Decomp to Decoder Connector** - Stream buffering and synchronization
4. **ORC Decoder** - ORC format decoding (DIRECT, DELTA, PATCHED BASE, SHORT REPEAT)
5. **ORC Filter** - Data filtering based on range or index conditions
6. **Data Writing** - Writing filtered results to memory

## Top-Level Function

```cpp
void orc_proc(
    tapa::mmap<_512b> input_port,           // Input data port
    tapa::mmap<_512b> FilterConf_port,      // Filter configuration
    tapa::mmap<_512b> output_port0_32b_8b,  // Output port 0
    tapa::mmap<_512b> output_port1_16b_8b,  // Output port 1
    tapa::mmap<_512b> output_port2_16b_8b,  // Output port 2
    tapa::mmap<_512b> output_port3_8b,      // Output port 3
    tapa::mmap<_512b> data_Idx,             // Index data
    tapa::mmap<_512b> output_port4_Track,   // Metadata output
    uint32_t data_count                     // Number of 512-bit words
);
```

## Key Components

### Memory Interface
- Uses TAPA async_mmap for memory access
- 512-bit AXI interface width
- Multiple output ports for parallel writing

### Stream Connections
- Extensive use of TAPA streams for inter-module communication
- Depth parameterization for buffering
- Support for both single and array streams

### Task Graph
The design uses TAPA's task graph to define a dataflow architecture with:
- Multiple parallel decompression units (16-way parallelism)
- Delta computation tree with carry propagation
- Filter processing with index tracking

## HLS Pragmas
- `#pragma HLS PIPELINE II=1` - Pipelining for throughput
- `#pragma HLS UNROLL` - Loop unrolling for parallelism
- `#pragma HLS BIND_STORAGE` - Memory binding directives
- `#pragma HLS BIND_OP` - Operation binding to DSP

## File Structure
- `orc_proc.cpp` - Main implementation
- `orc_proc.h` - Type definitions and constants
- `orcDecomp.h` - Decompression module
- `zlibTapa.h` - Zlib decompression
- `orc_decoder.h` - ORC decoding
- `orc_filter.h` - Data filtering

## Dependencies
- TAPA framework for FPGA task parallelism
- Xilinx ap_int for arbitrary precision integers
- Fixed code tables for Huffman decoding

## Performance Targets
- 2.4 GB/s throughput on Alveo U280
- 128x geomean speedup over CPU
- Tested with Vitis 2021.2

## Usage
```bash
make rtl_gen
cd orc_proc.xilinx_u280_xdma_201920_3.hw.xo.tapa/run-1/
bash orc_proc.xilinx_u280_xdma_201920_3.hw_generate_bitstream.sh
```

## References
- FPL 2024: FORC: A High-Throughput Streaming FPGA Accelerator for Optimized Row Columnar File Decoders
- https://www.sfu.ca/~zhenman/files/C38-FPL2024-FORC.pdf
