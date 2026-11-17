#!/usr/bin/env bash
# GitHub repository search (30/min shared by all agents; per-paper budget enforced).
# usage: gh_search_repos.sh "<query>" [per_page]
# query examples: 'InTAR in:name', 'inter-task accelerator in:readme language:C++'
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
q=$(jq -rn --arg q "$1" '$q|@uri')
"$here/gh_api.sh" "search/repositories?q=$q&per_page=${2:-10}" | jq 'if .items then
  {total_count, items: [.items[] | {full_name, description, language, stargazers_count,
    fork, archived, created_at, pushed_at}]} else . end'
