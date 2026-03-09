# GEMM (General Matrix Multiply) N-Cubed Kernel

## Description
Matrix multiplication kernel with N-cubed complexity.
Performs standard O(n^3) matrix multiplication optimized for hardware synthesis.

## Top-Level Function
- Name: `main`
- Design: `gemm`
- Return Type: void

## Framework
HECTOR - MLIR-based hardware synthesis framework.

## Source Files
- `design.mlir` - MLIR TOR dialect representation

## Algorithm
Triple-nested loop algorithm for general matrix multiply.
