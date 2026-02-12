# HLSFactory OpenCode Native

Automated HLS design extraction using OpenCode natively.

## Prerequisites

- [OpenCode](https://opencode.ai/) installed (`npm i -g opencode-ai@latest`)
- `clang++` available in PATH
- Bash shell (Git Bash on Windows, or native on Linux/macOS)

## Usage

```bash
# Linux/macOS
./run.sh /path/to/source/repo [/path/to/output]

# Windows
run.bat C:\path\to\source\repo [C:\path\to\output]
```

If no output directory is given, it defaults to `<source_repo>/_hlsfactory_output_native`.

## Pipeline

The orchestrator agent runs these stages in a single OpenCode session:

1. Analyze the repository and identify all HLS designs
2. Extract each design into its own folder
3. Find or generate testbenches
4. Generate documentation
5. Compile with clang++ and fix errors
6. Generate TCL synthesis scripts and a final manifest

## Output Structure

```
output/
├── designs.json
├── manifest.json
└── design_name/
    ├── kernel.cpp
    ├── kernel.h
    ├── tb.cpp
    ├── description.md
    ├── dataset_hls.tcl
    └── compile_log.txt
```

## Stubs

The `stubs/` directory contains stub implementations of Xilinx HLS headers (`ap_int.h`, `ap_fixed.h`, `hls_stream.h`, `ap_axi_sdata.h`) for compilation testing with standard compilers.
