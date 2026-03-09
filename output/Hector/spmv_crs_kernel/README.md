# SpMV (Sparse Matrix-Vector) CRS Kernel

## Description
Sparse Matrix-Vector multiplication kernel using Compressed Row Storage (CRS/CSC) format.
Optimized for sparse linear algebra operations common in scientific computing.

## Top-Level Function
- Name: `main`
- Design: `spmv`
- Return Type: void

## Framework
HECTOR - MLIR-based hardware synthesis framework.

## Source Files
- `design.mlir` - MLIR TOR dialect representation

## Algorithm
Uses Compressed Row Storage format for efficient sparse matrix representation.
