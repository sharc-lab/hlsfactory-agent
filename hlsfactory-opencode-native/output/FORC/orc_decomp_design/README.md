# ORC Decompression Design (orcDecomp)

## Overview
The orcDecomp module handles the decompression of ORC file data. It processes the ORC compression headers and routes data between compressed and uncompressed streams, supporting both compressed (zlib) and uncompressed data blocks.

## Key Functions

### DecompHead
Parses ORC compression headers and separates compressed from uncompressed data.

```cpp
template<int SIZE_ST = 1>
void DecompHead(
    tapa::istream<_512b>& inStream,
    tapa::ostream<_512b>& outCstm,
    tapa::ostream<_512b>& outUCstm,
    tapa::ostream<ap_uint<20>>& outCMDstm,
    tapa::ostream<ap_uint<20>>& outUCMDstm,
    uint32_t data_count
);
```

**Parameters:**
- `inStream` - Input compressed ORC data stream
- `outCstm` - Output compressed data stream (to decompression)
- `outUCstm` - Output uncompressed data stream
- `outCMDstm` - Metadata for compressed stream
- `outUCMDstm` - Metadata for uncompressed stream
- `data_count` - Number of 512-bit input words

**Header Format:**
- 24-bit header per block
- Bit 0: Original data flag (1=uncompressed, 0=compressed)
- Bits 23:1: Data size in bytes

### decompSender
Routes decompressed data to multiple parallel decompression units based on metadata.

```cpp
template<int SIZE_ST = 1>
void decompSender(
    tapa::istream<_512b>& inStream,
    tapa::istream<ap_uint<20>>& inMetaStream,
    tapa::ostream<uint8_t>& idxCombinerSend,
    tapa::ostreams<bool, DMUL>& outCEBoSstrm,
    tapa::ostreams<_512b, DMUL>& outCdata,
    tapa::ostreams<ap_uint<7>, DMUL>& outVdata
);
```

### zlib_Sender
Converts 512-bit data to 16-bit chunks for zlib processing.

### decompData
Combines decompressed 72-bit output words back into 512-bit words.

### DataCombiner
Merges compressed and uncompressed data streams with proper ordering.

## Metadata Format (20-bit)

| Bits | Field | Description |
|------|-------|-------------|
| 0 | orig_data | 1=uncompressed, 0=compressed |
| 1 | is_last_iteration | Last block flag |
| 2 | is_block_start | Block start flag |
| 3 | exit_loop | End of data flag |
| 19:4 | remainBits | Remaining bits in last word |

## Processing Flow

1. **Header Parsing** - Extract size and compression flags
2. **Data Routing** - Separate compressed/uncompressed
3. **Parallel Decompression** - Distribute to 16 decompression units
4. **Data Reassembly** - Combine outputs with correct ordering

## HLS Optimization

- Pipeline II=1 for maximum throughput
- Unroll factor for parallel decompression
- Buffer sizing for stream depth matching

## Constants

```cpp
#define DMUL 16              // Decompression parallelism factor
#define DECOMP_DEPTH 4072    // Decompression buffer depth
```

## Testing

The design supports single stripe and single column testing through TAPA's csim capability.

## Integration

This module is instantiated in the main `orc_proc` task graph as:
```cpp
.invoke(orcDecompHeadProc, outLstrm, outCstm, outUCstm, outCMDstm, outUCMDstm, data_count)
.invoke(orcDecompSender, outCstm, outCMDstm, idxCombinerSend, outCEBoSstrm, outCdata, outVdata)
.invoke<tapa::join, DMUL>(orcDecompTop, outCdata, outVdata, outCEBoSstrm, outDstm, outVDStm)
.invoke(orcDecompCombiner, outUCMDstm, idxCombinerSend, outUCstm, outDstm, outVDStm, outData, OutEoSdata)
```
