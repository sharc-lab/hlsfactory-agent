#!/bin/bash
# HLSFactory OpenCode Native - Run Script
# Usage: ./run.sh [SOURCE_REPO] [OUTPUT_DIR]

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default values from config or arguments
SOURCE_REPO="${1:-C:/Users/tanma/OneDrive/Documents/GitHub/S2CBench}"
OUTPUT_DIR="${2:-C:/Users/tanma/OneDrive/Documents/GitHub/S2CBench/_hlsfactory_output_native}"

echo "=============================================="
echo "HLSFactory OpenCode Native"
echo "=============================================="
echo "Source Repository: $SOURCE_REPO"
echo "Output Directory:  $OUTPUT_DIR"
echo "=============================================="
echo ""

# Check if opencode is available
if ! command -v opencode &> /dev/null; then
    echo "Error: opencode is not installed or not in PATH"
    echo "Install with: npm i -g opencode-ai@latest"
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Build the prompt
PROMPT="Process the HLS repository at '$SOURCE_REPO' and extract all HLS designs to '$OUTPUT_DIR'.

Execute the complete HLSFactory pipeline:
1. Analyze the repository and identify all HLS designs
2. Extract each design into its own folder
3. Find or generate testbenches for each design
4. Generate documentation for each design
5. Compile with clang++ and fix any errors
6. Generate TCL synthesis scripts
7. Create the final manifest

The HLS stub headers are available at: $SCRIPT_DIR/stubs

Work through each stage systematically and process ALL designs found."

# Run OpenCode with the orchestrator agent
echo "Starting OpenCode orchestrator..."
echo ""

cd "$SCRIPT_DIR"
opencode run --agent hlsfactory-orchestrator "$PROMPT"

echo ""
echo "=============================================="
echo "Pipeline complete!"
echo "Check $OUTPUT_DIR for results"
echo "=============================================="
