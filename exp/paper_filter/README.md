# Paper filter

Finds the public GitHub repo behind each HLS paper and checks whether it contains HLS designs,
producing a list of repos to hand to HLSFactory (`exp/run_base/run.py`). Input: a folder of
paper PDFs, joined to `hls_source_indexing/hls_papers_merged.xlsx` by title.

Library code: `hlsfactory_agent/paper_filter/`. This folder: the run script, PDFs (`pdfs/`,
gitignored) and run outputs (`runs/`, gitignored).

## Usage

Everything goes through one script, run from the **repo root**:

```bash
# 1. a quick free check: reads the PDFs, no Docker, no API calls
.venv/bin/python exp/paper_filter/run_agent.py --dry-run --run-dir exp/paper_filter/runs/check

# 2. the real run, in the background (survives logging out; no tmux needed)
.venv/bin/python exp/paper_filter/run_agent.py --detach --run-dir exp/paper_filter/runs/myrun

# 3. check on it any time
.venv/bin/python exp/paper_filter/run_agent.py --status --run-dir exp/paper_filter/runs/myrun

# 4. when it is done, open the report
less exp/paper_filter/runs/myrun/report.md
```

Leave out `--detach` to run in the foreground with progress printed to the terminal.

| Option | Meaning |
|---|---|
| `--pdfs DIR` | PDF folder (default `exp/paper_filter/pdfs/`), searched recursively. A subfolder name, e.g. `with_repo/`, becomes the paper's `group` in the outputs. |
| `--run-dir DIR` | Where outputs go. Rerunning into the same folder skips finished papers and retries failed ones. |
| `--only TEXT` | Only papers whose id (PDF filename) contains TEXT. |
| `--limit N` | Only the first N papers. |
| `--force` | Redo papers that already finished. |
| `--jobs N` | Agents in parallel (default 6). |
| `--timeout S` | Seconds per paper (default 1800). |
| `--detach` / `--status` | Run in the background / show progress, as above. |

To stop a detached run: `kill <pid>` (printed by `--detach` and `--status`). Finished papers are kept;
run the same command again to resume.

### Before the first run

The script checks all of this itself and stops with a fix if something is missing:

- **Python packages** in `.venv` (do not use `uv sync`/`uv run` on chao-srv1; it hangs on an SSH-only
  dependency):
  `UV_CACHE_DIR=/export/hdd/scratch/jzhou652/uv-cache ~/.local/bin/uv pip install --python .venv/bin/python docker joblib pandas openpyxl pymupdf python-dotenv requests pytest psutil`
- **`.env`** in the repo root with `OPENROUTER_API_KEY` (required), `GITHUB_TOKEN` (strongly
  recommended) and `EXA_KEY` (optional web search). Not needed for `--dry-run`.
- **Docker** access, and the `paper-filter-agent` image. If the image is missing, the script builds it
  (`docker_images/paper_filter/build.sh`, a few minutes, once).

Tests: `PYTHONPATH=. .venv/bin/python -m pytest -q tests`

## Outputs (in `--run-dir`)

| File | What it is |
|---|---|
| `report.md` | **Start here.** Results by group, handoff list with evidence, review list, every paper with a link to its agent transcript, API usage. |
| `handoff_repos.txt` | `owner/name` per line, for `repo_urls` in `exp/run_base/run.py`. |
| `handoff.csv` | The same repos with provenance, confidence, HLS dialects, flags and source paper(s). |
| `review_list.csv` | Repos that need a human look, with the reason and the evidence quotes. |
| `summary.csv` | One row per paper. |
| `stats.json` | Counts per status and outcome, per group. |
| `papers/<id>.json` | Full record per paper: each repo with evidence, HLS scan and flags. |
| `work/<id>/` | Prompt, agent output, PDF pre-pass (`run_area/prepass.json`), transcript (`run_area/.pi/sessions/*.html`). |
| `run.log` | Log of a `--detach` run. |
| `proxy_calls.jsonl`, `registry.sqlite` | Every GitHub/Exa call; repos already seen (internal). |

### Reading the results

Each repo gets a **provenance** and an **outcome**:

