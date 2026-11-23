# Paper-filter agent: implementation plan

Decisions and reasoning live in `CLAUDE.md` (same directory, chunks 1-8). This file is the
build order.

## Build status

| Phase | Code | Verified |
|---|---|---|
| 1 Pre-pass cleanup | written (`extract.py`, `manifest.py`, `workfolder.py`, `github.py` size) | offline tests; dry run on the 3 sample PDFs; PDFs found recursively, subfolder = `group` |
| 2 Registry | written (`registry.py`, `seed_repos.txt`) | offline tests incl. concurrent merges |
| 3 HLS scanner | written (`scan.py`, stdlib, runs on host and in container) | offline tests; real repos (SEAL, InTAR) |
| 4 Evidence + scoring | written (`evidence.py`, `postprocess.py`) | offline tests |
| 5 Proxy + wrappers | written (`proxy.py`, `container_tools/*.sh`, `ExaProvider`) | offline tests; live via proxy with token (`/rate_limit`: core 5000/hr, search 30/min, code_search 10/min) and Exa (cached) |
| 6 Image + prompt | written (`docker_images/paper_filter/`, `prompt.py`) | image never built; prompt never run |
| 7 Orchestrator + outputs | written (`runner.py`, `outputs.py`, `exp/paper_filter/run_agent.py`) | dry run only; container path untested |
| 8 Server test, pilot, batch | not started | needs keys and the server |

Run order once the keys exist: build the image (`docker_images/paper_filter/build.sh`), check
`OPENROUTER_API_KEY`, `GITHUB_TOKEN` and `EXA_KEY` are in `.env` (all three are), run ONE paper
(`run_agent.py --only <id> --jobs 1`), read its transcript and `papers/<id>.json`, then
scale to the pilot.

## Goal

Given a folder of paper PDFs (one-off batch, ~2.5k papers from
`hls_source_indexing/hls_papers_merged.xlsx`), run one agent per paper, in parallel, to
find GitHub repos that may hold HLS designs and produce a precise list to hand to the
HLSFactory agent (`exp/run_base/run.py`). Precision over recall; GitHub only.

## Architecture

```
PDF folder
   |
   v
host pre-pass (pymupdf, URL regex, GitHub existence check)  -> work folder per paper
   |                                                             (pdf, page text, pre-pass json,
   v                                                              registry snapshot, scripts)
orchestrator (joblib, resume, controls)
   |  one container per paper (pi + deepseek/deepseek-v4-flash, poppler, python+pymupdf)
   |      |  curl
   |      v
   |   host proxy thread: token, rate limits, cache, logs ---> api.github.com / web search API
   v
agent result json (evidence with quotes) -> host merges into SQLite registry
   v
deterministic confidence scoring -> handoff_repos.txt, review_list.csv, summary.csv, stats.json
```

## Prerequisites (from the user)

| Needed | For phase | Status |
|---|---|---|
| GitHub token (read-only, no scopes) in `.env` as `GITHUB_TOKEN` | 5, 8 | done, verified 2026-11-21 |
| New OpenRouter key | 6, 8 | done, verified with deepseek-v4-flash 2026-11-21 |
| Web search API choice and key | 5 | Exa, `EXA_KEY` in `.env`, verified 2026-11-21 (~$0.007/query) |
| Mentor's answer on "HLS-rich" | 3 (interim: any one HLS kernel) | user will ask |
| PDF to spreadsheet row mapping | before 8 | parked; interim is filename stem |
| Server CPU/memory, Docker access for the user | 8 | unknown |

## Phases

Each phase should be testable on its own, and mostly on the dev Mac, before the next one.
Use `.venv/bin/python` with `PYTHONPATH=.`; never plain `uv run` (see CLAUDE.md, open items).

### Phase 1: pre-pass cleanup
- Done: `classify.py` removed, `extract.py` and `run_extract.py` no longer call an LLM.
- Add `size` (KB) to `check_repo` in `github.py`.
- Write per-page text files and a pre-pass JSON into a per-paper work folder.
- Scanned PDFs: record `needs_ocr` and continue to the search path using metadata.
- Paper id = PDF filename stem for now; title from the spreadsheet once the join exists.
- Tests: extend `tests/`; keep the 11 existing tests green.
- Done when: the 3 sample PDFs produce work folders; RPS-KNN shows zero candidates.

