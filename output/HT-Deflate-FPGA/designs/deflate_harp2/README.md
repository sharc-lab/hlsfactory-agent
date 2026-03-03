# HARP2 Deflate Compression Engine

## Overview
This is the HARP2 FPGA implementation of a high-throughput deflate compression engine, optimized for Intel HARP2 platform.

## Components
- **deflate_top.v**: Top-level module for the deflate compression
- **huffman_translation.v**: Huffman coding implementation
- **hash_match.v**: Hash-based string matching for compression
- **calc_match_len.v**: Match length calculation
- **pack_out_data.v**: Output data packing
- **register_compression.v**: Compression register management

## Features
- High-throughput compression using LZ77 and Huffman coding
- Optimized for Intel HARP2 FPGA infrastructure
- Configurable compression levels

## Differences from AWS version
- Uses Intel-specific naming conventions
- Different memory interface configurations
- Optimized for Intel Quartus toolchain

## Build Requirements
- Intel Quartus Prime Pro
- Intel HARP2 Development Kit
