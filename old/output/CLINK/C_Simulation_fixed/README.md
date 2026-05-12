# C Simulation (fixed‑point) Design

This design provides a reference fixed‑point implementation of the LSTM inference used in the CLINK project.

## Sources
- `lstm_inference.c`
- Header files: `weight_1.hpp`, `weight_2.hpp`, `input.hpp`, `fixed_infer_result_1.hpp`, `fixed_infer_result_2.hpp`

## Description
The code implements LSTM inference using 16‑bit fixed‑point arithmetic, suitable for CPU simulation and functional verification.

## Usage
Compile the source files with a C/C++ compiler (e.g., `clang++`) using the provided stubs if needed.
