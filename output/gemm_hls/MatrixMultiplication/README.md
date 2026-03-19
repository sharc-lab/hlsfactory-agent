# MatrixMultiplication HLS Design

This design implements a matrix multiplication kernel using HLS. The top-level function is
`MatrixMultiplicationKernel`, defined in `Top.cpp`. Supporting functions for memory handling
and computation are located in `Memory.cpp` and `Compute.cpp`. Header files in the `include/`
directory provide type definitions and utility functions.

The provided testbench (`testbench.cpp`) simulates the kernel behavior.
