# FORC HLS Design Extraction Report

## Extraction Summary

**Repository:** https://github.com/SFU-HiAccel/FORC  
**Output Directory:** /output/FORC  
**Extraction Date:** 2024-02-23

## Repository Information

FORC (FPGA ORC File Processor) is a high-throughput streaming-based FPGA accelerator for ORC (Optimized Row Columnar) file processing. Published at FPL 2024.

### Key Features
- ORC Zlib decompression with Huffman decoding
- ORC encoding support (SR, DI, PA, DE)
- Data filtering with range and index-based predicates
- 2.4 GB/s target throughput on Alveo U280

## Extracted Designs

### 1. Main Kernel: orc_proc
- **Top Function:** `orc_proc`
- **Type:** TAPA Kernel
- **Description:** Main orchestration kernel implementing complete dataflow
- **Source Files:**
  - orc_proc.cpp (986 lines)
  - orc_proc.h (71 lines)
  - orcDecomp.h (620 lines)
  - zlibTapa.h (1199 lines)
  - orc_decoder.h (1280+ lines)
  - orc_filter.h (505 lines)
  - fixed_codes.hpp (80 lines)

### 2. ORC Decompression Module
- **Top Function:** `DecompHead`
- **Type:** HLS Module
- **Description:** Parses ORC headers and manages compressed/uncompressed data streams
- **Key Functions:**
  - `DecompHead`: Header parsing and routing
  - `decompSender`: Data distribution to parallel decompressors
  - `zlib_Sender`: Zlib data transmission
  - `decompData`: Decompressed data reassembly
  - `DataCombiner`: Compressed/uncompressed stream merging

### 3. Zlib Decompression Module
- **Top Function:** `huffmanDecoder`
- **Type:** HLS Module
- **Description:** RFC 1951 inflate algorithm implementation
- **Key Functions:**
  - `huffmanDecoder`: Huffman decoding (FIXED, DYNAMIC, FULL modes)
  - `lzProcessingUnit`: LZ77 token processing
  - `lzLiteralUpsizer`: Literal packing
  - `lzMultiByteDecompress`: LZ77 decompression with history buffer
  - `huffmanBytegen`: Byte-level Huffman decoding
  - `code_generator_array_dyn`: Dynamic Huffman table generation

### 4. ORC Decoder Module
- **Top Function:** `load`
- **Type:** HLS Module
- **Description:** ORC encoding type decoder
- **Key Functions:**
  - `load`: Header parsing and data extraction (supports SR, DI, PA, DE)
  - `data_Sender`: Data distribution to compute units
  - `compSR`: Short Repeat expansion
  - `compute_delta`: Delta computation
  - `PA_data_proc`: Patched Base processing
  - `delta_sum*`: Various delta summation operations

### 5. ORC Filter Module
- **Top Function:** `FilterData`
- **Type:** HLS Module
- **Description:** Configurable data filtering
- **Key Functions:**
  - `FilterData`: Range and index-based filtering
  - `br_s0e`: 512→1024 bit bubble removal
  - `br_s1`: 1024→2048 bit bubble removal
  - `br_WrTracker`: Write tracking and counting

## Architecture Overview

```
Host Memory
    ↓
[Data Reader] mmap2s
    ↓
[ORC Decompression] orcDecomp.h
    ↓
[Zlib Decompression] zlibTapa.h (16 parallel units)
    ↓
[ORC Decoder] orc_decoder.h
    ↓
[ORC Filter] orc_filter.h (16 PEs)
    ↓
[Data Writer] store_all
    ↓
Host Memory
```

## File Structure

