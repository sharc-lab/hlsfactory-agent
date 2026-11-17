#!/usr/bin/env bash
# GitHub code search (9/min shared by all agents; scarce, per-paper budget enforced).
# Best use: the exact paper title in quotes, which READMEs and CITATION files often carry.
# usage: gh_search_code.sh '"exact paper title"' [per_page]
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
q=$(jq -rn --arg q "$1" '$q|@uri')
"$here/gh_api.sh" "search/code?q=$q&per_page=${2:-10}" | jq 'if .items then
  {total_count, items: [.items[] | {repo: .repository.full_name, path, url: .html_url}]}
  else . end'
