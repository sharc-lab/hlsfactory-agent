# aeloss_pull HLS Design (Tuned)

## Description
This is an MLIR-based HLS design for an adaptive error loss pull computation kernel.
The design implements a nested loop computation with optimized scheduling.

## Top-Level Function
- Name: `main`
- Design: `aeloss_pull`
- Return Type: `f64` (double precision float)

## Framework
HECTOR - MLIR-based hardware synthesis framework.

## Source Files
- `design.mlir` - MLIR TOR dialect representation

## Resource Configuration
- Clock: 6.0ns
- Resource file: resource_dynamatic.json

## Note
MLIR intermediate representation for hardware synthesis.
