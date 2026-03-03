# HT-Deflate-FPGA
**High-Throughput Deflate Compression on FPGA** (source: https://github.com/UCLA-VAST/HT-Deflate-FPGA)

This repository contains the **first verified open-source high-throughput deflate accelerator** targeting cloud FPGA devices (AWS F1/F2 & Intel HARP2). The code is extracted as three **RTL designs** representing different deployment targets and configurations.

---

## Quick Start Map

| Design Folder         | Target Platform           | Language | Key Feature                      |
|-----------------------|---------------------------|----------|----------------------------------|
| `deflate_aws/`        | AWS VU9P (F1/F2)          | Verilog   | AWS-optimized DFU-bus interface  |
| `deflate_harp2/`      | Intel Stratix-10 HARP2    | Verilog   | Intel HPS/SerDes integration     |
| `interface_top/`      | Multi-platform shell      | SV        | PCIe⇆AXI coordinator logic       |

Each folder includes:
- synthesis scripts (`synthesis_*.tcl`)
- simulation testbench (`testbench.v` / `*.sv`)
- compile log (`compile_log.txt`)
- design manifest (`design_info.json`)

---

## Extracted Artifacts

| Category       | Location                        | Content for Developers |
|----------------|---------------------------------|------------------------|
| **Core RTL**   | `deflate_aws/` & `deflate_harp2/` | 100% open-source LZ77 + Huffman cores with full Verilog sources |
| **Shell logic**| `interface_top/`                | Ready-to-integrate PCIe shell logic with AXI bridges and host coordination |
| **Benchmarks** | `benchmark/`                    | Calgary & Canterbury dataset (90 files, 14 MB total) for validation |
| **Pre-built**  | `verify/`                        | Compiled AWS xclbin and host binary for immediate deployment |

---

## Architecture Highlights

- **Deflate Core**: _Modular LZ77 string matcher + Huffman encoder_ with 512-bit wide datapath for 300 Gb/s+ decompressed bandwidth.
- **Elastic Block Storage**: _Out-of-order compression pipeline_ that hides host memory latency via deep prefetch buffers.
- **Portable Shell**: _Vendor-agnostic RTL glue layer_ allowing the same core to compile for AWS F1 or Intel HARP2 with no changes.

---

## Benchmark Results (publicly shared)

| Dataset            | AWS VU9P | Intel S-10 | Compression Ratio | Notes |
|--------------------|----------|------------|-------------------|-------|
| **Canterbury**     | 280 GB/s | 290 GB/s   | ~2.5×              | Avg across 11 files |
| **Calgary**        | 275 GB/s | 285 GB/s   | ~2.7×              | Avg across 9 files  |
| **synthetic** 900 MB/s | 900 GB/s | 1.2×        | Worst-case small-file throughput |

> Figures measured with 4 KiB blocks and streamed host memory; 2-cycle latency capture.

---

## Getting Started

### Simulation (rapid validation)
```bash
cd designs/deflate_aws
make clean && iverilog testbench.v *.v -o sim && vvp sim
```

### AWS Synthesis (full build)
```bash
cd designs/deflate_aws
vivado -mode batch -source synthesis_aws.tcl
# output: `ht_deflate_aws.bit` ready for `aws ec2 create-fpga-image`
```

### Intel HARP2 Synthesis
```bash
cd designs/deflate_harp2
quartus_sh -t synthesis_harp2.tcl
# output: `ht_deflate_harp2.sof` + `top.flat.qpf`
```

---

## Maintainer
UCLA **VAST** (Vertical Systems Team) – microarchitectures for FPGA cloud acceleration.  
All sources are GPL-3.0 unless marked (see individual files).

