#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: ./organize.sh <github-repo-url>"
    exit 1
fi

REPO_URL=$1

if [ -z "$ANTHROPIC_API_KEY" ]; then
    echo "Error: ANTHROPIC_API_KEY not set"
    echo "Run: export ANTHROPIC_API_KEY=your-key"
    exit 1
fi

echo "Building and running..."
docker-compose build
docker-compose run --rm hls-organizer python agent_script.py "$REPO_URL"

echo "Done! Check ./output/"