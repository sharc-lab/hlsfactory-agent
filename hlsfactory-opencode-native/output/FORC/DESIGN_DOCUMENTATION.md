# FORC Design Documentation

## Overview

FORC (FPGA ORC File Processor) is a high-throughput streaming-based FPGA accelerator designed for processing ORC (Optimized Row Columnar) files. It implements a complete dataflow architecture on AMD/Xilinx Alveo U280 FPGA.

## Architecture

### High-Level Dataflow

```
Host Memory → Data Reader → ORC Decompress → ORC Decoder → ORC Filter → Data Writer → Host Memory
```

### Module Breakdown

#### 1. Data Reader (`mmap2s`)
- Reads 512-bit chunks from DDR memory
- Implements async_mmap interface for efficient memory access
- Generates tapa::streams for downstream processing

#### 2. ORC Decompression (`orcDecomp.h`)
**Function: `DecompHead`**
- Parses ORC compression headers (24-bit format: 1-bit orig flag + 23-bit size)
- Routes compressed/uncompressed data to appropriate streams
- Handles block-based data organization

**Function: `decompSender`**
- Distributes compressed data to parallel decompressors
- Manages flow control with start/stop signals

#### 3. Zlib Decompression (`zlibTapa.h`)
**Module: `huffmanDecoder`**
- Supports FIXED, DYNAMIC, and FULL huffman decoding modes
- Implements RFC 1951 inflate algorithm
- Fixed blocks use pre-computed Huffman tables (fixed_codes.hpp)
- Dynamic blocks build code trees from block preamble

**Module: `lzProcessingUnit`**
- Parses LZ77 tokens (literals, match lengths, distances)
- Outputs 17-bit tokens: [16:1] = data, [0] = end-of-block

**Module: `lzLiteralUpsizer`**
- Buffers and packs literals into 64-bit (8-byte) chunks

**Module: `lzMultiByteDecompress`**
- Implements LZ77 decompression with history buffer
- Uses URAM for large history (32KB)
- Handles match copying with parallel byte processing

#### 4. ORC Decoder (`orc_decoder.h`)
**Module: `load`**
- Parses ORC column encoding headers
- Supports 4 encoding types:
  - **SR (Short Repeat)**: Simple value repetition
  - **DI (Direct)**: Bit-packed direct values
  - **PA (Patched Base)**: Base value + bit-packed deltas + patches
  - **DE (Delta)**: Delta encoding with base value

**State Machine:**
- HEADER_ST → Decode encoding type header
- SR_STATE → Short repeat processing
- DI_STATE → Direct value extraction
- PA_STATE → Patched base processing
- DE_STATE → Delta decoding
- ASSIGN_DATA → Prepare data for streaming

**Module: `data_Sender`**
- Distributes decoded data to parallel processing lanes (4 streams)
- Handles Short Repeat values separately
- Generates metadata for each data block

**Module: `compSR`**
- Processes Short Repeat encoded values
- Expands repeated values into output stream

**Delta Decoding Modules:**
- `compute_delta`: Calculates base values and deltas
- `delta_sumNC_in`, `delta_sumNC_all`: Non-carry sum operations
- `delta_sum_2out`, `delta_sum_1out`, `delta_sum_0out`: Carry propagation
- `delta_Fsum`: Final sum with carry aggregation

**Module: `PA_data_proc`**
- Processes Patched Base encoding
- Adds patch values to base+delta results

**Module: `Data_Aligner`**
- Synchronizes multiple data streams (DE, DI, PA, SR)
- Generates alignment metadata for downstream modules

**Module: `brDecData`**
- Removes bubbles (empty cycles) from data streams
- Implements ready/valid handshaking

#### 5. ORC Filter (`orc_filter.h`)
**Module: `FilterData`**
- Applies configurable filter conditions
- Supports two modes:
  - **Range Filter**: Left and right range with comparison operators (<, <=, ==, !=, >, >=)
  - **Index Filter**: Index-based filtering with bit-mask

