# VAE - Vae-Intrra-Kernel.Cpp Variant

## Overview
This is an HLS design from the InTAR repository implementing vae.

## Design Details
- **Name**: vae
- **Variant**: vae-intrra-kernel.cpp
- **Source**: /workspace/repo/benchmark/vae/vae-intrra-kernel.cpp

## Implementation Notes
This design uses TAPA (Task-level Parallelism for HLS) framework.
The original code has been adapted for standalone Vitis HLS synthesis.

## Top Function
The top-level function for synthesis is identified in the source code.

## Interface
- Uses AXI memory mapped interfaces for data transfer
- Includes cycle counting for performance measurement

## Synthesis
To synthesize this design:
```bash
vitis_hls -f run_hls.tcl
```

## Files
- `kernel.cpp` - Main kernel implementation
- `testbench.cpp` - Testbench for verification
- `run_hls.tcl` - Vitis HLS synthesis script

## Notes
This design was auto-extracted and may require manual tuning for optimal performance.
