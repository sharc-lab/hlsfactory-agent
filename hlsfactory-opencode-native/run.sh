#!/bin/bash
# HLSFactory OpenCode Native - Run Script
# Usage: ./run.sh SOURCE_REPO [OUTPUT_DIR]

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Load .env file from project root
# Docker Compose also reads .env automatically for the YAML substitutions,
# but we load it here so we can use the variables in this script too.
if [ -f "$PROJECT_ROOT/.env" ]; then
    set -a
    source "$PROJECT_ROOT/.env"
    set +a
fi

# Require SOURCE_REPO argument, OUTPUT_DIR is optional (host path for results)
# ? means required and - means optional
SOURCE_REPO="${1:?Usage: ./run.sh SOURCE_REPO [OUTPUT_DIR]}"
OUTPUT_DIR="${2:-./output}"

echo "=============================================="
echo "HLSFactory OpenCode Native"
echo "=============================================="
echo "Source Repository: $SOURCE_REPO"
echo "Output Directory:  $OUTPUT_DIR"
echo "=============================================="
echo ""

# build the prompt — all paths are container paths, not host paths
PROMPT="Process the HLS repository at '$SOURCE_REPO' and extract all HLS designs to '/output'.

Execute the complete HLSFactory pipeline:
1. Clone the repository from '$SOURCE_REPO' into /workspace/repo
2. Analyze the repository and identify all HLS designs
3. Extract each design into its own folder under /output
4. Find or generate testbenches for each design
5. Generate documentation for each design
6. Compile with clang++ and fix any errors
7. Generate TCL synthesis scripts
8. Create the final manifest
9. Put all the design folders within a parent folder with the name of repository under /output

The HLS stub headers are available at: /workspace/stubs

Work through each stage systematically and process ALL designs found.

IMPORTANT: Start executing immediately. Do not just list steps — run the actual commands. Begin by cloning the repository now."

# Build the Docker image
echo "Building Docker image..."
docker-compose -f "$PROJECT_ROOT/docker-compose.yml" build

# Run the agent inside the container, no --rm so we can copy files out
echo "Starting OpenCode orchestrator..."
echo ""
docker-compose -f "$PROJECT_ROOT/docker-compose.yml" run \
    hls-organizer \
    opencode run -m openrouter/moonshotai/kimi-k2.5 "$PROMPT"

echo ""
echo "=============================================="
echo "Pipeline complete!"
echo "=============================================="

# Copy results from the container to the host
CONTAINER_ID=$(docker ps -lq)
mkdir -p "$OUTPUT_DIR"
docker cp "$CONTAINER_ID":/output/. "$OUTPUT_DIR"/
\
# Clean up the container
docker rm "$CONTAINER_ID"

echo "Results copied to $OUTPUT_DIR"