```
/output/FORC/
├── README.md                    # Basic overview
├── DESIGN_DOCUMENTATION.md      # Detailed architecture documentation
├── manifest.json               # Machine-readable manifest
├── EXTRACTION_REPORT.md        # This file
├── run_hls.tcl                 # Main HLS synthesis script
├── kernel/
│   ├── orc_proc.cpp            # Main kernel
│   ├── orc_proc.h              # Type definitions and constants
│   ├── orcDecomp.h             # ORC decompression
│   ├── zlibTapa.h              # Zlib decompression
│   ├── orc_decoder.h           # ORC encoding decoder
│   ├── orc_filter.h            # Data filtering
│   ├── fixed_codes.hpp         # Static Huffman tables
│   ├── tapa.h                  # TAPA framework stub
│   ├── ap_int.h                # Arbitrary-precision integer stub
│   ├── ap_fixed.h              # Fixed-point stub
│   ├── hls_stream.h            # HLS stream stub
│   └── ap_axi_sdata.h          # AXI stream data stub
└── host/
    ├── orc_proc_host.cpp       # Full dataflow host
    ├── orc_proc_host.h
    ├── orc_proc_host_1S1C.cpp  # Single-stripe-single-column host
    ├── orc_proc_host_1S1C.h
    └── opencl_util.h           # OpenCL utilities
```

## Key Constants

### Data Types
- AXI_WIDTH = 512 bits (standard stream width)
- AXI_WIDTH_2X = 1024 bits
- AXI_WIDTH_4X = 2048 bits
- SR_DATAW = 320 bits (Short Repeat width)

### Parallelism
- DMUL = 16 (parallel decompression units)
- PEs = 16 (processing elements for filtering)
- DECOMP_DEPTH = 4072 (stream buffer depth)

### Encoding Types
- SR = 0 (Short Repeat)
- DIRECT = 1 (Direct encoding)
- PATCHED = 2 (Patched Base)
- DELTA = 3 (Delta encoding)

### Filter Operators
- FOP_LT = 1 (<)
- FOP_LTE = 2 (<=)
- FOP_EQ = 3 (==)
- FOP_NE = 4 (!=)
- FOP_GT = 5 (>)
- FOP_GTE = 6 (>=)

## Synthesis Information

### Target Platform
- **Device:** xcu280-fsvh2892-2L-e (Alveo U280)
- **Clock Period:** 3.0 ns (333 MHz)
- **Tool:** Vitis HLS 2021.2+
- **Framework:** TAPA 0.0.20221113.1

### Synthesis Directives Used
- `#pragma HLS PIPELINE II=1` - Single-cycle initiation
- `#pragma HLS UNROLL` - Loop unrolling
- `#pragma HLS ARRAY_PARTITION` - Memory partitioning
- `#pragma HLS BIND_STORAGE` - Memory type binding (URAM/BRAM)
- `#pragma HLS BIND_OP` - Operator binding to DSPs

### Resource Targets
- URAM: LZ77 history buffer (32KB per decompressor)
- BRAM: Huffman tables and FIFOs
- DSP: Delta computation multipliers

## Build Instructions

### HLS Synthesis
```bash
cd /output/FORC
vitis_hls -f run_hls.tcl
```

### Individual Module Synthesis
```bash
vitis_hls -f kernel/hls_config.tcl
# Then in vitis_hls shell:
synth_orcDecomp
synth_zlibTapa
synth_orc_decoder
synth_orc_filter
```

### TAPA Compilation (requires TAPA installation)
```bash
make rtl_gen
```

## Compilation Notes

The code is designed for Vitis HLS and uses several non-standard C++ features:
1. Arbitrary-precision integers (ap_int/ap_uint)
2. HLS pragmas for synthesis directives
3. TAPA task-based programming model
4. Streaming interfaces (hls::stream)

Standard C++ compilation requires stub headers which are provided in `/output/FORC/kernel/`.

## Testbench

Testbenches are available in the `host/` directory:
- `orc_proc_host_1S1C.cpp` - For testing single stripe/column
- `orc_proc_host.cpp` - For full dataflow testing

Note: Full testing requires TAPA framework and test data.

## References

- [FPL 2024] FORC: A High-Throughput Streaming FPGA Accelerator for Optimized Row Columnar File Decoders in Big Data Engines
- TAPA: https://github.com/UCLA-VAST/tapa
- ORC Specification: https://orc.apache.org/specification/

## Status

✅ Repository cloned  
✅ All HLS designs identified and extracted  
✅ Stubs created for HLS-specific types  
✅ TCL synthesis scripts generated  
✅ Documentation created  
✅ Manifest generated  

Note: Code is designed for Vitis HLS synthesis. Standard C++ compilation requires non-standard features (ap_int, HLS pragmas, TAPA).
