#!/usr/bin/env bash
# GitHub REST API through the host proxy (token, rate limits, cache and budget live there).
# usage: gh_api.sh "repos/owner/name/commits?per_page=30"
set -euo pipefail
: "${PROXY_URL:?PROXY_URL not set}" "${PROXY_SECRET:?PROXY_SECRET not set}"
curl -sS --max-time 300 \
  -H "X-Proxy-Secret: $PROXY_SECRET" -H "X-Paper-Id: ${PAPER_ID:-unknown}" \
  "$PROXY_URL/github/${1#/}"
