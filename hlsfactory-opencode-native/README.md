# HLSFactory OpenCode Native

Automated HLS design extraction using OpenCode inside Docker.

## Prerequisites

- [Docker](https://www.docker.com/) installed and running
- An `OPENROUTER_API_KEY` set in the project root `.env` file (`../`)

## Usage

The recommended entry point is `run.py` (cross-platform, requires only Python 3):

```bash
python run.py <SOURCE_REPO> [-o OUTPUT_DIR] [--dry-run]
```

- `SOURCE_REPO` — URL or local path to the HLS repository to process
- `-o` / `--output-dir` — host directory for results (default: `./output`)
- `--dry-run` — print Docker commands without executing them

### Examples

```bash
# Process a GitHub repo
python run.py https://github.com/user/hls-designs

# Specify output directory
python run.py https://github.com/user/hls-designs -o ./results

# Preview commands without running
python run.py https://github.com/user/hls-designs --dry-run
```

### Alternative shell scripts

Platform-specific shell scripts are also available:

```bash
# Linux/macOS
./run.sh <SOURCE_REPO> [OUTPUT_DIR]

# Windows
run.bat <SOURCE_REPO> [OUTPUT_DIR]
```

## Pipeline

The orchestrator agent runs these stages in a single OpenCode session:

1. Clone and analyze the repository to identify all HLS designs
2. Extract each design into its own folder
3. Find or generate testbenches
4. Generate documentation
5. Compile with clang++ and fix errors
6. Generate TCL synthesis scripts
7. Create the final manifest

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
