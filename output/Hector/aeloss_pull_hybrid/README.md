# aeloss_pull HLS Design (Hybrid Scheduling)

## Description
This is a hybrid-scheduled version of the aeloss_pull kernel.
Optimized for dynamic workloads with adaptive scheduling.

## Top-Level Function
- Name: `main`
- Design: `aeloss_pull`
- Return Type: `f64`
- Submodules: submodule1, submodule2, submodule3 for modular execution

## Framework
HECTOR with hybrid scheduling support.

## Source Files
- `design.mlir` - MLIR TOR dialect representation

## Note
Uses hybrid scheduling with multiple submodules.
