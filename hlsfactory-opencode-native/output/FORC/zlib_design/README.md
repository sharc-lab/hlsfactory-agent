# Zlib Decompression Design (zlibTapa)

## Overview
The zlibTapa module implements zlib/inflate decompression using the TAPA framework. It includes Huffman decoding, LZ77 decompression, and parallel processing units for high-throughput decompression.

## Key Components

### 1. LZ Processing Unit
Implements LZ77 parsing to extract literals, match lengths, and offsets.

```cpp
template <class SIZE_DT = uint8_t>
void lzProcessingUnit(
    tapa::istream<ap_uint<17> >& inStream,
    tapa::istream<bool>& inDEos,
    tapa::ostream<bool>& outLMDEos,
    tapa::ostream<bool>& outLUDEos,
    tapa::ostream<SIZE_DT>& litLenStream,
    tapa::ostream<SIZE_DT>& matchLenStream,
    tapa::ostream<ap_uint<16> >& offsetStream,
    tapa::ostream<ap_uint<10> >& outStream
);
```

**Features:**
- Maximum literal length of 128 bytes
- Dynamic literal grouping
- End-of-stream handling

### 2. LZ Literal Upsizer
Groups literals into parallel bytes for efficient processing.

```cpp
template <int PARALLEL_BYTES>
void lzLiteralUpsizer(
    tapa::istream<ap_uint<10> >& inStream,
    tapa::istream<bool>& inDEos,
    tapa::ostream<ap_uint<PARALLEL_BYTES * 8> >& litStream
);
```

### 3. LZ Multi-Byte Decompress
Main decompression engine with history buffer.

```cpp
template <int PARALLEL_BYTES, int HISTORY_SIZE, class SIZE_DT = uint8_t, class SIZE_OFFSET = ap_uint<16> >
void lzMultiByteDecompress(
    tapa::istream<SIZE_DT>& litlenStream,
    tapa::istream<ap_uint<PARALLEL_BYTES * 8> >& litStream,
    tapa::istream<SIZE_OFFSET>& offsetStream,
    tapa::istream<SIZE_DT>& matchlenStream,
    tapa::istream<bool>& inDEos,
    tapa::ostream<ap_uint<(PARALLEL_BYTES * 8) + PARALLEL_BYTES> >& outStream
);
```

**Parameters:**
- `PARALLEL_BYTES` - Number of bytes processed in parallel (default: 8)
- `HISTORY_SIZE` - History buffer size in bytes (default: 32KB)

### 4. Huffman Decoder
Implements both fixed and dynamic Huffman decoding.

```cpp
template <eHuffmanType DECODER = FULL>
void huffmanDecoder(
    tapa::istream<ap_uint<16> >& inStream,
    tapa::istream<bool>& inEos,
    tapa::istream<bool>& inDEos,
    tapa::ostream<bool>& outDEos,
    tapa::ostream<ap_uint<17> >& outStream
);
```

**Decoder Types:**
- `FIXED` - Fixed Huffman codes only
- `DYNAMIC` - Dynamic Huffman codes only
- `FULL` - Both fixed and dynamic

## Data Flow

```
Compressed Data -> Huffman Decoder -> LZ Processing Unit -> 
Literal Upsizer -> Multi-Byte Decompress -> Decompressed Data
```

## Memory Architecture

### History Buffer
```cpp
ap_uint<c_parallelBit> ramHistory[2][c_ramHistSize];
#pragma HLS BIND_STORAGE variable = ramHistory type = RAM_2P impl = URAM
```

- Dual-port UltraRAM implementation
- Supports both low-offset and high-offset matches
- Partitioned for parallel access

### Register History
```cpp
ap_uint<c_parallelBit> regHistory[2][c_regHistSize];
#pragma HLS ARRAY_PARTITION variable = regHistory dim = 0 complete
```

- Fully partitioned for register-based access
- Handles low-offset matches efficiently

## Huffman Tables

The design includes pre-computed fixed Huffman code tables:
- `fixed_litml_op` - Operation codes
- `fixed_litml_bits` - Bit lengths
- `fixed_litml_val` - Values

Dynamic Huffman tables are generated at runtime using `code_generator_array_dyn`.

## Decompression States

```cpp
enum lzDecompressStates { 
    READ_LIT_LEN,    // Read literal length
    WRITE_LITERAL,   // Write literal data
    READ_OFFSET,     // Read match offset
    READ_MATCH,      // Read match data
    NO_OP           // No operation (timing optimization)
};
```

## Performance

- 8-byte parallel processing
- 32KB history buffer
- Pipeline II=1 for continuous processing
- Supports both stored and compressed blocks

## Block Types

| Type | Description |
|------|-------------|
| 0 | Stored (uncompressed) block |
| 1 | Fixed Huffman compressed block |
| 2 | Dynamic Huffman compressed block |
| 3 | Reserved/Error |

## Limitations

- No GZIP header processing (raw deflate only)
- No checksum verification
- Limited to 32KB window size

## References

Based on zlib compression standard (RFC 1950, RFC 1951)
