"""Paper filter: find GitHub repos with HLS designs for a folder of paper PDFs.

The one entry point. Run from the repo root (see exp/paper_filter/README.md):

    .venv/bin/python exp/paper_filter/run_agent.py --run-dir exp/paper_filter/runs/NAME [options]

    --detach         run in the background (survives logout; no tmux needed), log in <run-dir>/run.log
    --status         show progress of a run (done / running / failed) and exit
    --dry-run        PDF pre-pass and work folders only: no Docker, no API keys, no cost
    --only TEXT      only papers whose id contains TEXT;  --limit N  only the first N papers
    --force          redo papers that already finished (default: skip them, retry failed ones)
    --pdfs DIR       PDF folder, searched recursively; a subfolder name (e.g. with_repo/)
                     becomes the paper's `group` in the outputs

Before starting it checks Python packages, .env keys, Docker access and the agent image
(building it if missing). One agent (pi in Docker) runs per paper, --jobs in parallel.

Outputs in --run-dir: report.md (start here), summary.csv, handoff_repos.txt, handoff.csv,
review_list.csv, stats.json, papers/<id>.json, work/<id>/ (incl. transcripts), run.log.
"""

import argparse
import json
import os
import secrets
import subprocess
import sys
from collections import Counter
from pathlib import Path

DIR_CURRENT = Path(__file__).resolve().parent
DIR_ROOT = DIR_CURRENT.parent.parent
SEEDS = DIR_ROOT / "hlsfactory_agent" / "paper_filter" / "seed_repos.txt"
BUILD_SH = DIR_ROOT / "docker_images" / "paper_filter" / "build.sh"
MODEL = "deepseek/deepseek-v4-flash"  # DeepSeek V4 Flash on OpenRouter; same as exp/run_base/run.py
PACKAGES = ["docker", "dotenv", "joblib", "pandas", "openpyxl", "pymupdf", "requests"]
INSTALL_HINT = (
    "UV_CACHE_DIR=/export/hdd/scratch/jzhou652/uv-cache ~/.local/bin/uv pip install "
    "--python .venv/bin/python docker joblib pandas openpyxl pymupdf python-dotenv requests pytest psutil"
)

sys.path.insert(0, str(DIR_ROOT))  # hlsfactory_agent importable without PYTHONPATH
sys.stdout.reconfigure(line_buffering=True)  # progress lines show up in run.log immediately


def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--pdfs", type=Path, default=DIR_CURRENT / "pdfs")
    ap.add_argument("--sheet", type=Path, default=DIR_ROOT / "hls_source_indexing/hls_papers_merged.xlsx")
    ap.add_argument("--run-dir", type=Path, default=DIR_CURRENT / "runs" / "run1")
    ap.add_argument("--model", default=MODEL)
    ap.add_argument("--image", default="paper-filter-agent")
    ap.add_argument("--jobs", type=int, default=6, help="parallel agents (6 worked well on chao-srv1)")
    ap.add_argument("--timeout", type=int, default=1800, help="seconds per paper")
    ap.add_argument("--limit", type=int, help="only the first N papers (pilot)")
    ap.add_argument("--only", help="only papers whose id contains this text")
    ap.add_argument("--dry-run", action="store_true", help="prepare work folders, no containers")
    ap.add_argument("--force", action="store_true", help="redo papers that already finished")
    ap.add_argument("--detach", action="store_true", help="run in the background, log to <run-dir>/run.log")
    ap.add_argument("--status", action="store_true", help="show progress of --run-dir and exit")
    return ap.parse_args()


# --- --status -------------------------------------------------------------------------


def _running_containers(run_dir: Path) -> list[str]:
    """Paper ids of this run's agent containers that are still up ([] if Docker is unreachable)."""
    try:
        import docker

        cs = docker.from_env().containers.list(filters={"label": f"paper_filter.run_dir={run_dir}"})
        return [c.labels.get("paper_filter.paper", c.name) for c in cs]
    except Exception:
        return []


def _pid_alive(run_dir: Path) -> int | None:
    try:
        pid = int((run_dir / "run.pid").read_text())
        os.kill(pid, 0)
        return pid
    except (OSError, ValueError):
        return None


