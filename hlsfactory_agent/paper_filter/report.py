"""Human-readable run report (report.md) built from the per-paper results of one run dir.

Written by run_agent.py after every run, next to the CSV/JSON outputs. Links are relative to
the run dir, so the report can be opened from there (VS Code, GitHub preview, any viewer).
"""

import json
from collections import Counter
from datetime import datetime
from pathlib import Path

from hlsfactory_agent.paper_filter.outputs import review_reasons

EXA_COST_PER_QUERY = 0.007  # USD, see proxy.ExaProvider


def _cell(text, limit: int = 0) -> str:
    s = str(text if text is not None else "").replace("|", "\\|").replace("\n", " ").strip()
    return s[: limit - 1] + "…" if limit and len(s) > limit else s


def _group(r: dict) -> str:
    return (r.get("meta") or {}).get("group") or ""


def _transcript(run_dir: Path, paper_id: str) -> str:
    """Relative link to the pi session HTML for one paper, or '' if there is none."""
    sessions = sorted((run_dir / "work" / paper_id / "run_area" / ".pi" / "sessions").glob("*.html"))
    return f"[transcript]({sessions[-1].relative_to(run_dir).as_posix()})" if sessions else ""


def _calls(run_dir: Path) -> tuple[Counter, int]:
    """Proxy calls per category, and how many web searches actually hit Exa (not cache)."""
    by_cat: Counter = Counter()
    exa = 0
    log = run_dir / "proxy_calls.jsonl"
    if not log.exists():
        return by_cat, exa
    for line in log.read_text().splitlines():
        try:
            c = json.loads(line)
        except ValueError:
            continue
        by_cat[c.get("category") or "?"] += 1
        if c.get("category") == "web" and c.get("status") == 200 and c.get("cached") is False:
            exa += 1
    return by_cat, exa


def _evidence(repo: dict, n: int = 3) -> str:
    return "; ".join(e["type"] for e in repo.get("evidence", [])[:n])


def build_report(run_dir: Path, results: list[dict], stats: dict) -> Path:
    run_dir = Path(run_dir)
    titles = {r["paper_id"]: r.get("title") or r["paper_id"] for r in results}
    out: list[str] = []
    w = out.append

    w(f"# Paper-filter run: `{run_dir.name}`\n")
    w(f"Generated {datetime.now():%Y-%m-%d %H:%M}. {stats['papers']} papers; "
      f"{stats['handoff_repos']} repos handed off; {stats['review_rows']} for review.\n")  # fmt: skip
    w("Status: " + ", ".join(f"{k} {v}" for k, v in sorted(stats["by_status"].items())) + "  ")
    w("Outcome: " + ", ".join(f"{k} {v}" for k, v in sorted(stats["by_outcome"].items())) + "\n")

    if stats.get("by_group"):
        outcomes = sorted({o for g in stats["by_group"].values() for o in g["by_outcome"]})
        w("## By group\n")
        w("| group | papers | " + " | ".join(outcomes) + " | with handoff |")
        w("|---|---|" + "---|" * len(outcomes) + "---|")
        for g, s in stats["by_group"].items():
            row = " | ".join(str(s["by_outcome"].get(o, 0)) for o in outcomes)
            w(f"| {g} | {s['papers']} | {row} | {s['with_handoff']} |")
        w("")

    w("## Handed off to HLSFactory\n")
    w("Also in `handoff_repos.txt` (paste into `repo_urls` in `exp/run_base/run.py`).\n")
    handoff = [(r, x) for r in results for x in r.get("repos", []) if x.get("handoff")]
    if handoff:
        w("| repo | provenance | HLS files | flags | paper | evidence |")
        w("|---|---|---|---|---|---|")
        for r, x in sorted(handoff, key=lambda t: t[1]["repo_id"].lower()):
            scan = x.get("scan") or {}
            n_hls = scan.get("hls_files", "")
            if scan.get("classification") == "hls_tcl_only":
                n_hls = f"{len(scan.get('tcl_set_top', []))} TCL projects"
            w(f"| [{x['repo_id']}](https://github.com/{x['repo_id']}) | {x['provenance']} | "
              f"{n_hls} |{_cell(';'.join(x.get('flags', [])))} | "
              f"{_cell(titles[r['paper_id']], 60)} | {_cell(_evidence(x))} |")  # fmt: skip
    else:
        w("None.")
    w("")

    w("## Needs a human look\n")
    review = [(r, x, review_reasons(x)) for r in results for x in r.get("repos", [])]
    review = [t for t in review if t[2]]
    if review:
        w("| repo | paper | reasons | flags |")
        w("|---|---|---|---|")
        for r, x, reasons in review:
            w(f"| [{x['repo_id']}](https://github.com/{x['repo_id']}) | "
              f"{_cell(titles[r['paper_id']], 60)} | {';'.join(reasons)} | "
              f"{_cell(';'.join(x.get('flags', [])))} |")  # fmt: skip
    else:
        w("None.")
    w("")

    failed = [r for r in results if r["status"] != "ok"]
    if failed:
        w("## Failed papers\n")
        w("Rerun the same command: failed papers are retried, finished ones skipped.\n")
        for r in failed:
            err = (r.get("error") or "").strip().splitlines()
            w(f"- `{r['paper_id']}`: {r['status']}: {_cell(err[-1] if err else '', 160)}")
        w("")

    w("## All papers\n")
    w("| group | paper | outcome | repos (outcome) | problems | |")
    w("|---|---|---|---|---|---|")
    for r in sorted(results, key=lambda r: (_group(r), r["paper_id"])):
        repos = ", ".join(f"{x['repo_id']} ({x['outcome']}{', handoff' if x.get('handoff') else ''})"
                          for x in r.get("repos", []))  # fmt: skip
        w(f"| {_group(r)} | {_cell(titles[r['paper_id']], 70)} | {r.get('outcome') or r['status']} | "
          f"{_cell(repos)} | {len(r.get('problems', [])) or ''} | {_transcript(run_dir, r['paper_id'])} |")  # fmt: skip
    w("")

    by_cat, exa = _calls(run_dir)
    w("## API usage\n")
    if by_cat:
        w("Proxy calls: " + ", ".join(f"{k} {v}" for k, v in sorted(by_cat.items())) + ".  ")
        w(f"Exa queries billed (not cached): {exa}, about ${exa * EXA_COST_PER_QUERY:.2f}.  ")
    w("LLM token cost is not tracked yet (see OpenRouter usage).\n")

    w("## Files in this folder\n")
    w("- `summary.csv`: one row per paper. `handoff.csv` / `handoff_repos.txt`: repos for HLSFactory.")
    w("- `review_list.csv`: repos needing a human look, with evidence quotes. `stats.json`: counts.")
    w("- `papers/<id>.json`: full per-paper record (evidence, HLS scan, flags, problems).")
    w("- `work/<id>/`: prompt, agent output, PDF pre-pass, pi transcript (`run_area/.pi/sessions/*.html`).")
    w("- `run.log` (with `--detach`), `proxy_calls.jsonl`, `registry.sqlite`: logs and internal state.")

    path = run_dir / "report.md"
    path.write_text("\n".join(out) + "\n")
    return path
