# HLS 5‑Node LSTM Design

This design is part of the **CLINK** repository and implements a 5‑node LSTM accelerator using SystemC for Vivado/Vitis HLS synthesis.

## Sources
- All SystemC source files from the original repository (`/workspace/repo/HLS_5-Node/lstm/solution1/syn/systemc/`).
- The main top‑level implementation is in `lstm_n5_16s_16b.cpp` and corresponding header files.

## Description
The design synthesizes an LSTM kernel with 5 processing elements, each operating on 16‑bit data paths. It includes necessary HLS pragmas and package files for successful synthesis.

## Usage
- Compile the source files with `clang++` (syntax check only) using the provided stubs.
- Generate a Vivado/Vitis HLS TCL script (`synth.tcl`) to perform synthesis.