def show_status(run_dir: Path) -> None:
    run_dir = run_dir.resolve()
    if not run_dir.exists():
        raise SystemExit(f"{run_dir} does not exist")
    results = [json.loads(f.read_text()) for f in sorted((run_dir / "papers").glob("*.json"))]
    started = len([d for d in (run_dir / "work").glob("*") if d.is_dir()]) if (run_dir / "work").exists() else 0
    by_status = Counter(r["status"] for r in results)
    running = _running_containers(run_dir)
    pid = _pid_alive(run_dir)
    print(f"run dir:  {run_dir}")
    print(f"process:  {'running (pid %d)' % pid if pid else 'not running'}")
    print(f"finished: {len(results)} papers  " + ", ".join(f"{k} {v}" for k, v in sorted(by_status.items())))
    print(f"started:  {started} work folders; agents running now: {len(running)}")
    for p in running:
        print(f"   - {p}")
    outcomes = Counter(r.get("outcome") or r["status"] for r in results)
    if outcomes:
        print("outcomes: " + ", ".join(f"{k} {v}" for k, v in sorted(outcomes.items())))
    log = run_dir / "run.log"
    if log.exists():
        print("\nlast lines of run.log:")
        print("".join(log.read_text().splitlines(keepends=True)[-5:]), end="")
    if (run_dir / "report.md").exists():
        print(f"\nreport: {run_dir / 'report.md'}")


# --- --detach -------------------------------------------------------------------------


def detach(args: argparse.Namespace) -> None:
    """Re-launch this script without --detach as a background process that outlives the shell."""
    args.run_dir.mkdir(parents=True, exist_ok=True)
    run_dir = args.run_dir.resolve()
    if pid := _pid_alive(run_dir):
        raise SystemExit(f"a run is already active in {run_dir} (pid {pid})")
    argv = [a for a in sys.argv[1:] if a != "--detach"]
    log = open(run_dir / "run.log", "a")
    env = {**os.environ, "PYTHONPATH": str(DIR_ROOT)}
    p = subprocess.Popen(
        [sys.executable, str(Path(__file__).resolve()), *argv],
        cwd=DIR_ROOT, stdout=log, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL,
        start_new_session=True, env=env,
    )  # fmt: skip
    (run_dir / "run.pid").write_text(str(p.pid))
    rel = os.path.relpath(run_dir, Path.cwd())
    print(f"started in the background (pid {p.pid}); log: {rel}/run.log")
    print(f"check:  .venv/bin/python exp/paper_filter/run_agent.py --status --run-dir {rel}")
    print(f"stop:   kill {p.pid}   (finished papers are kept; rerun the same command to resume)")


# --- preflight ------------------------------------------------------------------------


def preflight(args: argparse.Namespace, env: dict) -> None:
    """Fail fast, with a fix, before any container starts."""
    missing = []
    for mod in PACKAGES:
        try:
            __import__(mod)
        except ImportError:
            missing.append(mod)
    if missing:
        raise SystemExit(f"missing Python packages: {', '.join(missing)}\ninstall with:\n  {INSTALL_HINT}")
    if not args.pdfs.exists():
        raise SystemExit(f"PDF folder {args.pdfs} does not exist")
    if args.dry_run:
        return
    if not env.get("OPENROUTER_API_KEY"):
        raise SystemExit("OPENROUTER_API_KEY missing from .env (or use --dry-run)")
    import docker

    try:
        client = docker.from_env()
        client.ping()
    except Exception as e:
        raise SystemExit(f"cannot reach Docker ({e}); is the daemon running and are you in the docker group?")
    try:
        client.images.get(args.image)
    except docker.errors.ImageNotFound:
        if args.image != "paper-filter-agent":
            raise SystemExit(f"Docker image {args.image} not found")
        print(f"Docker image {args.image} not found; building it (a few minutes, once)...")
        subprocess.run(["bash", str(BUILD_SH)], check=True)


# --- main run -------------------------------------------------------------------------


