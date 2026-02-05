#!/bin/bash
# Start OpenCode interactively in the HLSFactory Native directory
# Usage: ./start.sh

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$SCRIPT_DIR"

echo "=============================================="
echo "HLSFactory OpenCode Native - Interactive Mode"
echo "=============================================="
echo ""
echo "Available agents:"
echo "  @hlsfactory-orchestrator - Run the full pipeline"
echo "  @hlsfactory-file-writer  - Batch file operations"
echo ""
echo "Example commands:"
echo "  @hlsfactory-orchestrator Process /path/to/repo to ./output"
echo "  Analyze the HLS designs in /path/to/repo"
echo ""
echo "=============================================="
echo ""

# Start OpenCode interactively
opencode