**Configuration Format (512-bit filter config):**
- [7:0]: Index flag (0=range, 1=index)
- [15:8]: Range flag
- [23:16]: Right range operator (RROP)
- [31:24]: Left range operator (LROP)
- [63:32]: Right range value (RR)
- [95:64]: Left range value (LR)

**Bubble Removal Modules:**
- `br_s0e`: Merges 2×512-bit streams into 1×1024-bit
- `br_s1`: Merges 2×1024-bit streams into 1×2048-bit
- `br_WrTracker`: Tracks filtered row counts and generates write addresses

#### 6. Data Writer (`store_all`)
- Writes filtered results to 4 parallel output ports
- Supports different data widths:
  - Port 0: 32-bit data, 8-bit per element
  - Port 1: 16-bit data, 8-bit per element
  - Port 2: 16-bit data, 8-bit per element
  - Port 3: 8-bit data
- Uses async_mmap for efficient DDR writes

## Data Types

### Width Definitions
- `_512b`: ap_uint<512> - Standard AXI stream width
- `_1024b`: ap_uint<1024> - Extended buffer width
- `_2048b`: ap_uint<2048> - Decoder output width
- `_256b`: ap_uint<256> - Metadata stream width
- `_128b`: ap_uint<128> - Final metadata width
- `_72b`: ap_uint<72> - Zlib decompressor output (64-bit data + 8-bit strobe)
- `_320b`: ap_uint<320> - Short Repeat data width

### Constants
- `DMUL = 16`: Number of parallel decompression units
- `PEs = 16`: Number of processing elements for filtering
- `DECOMP_DEPTH = 4072`: Decompressor stream depth

## Encoding Types

### ORC Encoding Type Constants
```cpp
SR = 0      // Short Repeat
DIRECT = 1  // Direct encoding
PATCHED = 2 // Patched Base encoding
DELTA = 3   // Delta encoding
```

### Filter Operators
```cpp
FOP_LT = 1   // Less Than
FOP_LTE = 2  // Less Than or Equal
FOP_EQ = 3   // Equal
FOP_NE = 4   // Not Equal
FOP_GT = 5   // Greater Than
FOP_GTE = 6  // Greater Than or Equal
```

## Performance Characteristics

- **Target Frequency**: 300 MHz (3.33ns period)
- **Target Throughput**: 2.4 GB/s
- **Memory Interface**: 512-bit AXI4-Stream to DDR4
- **Parallelism**: 16-way parallel decompression
- **Processing Units**: 16 PEs for filtering

## Synthesis Directives

### HLS Pragmas Used
```cpp
#pragma HLS PIPELINE II=1      // Initiation interval of 1
#pragma HLS UNROLL             // Fully unroll loops
#pragma HLS ARRAY_PARTITION    // Partition arrays for parallel access
#pragma HLS BIND_STORAGE       // Bind arrays to specific memory types (URAM, BRAM)
#pragma HLS BIND_OP            // Bind operators to specific DSP implementations
#pragma HLS dependence         // Specify data dependencies
```

### Memory Resources
- **URAM**: Used for LZ77 history buffer (32KB)
- **BRAM**: Used for code tables and small FIFOs
- **LUTRAM**: Used for registers and small buffers

## Integration with TAPA

The design uses TAPA's task-based programming model:
- `tapa::task()`: Creates a task graph
- `.invoke()`: Connects function tasks with streams
- `tapa::detach`: Runs task independently
- `tapa::join`: Waits for task completion

## Testing

The design includes two host applications:
1. **Single-Stripe-Single-Column** (`orc_proc_host_1S1C`): For testing individual columns
2. **Full Dataflow** (`orc_proc_host`): For complete dataflow integration

## References

- [FPL 2024] FORC: A High-Throughput Streaming FPGA Accelerator for Optimized Row Columnar File Decoders in Big Data Engines
- TAPA Documentation: https://tapa.readthedocs.io/
- ORC Specification: https://orc.apache.org/specification/
- Zlib RFC 1950/1951