def run(args: argparse.Namespace) -> None:
    from dotenv import dotenv_values

    env = dotenv_values(DIR_ROOT / ".env")
    preflight(args, env)

    from joblib import Parallel, delayed

    from hlsfactory_agent.paper_filter.github import load_token
    from hlsfactory_agent.paper_filter.manifest import discover_papers
    from hlsfactory_agent.paper_filter.outputs import build_outputs
    from hlsfactory_agent.paper_filter.proxy import ExaProvider, ProxyConfig, ProxyServer, bridge_address
    from hlsfactory_agent.paper_filter.registry import Registry, load_seeds
    from hlsfactory_agent.paper_filter.report import build_report
    from hlsfactory_agent.paper_filter.runner import RunConfig, run_one

    args.run_dir.mkdir(parents=True, exist_ok=True)
    github_token = load_token(DIR_ROOT / ".env")
    if github_token is None:
        print("warning: no GITHUB_TOKEN; code search needs one and limits are very low without")
    exa_key = env.get("EXA_KEY")
    if not exa_key:
        print("note: no EXA_KEY in .env; web search answers 501 and agents skip it")

    sheet = args.sheet if args.sheet.exists() else None
    if sheet is None:
        print(f"note: spreadsheet {args.sheet} not found; titles come from filenames")
    papers = discover_papers(args.pdfs, sheet)
    if args.only:
        papers = [p for p in papers if args.only in p.paper_id]
    unmatched = [p.paper_id for p in papers if not p.matched]
    print(f"{len(papers)} PDFs; {len(unmatched)} not matched to a spreadsheet row")
    groups = Counter(p.group or "(top level)" for p in papers)
    if len(groups) > 1 or None not in {p.group for p in papers}:
        print("by folder: " + ", ".join(f"{g}={n}" for g, n in sorted(groups.items())))

    registry = Registry(args.run_dir / "registry.sqlite")
    registry.seed(load_seeds(SEEDS))

    done = set() if args.force else {p.paper_id for p in papers if registry.paper_done(p.paper_id)}
    todo = [p for p in papers if p.paper_id not in done]
    if args.limit:
        todo = todo[: args.limit]
    print(f"{len(done)} already done, running {len(todo)}")

    proxy = None
    cfg = RunConfig(
        model=args.model, api_key=env.get("OPENROUTER_API_KEY"), image=args.image,
        github_token=github_token, timeout_s=args.timeout, dry_run=args.dry_run,
    )  # fmt: skip
    if not args.dry_run:
        secret = secrets.token_hex(16)
        proxy = ProxyServer(
            ProxyConfig(secret=secret, token=github_token, log_path=args.run_dir / "proxy_calls.jsonl"),
            registry,
            host=bridge_address(),
            web_provider=ExaProvider(exa_key) if exa_key else None,
        ).start()
        cfg.proxy_url = f"http://host.docker.internal:{proxy.port}"
        cfg.proxy_secret = secret
        print(f"proxy listening on port {proxy.port}")

    # Papers being (re)run must not see their own earlier finds as already known.
    snapshot = registry.snapshot(exclude_papers={p.paper_id for p in todo})
    n_done = 0
    try:
        gen = Parallel(n_jobs=args.jobs, return_as="generator_unordered")(
            delayed(run_one)(p, cfg, args.run_dir, snapshot) for p in todo
        )
        for final in gen:  # single writer: merge here, in the parent
            if final["status"] == "ok":
                registry.merge_result(final, str(args.run_dir / "papers" / f"{final['paper_id']}.json"))
            n_done += 1
            print(f"[{n_done}/{len(todo)}] {final['paper_id'][:50]}: {final['status']} {final.get('outcome') or ''}")
    finally:
        if proxy:
            proxy.shutdown()

    # Outputs cover every paper with a result on disk (this run and earlier ones).
    results = [json.loads(f.read_text()) for f in sorted((args.run_dir / "papers").glob("*.json"))]
    stats = build_outputs(args.run_dir, results)
    report = build_report(args.run_dir, results, stats)
    print(json.dumps(stats, indent=2))
    print(f"report: {report}")


def main() -> None:
    args = parse_args()
    if args.status:
        show_status(args.run_dir)
    elif args.detach:
        detach(args)
    else:
        try:
            run(args)
        finally:
            pid_file = args.run_dir / "run.pid"
            if pid_file.exists() and pid_file.read_text().strip() == str(os.getpid()):
                pid_file.unlink()


if __name__ == "__main__":
    main()
