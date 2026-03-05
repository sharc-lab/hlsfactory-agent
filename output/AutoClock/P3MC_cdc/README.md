# P3MC_cdc HLS Design

## Overview
This is an HLS design from the AutoClock benchmark suite.

## Files
- **top.cpp**: Main kernel implementation
- **top.h**: Header file with type definitions and function declarations
- **testbench.cpp**: Testbench for simulation
- **run_hls.tcl**: TCL script for HLS synthesis

## Design Description
This design implements a matrix multiplication kernel with systolic array architecture.
It uses HLS dataflow optimization with multiple processing elements (PEs) and
streaming interfaces between modules.

## Key Features
- Uses ap_int types for data packing
- Implements hls::stream for data communication
- DATAFLOW pragma for task-level parallelism
- Multiple clock domains support
- Memory interface via AXI4

## Top Function

```
```c
```
void kernel_sep(A_t16 *A, B_t16 *B, C_t16 *C_temp1, C_t16 *C_temp2)
```

```
```c
```
void kernel_m1_1(A_t16 *A, B_t16 *B, C_t16 *C)
```

```
```c
```
void kernel_m1_2(A_t16 *A, B_t16 *B, C_t16 *C)
```

## Data Types

```
```c
```
typedef float A_t1;
```

```
```c
```
typedef float B_t1;
```

```
```c
```
typedef float C_t1;
```

```
```c
```
typedef ap_uint<512> A_t16;
```

```
```c
```
typedef ap_uint<64> A_t2;
```

```
```c
```
typedef ap_uint<512> B_t16;
```

```
```c
```
typedef ap_uint<64> B_t2;
```

```
```c
```
typedef ap_uint<512> C_t16;
```

```
```c
```
typedef ap_uint<128> C_t4;
```

## Synthesis
Run the TCL script in Vivado HLS:
```bash
vivado_hls -f run_hls.tcl
```

## Compilation
The design can be compiled with clang for syntax checking:
```bash
clang -c -I/workspace/stubs top.cpp
```

