"""Run the repo-discovery agent over a folder of paper PDFs (one agent per paper).

    python exp/paper_filter/run_agent.py [--pdfs DIR] [--sheet XLSX] [--run-dir DIR]
        [--jobs N] [--limit N] [--only SUBSTR] [--dry-run] [--force]

Use `.venv/bin/python` with PYTHONPATH=. (plain `uv run` does not work here; see CLAUDE.md).
Needs: OPENROUTER_API_KEY in .env (not for --dry-run); GITHUB_TOKEN in .env or the
environment (strongly recommended). Build the image first: docker_images/paper_filter/build.sh.

Outputs under --run-dir: papers/*.json, handoff_repos.txt, handoff.csv, review_list.csv,
summary.csv, stats.json, registry.sqlite, proxy_calls.jsonl, work/<paper>/ (incl. transcripts).
Finished papers (status ok) are skipped on rerun; failed ones are retried.
"""

import argparse
import json
import secrets
from pathlib import Path

from dotenv import dotenv_values
from joblib import Parallel, delayed

from hlsfactory_agent.paper_filter.github import load_token
from hlsfactory_agent.paper_filter.manifest import discover_papers
from hlsfactory_agent.paper_filter.outputs import build_outputs
from hlsfactory_agent.paper_filter.prompt import DEFAULT_BUDGETS
from hlsfactory_agent.paper_filter.proxy import ProxyConfig, ProxyServer, bridge_address
from hlsfactory_agent.paper_filter.registry import Registry, load_seeds
from hlsfactory_agent.paper_filter.runner import RunConfig, run_one

DIR_CURRENT = Path(__file__).resolve().parent
DIR_ROOT = DIR_CURRENT.parent.parent
SEEDS = DIR_ROOT / "hlsfactory_agent" / "paper_filter" / "seed_repos.txt"
MODEL = "deepseek/deepseek-v4-flash"  # same as exp/run_base/run.py

ap = argparse.ArgumentParser()
ap.add_argument("--pdfs", type=Path, default=DIR_CURRENT / "pdfs")
ap.add_argument("--sheet", type=Path, default=DIR_ROOT / "hls_source_indexing/hls_papers_merged.xlsx")
ap.add_argument("--run-dir", type=Path, default=DIR_CURRENT / "runs" / "run1")
ap.add_argument("--model", default=MODEL)
ap.add_argument("--image", default="paper-filter-agent")
ap.add_argument("--jobs", type=int, default=16, help="parallel agents (tune in the pilot)")
ap.add_argument("--timeout", type=int, default=1800, help="seconds per paper")
ap.add_argument("--limit", type=int, help="only the first N papers (pilot)")
ap.add_argument("--only", help="only papers whose id contains this text")
ap.add_argument("--dry-run", action="store_true", help="prepare work folders, no containers")
ap.add_argument("--force", action="store_true", help="redo papers that already finished")
args = ap.parse_args()

args.run_dir.mkdir(parents=True, exist_ok=True)
env = dotenv_values(DIR_ROOT / ".env")
api_key = env.get("OPENROUTER_API_KEY")
if not args.dry_run and not api_key:
    raise SystemExit("OPENROUTER_API_KEY missing from .env (or use --dry-run)")
github_token = load_token(DIR_ROOT / ".env")
if github_token is None:
    print("warning: no GITHUB_TOKEN; code search needs one and limits are very low without")

sheet = args.sheet if args.sheet.exists() else None
if sheet is None:
    print(f"note: spreadsheet {args.sheet} not found; titles come from filenames")
papers = discover_papers(args.pdfs, sheet)
if args.only:
    papers = [p for p in papers if args.only in p.paper_id]
unmatched = [p.paper_id for p in papers if not p.matched]
print(f"{len(papers)} PDFs; {len(unmatched)} not matched to a spreadsheet row")

registry = Registry(args.run_dir / "registry.sqlite")
registry.seed(load_seeds(SEEDS))

done = set() if args.force else {p.paper_id for p in papers if registry.paper_done(p.paper_id)}
todo = [p for p in papers if p.paper_id not in done]
if args.limit:
    todo = todo[: args.limit]
print(f"{len(done)} already done, running {len(todo)}")

proxy = None
cfg = RunConfig(
    model=args.model, api_key=api_key, image=args.image, github_token=github_token,
    timeout_s=args.timeout, budgets=DEFAULT_BUDGETS, dry_run=args.dry_run,
)  # fmt: skip
if not args.dry_run:
    secret = secrets.token_hex(16)
    proxy = ProxyServer(
        ProxyConfig(secret=secret, token=github_token, log_path=args.run_dir / "proxy_calls.jsonl"),
        registry,
        host=bridge_address(),
    ).start()
    cfg.proxy_url = f"http://host.docker.internal:{proxy.port}"
    cfg.proxy_secret = secret
    print(f"proxy listening on port {proxy.port}")

snapshot = registry.snapshot()
all_results: list[dict] = []
try:
    gen = Parallel(n_jobs=args.jobs, return_as="generator_unordered")(
        delayed(run_one)(p, cfg, args.run_dir, snapshot) for p in todo
    )
    for final in gen:  # single writer: merge here, in the parent
        if final["status"] == "ok":
            registry.merge_result(final, str(args.run_dir / "papers" / f"{final['paper_id']}.json"))
        all_results.append(final)
        print(f"{final['paper_id'][:50]}: {final['status']} {final.get('outcome') or ''}")
finally:
    if proxy:
        proxy.shutdown()

# Outputs cover every paper with a result on disk (this run and earlier ones).
results = [json.loads(f.read_text()) for f in sorted((args.run_dir / "papers").glob("*.json"))]
stats = build_outputs(args.run_dir, results)
print(json.dumps(stats, indent=2))
