# AWS Deflate Compression Engine

## Overview
This is the AWS FPGA implementation of a high-throughput deflate compression engine.

## Components
- **deflate_top.v**: Top-level module for the deflate compression
- **huffman_translation.v**: Huffman coding implementation
- **hash_match.v**: Hash-based string matching for compression
- **calc_match_len.v**: Match length calculation
- **pack_out_data.v**: Output data packing
- **register_compression.v**: Compression register management

## Features
- High-throughput compression using LZ77 and Huffman coding
- Optimized for AWS FPGA infrastructure
- Configurable compression levels

## Key Files
- All main RTL files are verilog/sv files
- ROM initialization files (.mif) for supported Huffman codes

## Build Requirements
- Xilinx Vivado Design Suite
- AWS FPGA Development Kit
