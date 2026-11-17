#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
docker build -t paper-filter-agent .
