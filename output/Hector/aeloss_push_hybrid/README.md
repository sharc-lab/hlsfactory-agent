# aeloss_push HLS Design (Hybrid Scheduling)

## Description
This is a hybrid-scheduled version of the aeloss_push kernel.
Combines static and dynamic scheduling techniques for improved performance.

## Top-Level Function
- Name: `main`
- Design: `aeloss`
- Return Type: `f64`

## Framework
HECTOR with hybrid scheduling support.

## Source Files
- `design.mlir` - MLIR TOR dialect representation

## Note
Uses hybrid scheduling (--dynamic-schedule flag).
