# ORC Decoder Design (orc_decoder)

## Overview
The orc_decoder module implements the complete ORC (Optimized Row Columnar) file format decoder. It supports all ORC encoding types: DIRECT, DELTA, PATCHED BASE, and SHORT REPEAT.

## Supported Encodings

| Encoding | Description | Supported Bit Widths |
|----------|-------------|---------------------|
| DIRECT | Direct bit-packed values | 8, 16, 24, 32 |
| DELTA | Delta encoding with base | 8, 16, 24, 32 |
| PATCHED BASE | Base value with patches | 8, 16, 24, 32 |
| SHORT REPEAT | Run-length encoding | 8, 16, 24, 32 |

## Key Functions

### 1. load
Header parser that processes ORC compression headers and prepares data for decoding.

```cpp
template<int SIZE_ST = 1>
void load(
    tapa::istream<_512b>& data_in,
    tapa::ostream<_2048b>& outAll_Lstrm,
    tapa::ostream<_256b> &out_strmC_Track,
    tapa::ostream<_512b>& PA_DATA,
    tapa::ostream<uint32_t>& PA_metaDATA,
    tapa::ostream<bool>& D_strm_e
);
```

**Header Processing:**
- Extracts encoding type
- Parses run length
- Extracts bit width
- Computes metadata for downstream modules

### 2. data_Sender
Distributes decoded data to parallel processing units based on encoding type.

```cpp
template<int SIZE_ST = 1>
void data_Sender(
    tapa::istream<_2048b>& data_in,
    tapa::istream<_256b>& meta_in,
    tapa::istream<bool>& D_strm_e,
    tapa::ostreams<_512b,4>& outAll_Pstrm,
    tapa::ostream<uint64_t>& outSR_Pstrm,
    tapa::ostreams<_256b, 4>& meta_out,
    tapa::ostream<uint8_t>& SR_meta_out,
    tapa::ostream<uint64_t>& All_meta_out,
    tapa::ostream<ap_uint<24>>& Dec_type_Out,
    tapa::ostream<bool>& MD_strm_e
);
```

### 3. compSR
Short Repeat decoder with endianness conversion and zigzag decoding.

```cpp
template<int SIZE_ST = 1>
void compSR(
    tapa::istream<uint64_t>& outSR_Pstrm,
    tapa::istream<uint8_t>& meta_out,
    tapa::ostream<_320b>& SR_Dout
);
```

### 4. compute_delta
Main computation unit for DIRECT and DELTA encoding types.

- Performs endianness conversion
- Applies zigzag decoding for DIRECT
- Prepares delta values for accumulation

### 5. Delta Sum Modules
Tree-based delta accumulation with carry propagation:

- `delta_sumNC_in` - First stage, no carry in
- `delta_sumNC_all` - Intermediate stage, no carry out
- `delta_sum_2out` - Fork stage, two carry outputs
- `delta_sum_1out` - Standard stage, one carry output
- `delta_sum_0out` - Final stage, no carry output
- `delta_Fsum` - Final accumulation with carry distribution

### 6. PA Modules (Patched Base)

#### PA_meta_proc
Processes patch metadata to extract patch gaps and values.

#### PA_sum_out
Accumulates base values with patch values.

### 7. Meta_Aligner
Aligns metadata between patched and non-patched data streams.

### 8. Data_Aligner
Aligns decoded data from all encoding types into a unified output stream.

### 9. brDecData
Bubble remover that eliminates gaps in the decoded data stream.

## ORC Encoding Format

### Short Repeat Header
```
Bits 2:0 - Repeat count (actual = count + 3)
Bits 5:3 - Bit width (actual = width + 1)
Bits 7:6 - Encoding type (00)
```

### Direct Header
```
Bits 0:0 - First bit
Bits 15:8 - Run length
Bits 20:16 - Bit width index
Bits 23:21 - Padding
Bits 31:24 - First byte of data
```

### Delta Header
```
Bits 0:0 - First bit
Bits 15:8 - Run length
Bits 20:16 - Bit width index
Bits 23:21 - Padding
Bits 31:24 - First byte of base value
... - Variable length base and delta base
```

### Patched Base Header
```
Bits 0:0 - First bit
Bits 20:16 - Bit width index
Bits 23:21 - Base value width - 1
Bits 28:24 - Patch list length
Bits 31:29 - Patch gap width + 1
... - Variable length base value and patch data
```

## Bit Width Mapping

```cpp
const uint16_t NDelta_BitMap[32] = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 26, 28, 30, 32, 40, 48, 56, 64
};
```

## Processing Pipeline

```
Input Data -> load -> data_Sender -> 
    ├─> compSR (Short Repeat)
    ├─> compute_delta -> Delta Tree (Delta)
    └─> compute_delta -> PA Modules (Patched Base)
-> Data_Aligner -> brDecData -> Output
```

## Parallelism

- 4-way parallel delta computation
- 16-way parallel data processing (PEs)
- Tree-based carry propagation for delta accumulation

## Memory Usage

- History buffers: UltraRAM
- Lookup tables: BRAM
- Pipeline registers: Fully partitioned

## Limitations

- Supports run lengths multiple of 64
- Fixed to 8, 16, 24, 32 bit widths
- Requires bit-aligned data
