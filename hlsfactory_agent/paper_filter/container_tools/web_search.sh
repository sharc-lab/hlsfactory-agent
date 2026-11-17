#!/usr/bin/env bash
# Web search fallback. May answer 501 "no web search provider configured": then skip it.
# usage: web_search.sh "<query>"
set -euo pipefail
: "${PROXY_URL:?PROXY_URL not set}" "${PROXY_SECRET:?PROXY_SECRET not set}"
q=$(jq -rn --arg q "$1" '$q|@uri')
curl -sS --max-time 120 \
  -H "X-Proxy-Secret: $PROXY_SECRET" -H "X-Paper-Id: ${PAPER_ID:-unknown}" \
  "$PROXY_URL/web/search?q=$q"