- Provenance: `own_in_paper` (linked in the paper), `own_found_by_search` (authors' repo, found by
  search), `third_party_demo` (a tool or benchmark the paper used).
- Outcome: `hls_found` (has HLS designs), `repo_no_hls`, `needs_review`, `already_known` (one of the 26
  repos HLSFactory already processed, `hlsfactory_agent/paper_filter/seed_repos.txt`), `link_dead`.
  A paper with no repo is `not_found`.

Only `hls_found` repos with **high** confidence are handed off. Confidence is computed from the
evidence the agent quotes (paper link, README title, commit authors, ...), not from the model's own
opinion. Quotes from the paper are checked against the PDF text.

Common flags: `third_party`, `hls_via_tcl` (directives in TCL scripts, no `#pragma HLS`),
`hls_maybe_in_archive` (no C/C++ but an archive that may hold the designs), `weak_hls_markers_only`
(only `ap_int.h`-style includes), `tapa_only` / `tapa_dominant` (HLSFactory skips TAPA), `fork`.

## How it works

1. **Pre-pass (host):** extract the PDF text, find GitHub links, flag links that only appear in the
   references (cited, not the authors' code), check each link with the GitHub API.
2. **Agent (Docker, one per paper):** `pi` with `deepseek/deepseek-v4-flash` via OpenRouter reads the
   paper and the pre-pass, then searches when needed: GitHub repo search, code search, then Exa web
   search. It reports each repo with quoted evidence and runs the HLS scanner on it.
3. **Proxy (host):** every GitHub/Exa call from a container goes through it. It adds the token,
   enforces rate limits and per-paper budgets (repo search 12, code search 4, web 3, other GitHub 80),
   and caches results. A `429` with `budget_exhausted` means the paper used its budget, not a GitHub
   limit.
4. **Post-process (host):** verify quotes, score confidence, classify the HLS scan, decide handoff,
   write the outputs and `report.md`.

## Test results (2026-11-23)

Run `exp/paper_filter/runs/groups1/`: 31 PDFs labeled by hand only as "has a repo" (3) or "no repo"
(28). All 31 finished (`ok`), with 6 agents in parallel, in about 20 minutes. Exa cost about $0.40;
LLM cost is not tracked yet.

| group | papers | handed off | review | already known | repo, no HLS | nothing found |
|---|---|---|---|---|---|---|
| with_repo | 3 | 1 | 0 | 2 | 0 | 0 |
| without_repo | 28 | 6 | 2 | 1 | 6 | 13 |

- **with_repo:** all 3 found the right repo. `MasLiang/AutoClock` was handed off; InTAR and AutoNTT
  are already in HLSFactory (`already_known`).
- **Found repos the human label missed** (authors' own code, found by search, handed off):
  `jiahaogai/LLM-for-HLS`, `zhaijiaqi/AccelES`, `Lucaz97/Agentic-HLS`. Each is backed by strong
  evidence (README title or system name, commit author = paper author, matching benchmarks).
  `aferikoglou/CollectiveHLS` is also the right repo but went to review (weak HLS markers only).
- **Third-party repos handed off** (tools or benchmarks the paper used, not the authors' release):
  `EPFL-LAP/dynamatic`, `jianyicheng/HLS-benchmarks`, `UT-LCA/ML4Accel-Dataset`, `breagen/MachSuite`.
  They qualify under the current rule (any HLS kernel counts) and are flagged `third_party`.
- **Not yet checked by hand:** the third-party handoffs and the 13 `not_found` papers (only
  RPS-KNN was checked: a manual GitHub search also finds nothing). No wrong repo has turned up in
  the repos that were checked.

Fixed during the test: the HLS scanner missed TCL-based Vitis projects (MachSuite) and archived
designs (HLSyn); `--force` reruns treated a paper's own earlier finds as already known and skipped the
scan.

## Known limitations

- LLM token cost is not recorded per paper.
- Only GitHub repos are handed off (GitLab, Zenodo, personal sites are recorded only).
- Scanned PDFs without a text layer are flagged `needs_ocr` and searched by title/authors only.
- The agent sometimes spends GitHub calls guessing usernames (many 404s).
- Open question (mentor): should third-party repos need more than one HLS kernel to be handed off?
