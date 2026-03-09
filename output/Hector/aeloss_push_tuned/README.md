# aeloss_push HLS Design (Tuned)

## Description
This is an MLIR-based HLS design for an adaptive error loss push computation kernel.
The design implements a nested loop computation with dataflow pipelining.

## Top-Level Function
- Name: `main`
- Design: `aeloss_push`
- Return Type: `f64` (double precision float)

## Framework
This design uses HECTOR (Hardware synthesis methodologies for Engineering Tools via Compiler Optimizations and Rewriting) - an MLIR-based hardware synthesis framework.

## Source Files
- `design.mlir` - MLIR TOR dialect representation of the design

## Resource Configuration
- Clock: 6.0ns
- Resource file: resource_dynamatic.json

## Design Details
The design performs:
- Accesses 1024-element arrays (`arg2` as f64, `arg3` as i32)
- Implements nested loops with pipeline II=1
- Contains conditional operations and floating-point arithmetic
- Uses control flow with scf.if operations for conditional execution

## Note
This is an MLIR intermediate representation. The design is meant to be processed through the HECTOR toolchain (--scf-to-tor, --schedule-tor, --split-schedule, --generate-hec --dump-chisel) to generate Chisel RTL output.
