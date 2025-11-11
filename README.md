# HLSFactory Agent

Analyze HLS/C++ source trees with an LLM and emit structured design metadata.

This agent:
- discovers top-level HLS kernels and their dependent files
- extracts synthesizable sub-components per design
- writes results to a destination directory for downstream tooling

All semantic decisions are delegated to an OpenRouter-backed LLM. The code uses the `llm_openrouter` plugin to talk to the OpenRouter API.

## What it does (two-pass analysis)

- **Pass 1 – Discover designs**:
  - Sends a filtered subset of repository files to the LLM with instructions to find top-level kernels and list their dependent source files.
  - Produces `designs.json` containing an array of discovered designs: kernel name + selected file paths.
- **Pass 2 – Extract sub-components**:
  - For each design from Pass 1, sends only that design’s files to the LLM.
  - Produces one JSON file per design under `subcomponents/` containing a list of sub-component function names.

## Token and size controls

To keep prompts small and predictable, the agent:
- **Includes only these file extensions**: `.c, .cpp, .cc, .cxx, .h, .hpp, .hxx`
- **Skips directories** anywhere in the path: `.git, .svn, .hg, build, cmake-build, out, dist, venv, .venv, node_modules, third_party, external, .cache`
- **Caps per-file size** with `--max_file_bytes` (default: 100,000 bytes)
- **Caps total prompt size** with `--max_prompt_chars` (default: 150,000 characters)

If adding another file would exceed the total budget, the agent stops adding more files. Adjust the limits via CLI flags as needed for your model/context window.

## Requirements

- Python environment with [`uv`](https://github.com/astral-sh/uv) recommended
- OpenRouter API key in the environment:
  - `OPENROUTER_API_KEY=<your_key>`

Dependencies are declared in `pyproject.toml` (notably `llm-openrouter`).

## Quickstart (run the agent directly)

```bash
UV_CACHE_DIR=.uv-cache uv run python hlsfactory_agent.py /path/to/src \
  --dst_dir ./HLSDesigns \
  --model_id__extract_top_level_designs deepseek/deepseek-v3.2-exp \
  --model_id__break_down_hls_design deepseek/deepseek-v3.2-exp
```

Useful flags to control prompt size:
- `--max_prompt_chars 120000`
- `--max_file_bytes 65536`

Example with limits:

```bash
UV_CACHE_DIR=.uv-cache uv run python hlsfactory_agent.py /path/to/src \
  --dst_dir ./HLSDesigns \
  --model_id__extract_top_level_designs deepseek/deepseek-v3.2-exp \
  --model_id__break_down_hls_design deepseek/deepseek-v3.2-exp \
  --max_prompt_chars 120000 \
  --max_file_bytes 65536
```

## GitHub ingestion helper

Fetch a GitHub repo, optionally target a branch/subdirectory, then run the agent on it:

```bash
UV_CACHE_DIR=.uv-cache uv run python scripts/fetch_from_github.py \
  https://github.com/user/repo \
  --github_branch main \
  --github_subdir path/inside/repo \
  --group_name my_repo_designs \
  --model_id__extract_top_level_designs deepseek/deepseek-v3.2-exp \
  --model_id__break_down_hls_design deepseek/deepseek-v3.2-exp
```

- Clones into `HLSSourceCode/<name or repo>/`
- Writes outputs to `HLSDesigns/<group_name or name>/`
- Omit `--github_subdir` to analyze the repo root; omit `--group_name` to default to the sources folder name.

### Interactive mode (optional)

```bash
UV_CACHE_DIR=.uv-cache uv run python scripts/fetch_from_github.py \
  https://github.com/user/repo \
  --interactive
```

Prompts:
- Name for HLSSourceCode folder [`<repo>`]
- Name for HLSDesigns group [`<answer above>`]
- Branch to fetch [auto-detect]

You can always skip prompts by supplying flags, for example:

```bash
UV_CACHE_DIR=.uv-cache uv run python scripts/fetch_from_github.py \
  https://github.com/user/repo \
  --name MySources --group_name MyDesigns \
  --github_branch master --github_subdir fpga
```

## Outputs

Under the `--dst_dir` (default `./HLSDesigns`):

- `designs.json`: list of designs detected by the LLM.
  - Each entry includes:
    - `kernel_name`: top-level kernel name
    - `source_files`: relative paths selected by the LLM that exist on disk
- `subcomponents/`: directory containing one JSON file per design:
  - Filename format: `<index>__<kernel_name_sanitized>.json`
  - JSON contains:
    - `sub_components`: list of function names considered synthesizable parts of the kernel

Example snippet of `designs.json`:

```json
{
  "designs": [
    {
      "kernel_name": "my_kernel",
      "source_files": [
        "src/top.cpp",
        "include/top.hpp",
        "src/utils.cpp"
      ]
    }
  ]
}
```

## How relevance is determined

- The agent assembles a prompt from files that pass the filters (extensions, skipped dirs, size limits).
- The LLM decides which files are actually part of each kernel and returns paths.
- The agent prunes any non-existent paths before writing outputs.
- The sub-components pass only includes files that the model selected for that design (and that remain under the per-file size limit).

## Model configuration

- The code constructs an OpenRouter client and uses the model IDs you provide:
  - `--model_id__extract_top_level_designs`
  - `--model_id__break_down_hls_design`
- Ensure `OPENROUTER_API_KEY` is set.
- You can point to any model available via OpenRouter; choose models with larger context windows for bigger codebases, or lower the prompt limits to fit within budget.

## Privacy considerations

- Only C/C++ sources and headers are sent to the LLM for analysis.
- Common vendor/build/cache directories are skipped entirely.
- Large files are excluded above the configured per-file cap.
- You control total and per-file budgets to reduce data sent externally.

## Developer notes

- Use `UV_CACHE_DIR=.uv-cache` when running commands so dependency caches stay within the workspace.
- `python -m compileall hlsfactory_agent.py` (via `uv run`) offers a quick syntax check.

## Smoke tests

If tests are added for schema validation, run them with:

```bash
UV_CACHE_DIR=.uv-cache uv run pytest -q
```
