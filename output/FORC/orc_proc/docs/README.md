# orc_proc - HLS Design Documentation

## Overview

FORC: FPGA-based ORC Filter - Accelerates Apache ORC file decompression and filtering on FPGA using TAPA framework

## Design Information

- **Name**: orc_proc
- **Top Function**: orc_proc
- **Framework**: TAPA
- **Target Platform**: Xilinx U280

## Interfaces

| Interface | Type | Width | Direction | Description |
|-----------|------|-------|-----------|-------------|
| input_port | mmap | 512 | in | |
| FilterConf_port | mmap | 512 | inout | |
| output_port0_32b_8b | mmap | 512 | out | |
| output_port1_16b_8b | mmap | 512 | out | |
| output_port2_16b_8b | mmap | 512 | out | |
| output_port3_8b | mmap | 512 | out | |
| data_Idx | mmap | 512 | inout | |
| output_port4_Track | mmap | 512 | out | |
| data_count | scalar | 32 | in | |

## File Structure

```
orc_proc/
├── src/          # Source files
├── tb/           # Testbench files
├── scripts/      # TCL synthesis scripts
└── docs/         # Documentation
```

## Source Files

- `orc_proc.cpp`
- `orc_proc.h`
- `orcDecomp.h`
- `orc_filter.h`
- `orc_decoder.h`
- `zlibTapa.h`
- `fixed_codes.hpp`

## Compilation

### Standalone Compilation (clang++)

```bash
cd src
clang++ -c -std=c++17 -I./stubs orc_proc.cpp -o orc_proc.o
```

### Vitis HLS Synthesis

```bash
vitis_hls -f scripts/orc_proc_synth.tcl
```

## Architecture

The orc_proc design implements a complete ORC file processing pipeline:

1. **Data Reading**: Reads compressed ORC data from memory
2. **Orc Decompression**: Decompresses zlib-compressed ORC data
3. **Orc Decoding**: Decodes ORC columnar format
4. **Data Filtering**: Applies filter conditions to decoded data
5. **Data Writing**: Writes filtered results back to memory

## Notes

- This design uses the TAPA (Task-Parallel Programming for Accelerators) framework
- Requires Xilinx Vitis HLS 2022.1 or later
- Target platform: Xilinx Alveo U280

## References

- Repository: https://github.com/SFU-HiAccel/FORC
- TAPA Framework: https://github.com/UCLA-VAST/tapa