### Phase 2: registry (SQLite)
- Module for schema (`repos`, `paper_repos`, `papers`, `search_cache`), snapshot export
  (read-only JSON for agents), and merge of one paper's result.
- Seed with the 26 repos in `run.py` plus known suites (PolyBench, MachSuite, Rosetta,
  CHStone). Confirm exact repo ids when seeding.
- Tests: merge twice is idempotent; two processes merging do not lose rows.
- Done when: seed, snapshot and merge round-trip in tests.

### Phase 3: repo HLS scanner
- Standalone script usable inside the container and from the host: `git clone --depth 1`
  under the 300 MB cap (decided from the API `size`), else the git tree API with a
  `partial_scan` flag.
- Count files per marker (Vitis/Vivado, Intel, Catapult, TAPA, Dynamatic/LegUp, Tcl
  `set_top`/`csynth_design`) and candidate top functions. Interim rule: one HLS source
  file qualifies as `hls_found`; always record counts.
- Tests on known repos: `OswaldHe/InTAR` (HLS), `Microsoft/SEAL` (not HLS), one TAPA repo.
- Done when: classification is correct on those and counts are written to JSON.

### Phase 4: evidence schema and confidence scoring
- Define the evidence item (type, quote, page or source, strength) and the result JSON the
  agent must write.
- Pure function computing confidence (high/medium/low) from evidence per CLAUDE.md
  chunk 4. The model never self-reports confidence.
- Tests with fixtures from the 3 sample papers: InTAR (paper-supplied link, high), AutoNTT
  (paper-supplied link, SEAL must not become `own`), RPS-KNN (no evidence yet).
- Done when: rubric thresholds are covered by unit tests.

### Phase 5: host proxy and search wrappers
- Thread-run HTTP server: token injection, token-bucket limits (30/min repo search,
  9/min code search, 5,000/hr core), waits instead of failing, cache in SQLite, call log.
- Shell wrappers for the agent: repo search, code search, core API passthrough, and a
  web-search wrapper that is a stub until a provider is chosen.
- Listen on the Docker bridge address on Linux; `host.docker.internal` on the Mac.
- Tests against a fake upstream: limits hold under concurrent callers; the cache hits.
- Call `/rate_limit` with the real token to confirm the actual code-search limit.
- Done when: 64 concurrent fake callers get correct, throttled, cached responses.

### Phase 6: image and agent prompt
- `docker_images/paper_filter/Dockerfile` (standalone; poppler-utils, python3 + pymupdf,
  git, curl, jq, ripgrep, tree, node 24.15.0, pi) and a build script.
- Prompt: the search ladder, budgets, precision rules, the quote-and-page requirement, the
  third-party-demo rule (follow only what the paper builds on or evaluates), and the result
  JSON schema.
- Run a single paper by hand on the Mac with a real OpenRouter key.
- Done when: InTAR and AutoNTT resolve correctly and RPS-KNN either finds a defensible
  repo or returns `not_found` with a clear trail.

### Phase 7: orchestrator and outputs
- `exp/paper_filter/run_agent.py` (modelled on `run_base/run.py` and `core.py`): launch
  the proxy, build work folders, run containers in parallel, apply per-paper turn cap and
  timeout, CPU/memory limits, record failures, resume.
- Produce `papers/*.json`, `handoff_repos.txt`, `review_list.csv`, `summary.csv`,
  `stats.json`, registry and session transcripts (chunk 8). CSV/JSON only.
- Add the runs directory to `.gitignore`.
- Done when: a 5-paper run end to end produces every output file and a rerun skips the
  finished papers.

### Phase 8: server test, pilot, then full batch
- Test networking and uid handling on the lab server (container user, writable HOME,
  `--add-host`, proxy address).
- Pilot 30-50 papers, then review cost, time, failures, search budgets and the confidence
  rubric (with the mentor if possible), and tune.
- Full batch. Review `review_list.csv` by hand; pass `handoff_repos.txt` to HLSFactory.

## Out of scope (deliberately)
OCR; hosts other than GitHub; triggering HLSFactory automatically; Excel write-back.

## Biggest risks
- Wrong-repo matches wasting HLSFactory runs: mitigated by evidence-based scoring and
  high-only handoff.
- Code-search throughput (about 540/hr): mitigated by the ladder order, cache and registry.
- Web search quality is unknown: the design must work well without it.
- Parallelism limits (OpenRouter rate limits, server memory): found in the pilot.
- Lost/duplicated work on crash: per-paper result files plus resume.
