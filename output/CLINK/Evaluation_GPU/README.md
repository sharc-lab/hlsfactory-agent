# Evaluation_GPU (CUDA‑converted)

This design contains the HLS C++ version of the CUDA LSTM kernel converted by `cuda2hls.py`.

## Sources
- `kernel.cpp`
- `kernel.h`

The top‑level function is `lstm_task` (as reported by the converter).

## Testbench
A minimal testbench is provided in `testbench.cpp` which invokes `lstm_task` with dummy arguments.
