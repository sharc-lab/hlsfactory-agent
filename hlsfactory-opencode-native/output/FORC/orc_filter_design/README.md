# ORC Filter Design (orc_filter)

## Overview
The orc_filter module implements data filtering functionality for ORC decoded data. It supports both range-based filtering and index-based filtering with parallel processing capabilities.

## Filter Modes

### 1. Range Filtering
Filters data based on left and right range conditions:
- Less Than (LT)
- Less Than or Equal (LTE)
- Equal (EQ)
- Not Equal (NE)
- Greater Than (GT)
- Greater Than or Equal (GTE)

### 2. Index Filtering
Filters data based on a pre-computed index mask, selecting specific rows.

## Key Functions

### 1. FilterData
Main filter processing unit with 16 parallel processing elements.

```cpp
template<int SIZE_ST = 1>
void FilterData(
    tapa::istream<_512b>& inConfStrm,
    tapa::istream<_512b>& inAllStrm,
    tapa::istream<uint16_t>& inFilIdx,
    tapa::istream<bool>& eFd_strm,
    tapa::ostream<_512b>& outFilterStrm,
    tapa::ostream<uint16_t>& outIdxStrm,
    tapa::ostream<uint8_t>& dataCnt,
    tapa::ostream<bool>& eFDout_strm,
    int F_idx
);
```

**Parameters:**
- `inConfStrm` - Filter configuration (512-bit)
- `inAllStrm` - Input data stream (16 x 32-bit values)
- `inFilIdx` - Index stream for index filtering
- `eFd_strm` - End-of-stream signal
- `outFilterStrm` - Filtered output data
- `outIdxStrm` - Output index tracking
- `dataCnt` - Count of filtered elements (0-16)
- `eFDout_strm` - Output end-of-stream
- `F_idx` - Filter unit index

**Configuration Format (512-bit):**

| Bits | Field | Description |
|------|-------|-------------|
| 7:0 | idx_flag | 0=range mode, 1=index mode |
| 15:8 | range_flag | Range configuration |
| 23:16 | RROP | Right range operation |
| 31:24 | LROP | Left range operation |
| 63:32 | RR | Right range value |
| 95:64 | LR | Left range value |

**Filter Operations:**
```cpp
const uint8_t FOP_LT  = 1;  // Less Than
const uint8_t FOP_LTE = 2;  // Less Than or Equal
const uint8_t FOP_EQ  = 3;  // Equal
const uint8_t FOP_NE  = 4;  // Not Equal
const uint8_t FOP_GT  = 5;  // Greater Than
const uint8_t FOP_GTE = 6;  // Greater Than or Equal
```

### 2. br_s0e
Bubble remover stage 0 - combines two 512-bit streams into one 1024-bit stream.

```cpp
template<int SIZE_ST = 1>
void br_s0e(
    tapa::istreams<_512b, 2>& inFilStrm,
    tapa::istreams<bool, 2>& eFDin_strm,
    tapa::istreams<uint8_t, 2>& inData_count,
    tapa::ostream<_1024b>& outFilStrm,
    tapa::ostream<uint8_t>& outData_count,
    tapa::ostream<bool>& eBR0out_strm
);
```

### 3. br_s1
Bubble remover stage 1 - combines two 1024-bit streams into one 2048-bit stream.

```cpp
template<int SIZE_ST = 1>
void br_s1(
    tapa::istreams<_1024b, 2>& inFilStrm1,
    tapa::istreams<bool, 2>& eBR0in_strm,
    tapa::istreams<uint8_t, 2>& inData_count1,
    tapa::ostream<_2048b>& outFilStrm1,
    tapa::ostream<bool>& eBR1out_strm,
    tapa::ostream<uint8_t>& outData_count1
);
```

### 4. br_WrTracker
Final bubble removal stage that packs filtered data and tracks write counts.

```cpp
template<int SIZE_ST = 1>
void br_WrTracker(
    tapa::istream<_2048b>& inFilStrm2,
    tapa::istream<bool>& inEstrm,
    tapa::istream<uint8_t>& inData_count2,
    tapa::ostream<ap_uint<24>>& store_writeCount,
    tapa::ostream<uint32_t>& numCnt,
    tapa::ostreams<_512b, 4>& Dout
);
```

## Processing Flow

```
Input Data (16 x 32-bit) -> FilterData (4 parallel units) ->
br_s0e (2:1 merge) -> br_s1 (2:1 merge) -> br_WrTracker ->
4 x 512-bit Output Streams
```

## Parallel Architecture

- 4 parallel FilterData units (each processing 16 elements)
- 16 parallel processing elements (PEs) per FilterData unit
- 2-stage bubble removal tree
- Final write tracking across 4 output ports

## Data Packing

### Stage 0 (br_s0e)
```
Input:  2 streams × 512-bit, counts c0, c1
Output: 1 stream × 1024-bit, count c0+c1
```

### Stage 1 (br_s1)
```
Input:  2 streams × 1024-bit, counts c0, c1
Output: 1 stream × 2048-bit, count c0+c1
```

### Final Stage (br_WrTracker)
```
Input:  1 stream × 2048-bit
Output: 4 streams × 512-bit (packed, no gaps)
```

## Index Tracking

Each FilterData unit generates a 16-bit index mask:
- Bit i = 1: Element i passed the filter
- Bit i = 0: Element i was filtered out

## HLS Optimizations

- Full unrolling for parallel comparison (16 PEs)
- Pipeline II=1 for continuous processing
- Switch-case optimization for operation selection
- Shift-based packing for bubble removal

## Example Usage

### Range Filter
```cpp
// Filter for values between 0 and 100
filter_conf.range(7, 0) = 0;      // idx_flag = 0 (range mode)
filter_conf.range(15, 8) = 1;     // range_flag = 1
filter_conf.range(23, 16) = 5;    // RROP = GTE (>=)
filter_conf.range(31, 24) = 5;    // LROP = GTE (>=)
filter_conf.range(63, 32) = 100;  // RR = 100
filter_conf.range(95, 64) = 0;    // LR = 0
```

### Index Filter
```cpp
// Select specific rows
filter_conf.range(7, 0) = 1;      // idx_flag = 1 (index mode)
// Index mask: 0x00FF = select first 8 rows
```

## Performance

- 64 elements processed per cycle (4 units × 16 PEs)
- Sub-cycle latency for comparison operations
- No bubbles in output stream after br_WrTracker

## Integration

The filter module is integrated into the main orc_proc task graph:

```cpp
// Filter Data
.invoke(orcFilterData, FilterConfigData[0], br_Strms[0], FilterIdxData[0], e_strm[0], outFilterStrm[0], outIdxStrm[0], dataCntFD[0], eFD_strm[0], 0)
...

// Bubble Removing
.invoke(filterBR1, outFilterStrm, eFD_strm, dataCntFD, outBR0_Strm[0], outData_count[0], eBR0out_strm[0])
.invoke(filterBR2, outBR0_Strm, eBR0out_strm, outData_count, outBR1_Strm, eBR1out_strm, outBR1Data_count)
.invoke(filterBR3, outBR1_Strm, eBR1out_strm, outBR1Data_count, store_writeCount, PartialnumCnt, BR_WR_OUT)
```
