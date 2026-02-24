# FORC - FPGA ORC File Processor

A high-throughput streaming-based FPGA accelerator for processing ORC (Optimized Row Columnar) files.

## Design Overview

FORC is an HLS-based design using TAPA (Task Parallel Programming for FPGA Accelerators) framework. It supports:
- ORC Zlib decompression
- ORC file decoding (DIRECT, DELTA, PATCHED BASE, SHORT REPEAT encoders)
- Data filtering

## Architecture

The design consists of several functional units:

1. **Data Reader** (`mmap2s`) - Reads data from memory
2. **ORC Decompression** - Zlib decompression with Huffman decoding
3. **ORC Decoder** - Parses ORC file format and decodes various encodings
4. **ORC Filter** - Applies filtering conditions on decoded data
5. **Data Writer** - Writes results back to memory

## Key Modules

- `orc_proc.cpp` - Main kernel orchestrating all operations
- `orcDecomp.h` - ORC header parsing and data extraction
- `zlibTapa.h` - Zlib decompression with LZ77 algorithm
- `orc_decoder.h` - ORC encoding decoder (SR, DI, PA, DE)
- `orc_filter.h` - Range and index-based filtering
- `fixed_codes.hpp` - Static Huffman tables for fixed blocks

## Build Requirements

- Vitis/Vivado 2021.2+
- TAPA framework (version 0.0.20221113.1)
- Alveo U280 FPGA

## Synthesis

Use the provided TCL scripts for synthesis:
```bash
vitis_hls -f run_hls.tcl
```

## Performance

- Target throughput: 2.4 GB/s
- Achieves 128x geomean speedup over CPU

## References

[FPL 2024] FORC: A High-Throughput Streaming FPGA Accelerator for Optimized Row Columnar File Decoders in Big Data Engines
