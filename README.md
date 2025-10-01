# HLSFactory Agent

Utilities for turning ad-hoc HLS source trees into HLSFactory-compatible design
directories, with *all* semantic decisions delegated to a configured large
language model via the [`llm`](https://github.com/simonw/llm) Python package.

## Quickstart

Run the agent using `uv run` so that the bundled dependencies (namely
[`llm`](https://github.com/simonw/llm)) are available. Point the tool at a root
directory containing one or more HLS designs:

```bash
UV_CACHE_DIR=.uv-cache uv run python hlsfactory_agent.py /path/to/hls/repos \
  --output-manifest extracted_designs.json \
  --materialise ./dataset_out
```

The command writes a structured manifest (by default to stdout) and, if the
`--materialise` flag is provided, emits HLSFactory-ready design directories
containing:

- the original sources (copied or symlinked)
- auto-generated `dataset_hls.tcl` entry script and `top.txt`
- `design_manifest.json` with static and LLM-enriched metadata
- optional `kernel_description_generated.md` summarising the kernel

## LLM Integration

All detection, metadata extraction, and summarisation is performed by the LLM –
there are no hand-written heuristics. Configure the backend via the standard
`llm` configuration files or by passing `--llm-model`. Useful knobs:

- `--llm-temperature` (default `0.2`)
- `--llm-max-output-tokens`
- `--llm-max-functions`
- `--llm-code-chars`

Disable the LLM phase with `--disable-llm` if you prefer heuristic-only output.

Remember to set any API keys required by the selected model provider. When using
OpenRouter, install the companion plugin as declared in `pyproject.toml` and set
`OPENROUTER_API_KEY` in the environment.

## Developer Notes

- Use `UV_CACHE_DIR=.uv-cache` when running commands under sandboxed
  environments so dependency caches stay within the workspace.
- `python -m compileall hlsfactory_agent.py` (through `uv run`) offers a quick
  syntax check.
