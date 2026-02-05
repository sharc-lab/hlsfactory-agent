# HLSFactory OpenCode Native

Automated HLS design extraction using OpenCode natively, without Python orchestration.

## Overview

This version runs OpenCode directly with a single comprehensive prompt. OpenCode handles the entire pipeline autonomously.

## Prerequisites

- [OpenCode](https://opencode.ai/) installed and configured with an AI provider
- `clang++` available in PATH (for compilation verification)
- Bash shell (Git Bash on Windows, or native on Linux/macOS)

## Installation

```bash
# Install OpenCode
npm i -g opencode-ai@latest

# Verify installation
opencode --version
```

## Usage

### Quick Start

1. Edit `config.yaml` to set your source repository and output paths
2. Run OpenCode from this directory:

```bash
cd hlsfactory-opencode-native
opencode run --agent hlsfactory-orchestrator "Process the HLS repository at SOURCE_PATH and output to OUTPUT_PATH"
```

Or use the provided run script:

```bash
./run.sh /path/to/source/repo ./output
```

### Manual Execution

You can also run OpenCode interactively:

```bash
cd hlsfactory-opencode-native
opencode
```

Then in the OpenCode session, use:
```
@hlsfactory-orchestrator Process the repository
```

## How It Works

The `hlsfactory-orchestrator` agent handles the complete pipeline:

1. **Analysis**: Scans the source repository to identify HLS designs
2. **Extraction**: Copies and reorganizes each design into a clean structure
3. **Testbench**: Finds or generates testbenches for each design
4. **Description**: Generates documentation for each kernel
5. **Compilation**: Compiles with clang++ and fixes errors

All stages run within a single OpenCode session, allowing the agent to maintain context and make intelligent decisions throughout.

## Output Structure

```
output/
├── designs.json           # Analysis results
├── manifest.json          # Summary of processed designs
├── design_1/
│   ├── kernel.cpp         # Main kernel implementation
│   ├── kernel.h           # Header (if applicable)
│   ├── tb.cpp             # Testbench
│   ├── description.md     # Documentation
│   ├── dataset_hls.tcl    # Vitis HLS synthesis script
│   └── compile_log.txt    # Compilation results
└── design_2/
    └── ...
```

## Configuration

Edit `config.yaml`:

```yaml
source_repo: /path/to/hls/designs
output_dir: ./output

compilation:
  compiler: clang++
  flags: ["-c", "-std=c++17", "-Wall"]
  max_fix_attempts: 3
```

## Agents

- **hlsfactory-orchestrator**: Main orchestration agent that runs the complete pipeline
- **hlsfactory-file-writer**: Specialized agent for batch file operations

## Troubleshooting

### OpenCode not finding agents
Ensure you're running from this directory, or set up OpenCode's agent paths.

### Compilation failures
Check `output/{design}/compile_log.txt` for details. HLS stub headers are available in `stubs/`.
