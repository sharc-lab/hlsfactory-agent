"""Full loop on one repository: extract with HLSFactory-Agent, oracle-check each design,
translate each design to a target with the translation agent, run the checks, and save the
outputs under results/ so they can be synthesized elsewhere.

Example:
    PYTHONPATH=. python exp/run_translate/run_repo.py --repo UCLA-VAST/HP-FFT-HLS --target catapult --jobs 4
    PYTHONPATH=. python exp/run_translate/run_repo.py --repo UCLA-VAST/HP-FFT-HLS --skip-extract   # reuse extraction
"""

import argparse
import json
import shutil
import time
from datetime import date
from pathlib import Path

from dotenv import dotenv_values
from joblib import Parallel, delayed

from hlsfactory_agent.core import HLSFactoryAgentRun
from hlsfactory_agent.translate import (
    OUTPUT_DIR_NAME,
    TARGETS,
    HLSTranslationRun,
    _rmtree_robust,
    check_translated_design,
    compare_outputs,
    get_target,
    run_oracle_check,
)
from hlsfactory_agent.utils import check_key

DIR_CURRENT = Path(__file__).resolve().parent
DIR_RUNS = DIR_CURRENT / "runs"
DIR_RESULTS = DIR_CURRENT / "results"

SAVE_SUFFIXES = {".cpp", ".cc", ".c", ".h", ".hpp", ".tcl", ".md", ".txt", ".json", ".dat", ".csv", ".hex", ".mem", ".bin"}


def extract(repo_id: str, model: str, api_key: str, dir_runs: Path) -> Path:
    repo_name = repo_id.split("/")[1]
    run_id = f"extract-{repo_id.replace('/', '__')}"
    run = HLSFactoryAgentRun(run_id, repo_id, dir_runs / run_id, model, api_key)
    t0 = time.monotonic()
    run.run()
    print(f"[extract] {repo_id} done in {time.monotonic() - t0:.0f}s")
    dir_designs = dir_runs / run_id / "run_area" / "output_hls_designs" / repo_name
    if not dir_designs.exists():
        raise RuntimeError(f"No designs extracted at {dir_designs}")
    return dir_designs


def session_stats(dir_run: Path) -> dict:
    """Wall time, tokens, cost, and tool-call counts from the saved Pi session in run_data.json."""
    stats = {"wall_s": None, "tokens_in": 0, "tokens_out": 0, "cost_usd": 0.0, "tool_calls": 0}
    f = dir_run / "run_data.json"
    if not f.exists():
        return stats
    try:
        events = json.loads(f.read_text(encoding="utf-8")).get("session_data") or []
    except Exception:
        return stats
    t0 = t1 = None
    for e in events:
        ts = e.get("timestamp")
        if ts:
            t0 = t0 or ts
            t1 = ts
        m = e.get("message") or {}
        u = m.get("usage") or {}
        if u:
            stats["tokens_in"] += int(u.get("input", u.get("input_tokens", 0)) or 0)
            stats["tokens_out"] += int(u.get("output", u.get("output_tokens", 0)) or 0)
            c = u.get("cost") or 0
            stats["cost_usd"] += float(c.get("total", 0) if isinstance(c, dict) else c)
        content = m.get("content")
        if isinstance(content, list):
            stats["tool_calls"] += sum(1 for part in content if isinstance(part, dict) and part.get("type") == "toolCall")
    if t0 and t1:
        from datetime import datetime
        fmt = "%Y-%m-%dT%H:%M:%S.%fZ"
        try:
            stats["wall_s"] = round((datetime.strptime(t1, fmt) - datetime.strptime(t0, fmt)).total_seconds())
        except ValueError:
            pass
    stats["cost_usd"] = round(stats["cost_usd"], 4)
    return stats


def translate_one(dir_design: Path, target: str, model: str, api_key: str, dir_runs: Path, resume: bool = False) -> dict:
    run_id = f"translate-{target}-{dir_design.name}"
    dir_run = dir_runs / run_id
    t0 = time.monotonic()
    row: dict = {"design": dir_design.name, "run_id": run_id}
    try:
        if resume and (dir_run / "check_data.json").exists() and (dir_run / "oracle.json").exists():
            oracle = json.loads((dir_run / "oracle.json").read_text(encoding="utf-8"))
            check = json.loads((dir_run / "check_data.json").read_text(encoding="utf-8"))
            # Static checks are cheap and deterministic: recompute them so resumed rows follow the current rules.
            scan_path = dir_run / "run_area" / "scan.json"
            scan = json.loads(scan_path.read_text(encoding="utf-8")) if scan_path.exists() else None
            check["static"] = check_translated_design(dir_run / "run_area" / OUTPUT_DIR_NAME, get_target(target), scan=scan)
            check["passed"] = bool(check["static"]["passed"] and check["container"]["testbench_ok"])
            (dir_run / "check_data.json").write_text(json.dumps(check, indent=4), encoding="utf-8")
            row["resumed"] = True
        else:
            oracle = run_oracle_check(dir_design)
            check = HLSTranslationRun(run_id, dir_design, dir_run, model, api_key, target=target).run()
            (dir_run / "oracle.json").write_text(json.dumps(oracle, indent=2), encoding="utf-8")
            row["resumed"] = False
        row.update(
            {
                "oracle_passed": oracle["passed"],
                "oracle": oracle,
                "translation_passed": check["passed"],
                "static_failures": check["static"]["failures"],
                "syntax_all_ok": check["container"]["syntax_all_ok"],
                "testbench_ok": check["container"]["testbench_ok"],
                "error": None,
            }
        )
        row.update(session_stats(dir_run))
        orig_run = oracle.get("run") or {}
        trans_run = (check.get("container") or {}).get("testbench_run") or {}
        if orig_run.get("exit_code") is not None and trans_run.get("exit_code") is not None:
            row["output_match"] = compare_outputs(orig_run.get("output", ""), trans_run.get("output", ""))
        else:
            row["output_match"] = None
    except Exception as e:  # keep the batch going; the row records the failure
        row.update(
            {
                "oracle_passed": False,
                "oracle": {"passed": False, "error": repr(e)},
                "translation_passed": False,
                "static_failures": [f"harness error: {type(e).__name__}"],
                "syntax_all_ok": False,
                "testbench_ok": False,
                "error": repr(e)[:2000],
            }
        )
    row["seconds"] = round(time.monotonic() - t0)
    print(f"[design] {dir_design.name}: oracle={row['oracle_passed']} translation={row['translation_passed']} ({row['seconds']}s)")
    return row


def _match_cell(m: dict | None) -> str:
    if m is None:
        return "n/a"
    return "yes" if m.get("identical") else f"no ({m.get('differing')} lines)"


def save_results(repo_id: str, target: str, dir_designs: Path, dir_runs: Path, rows: list[dict], dir_results: Path) -> Path:
    repo_name = repo_id.split("/")[1]
    out = dir_results / f"{date.today().isoformat()}-{repo_name}-to-{target}"
    if out.exists():
        _rmtree_robust(out)
    out.mkdir(parents=True)
    for row in rows:
        d = out / row["design"]
        (d / "vitis").mkdir(parents=True)
        (d / target).mkdir(parents=True)
        for p in (dir_designs / row["design"]).rglob("*"):
            if p.is_file() and p.suffix.lower() in SAVE_SUFFIXES:
                dst = d / "vitis" / p.relative_to(dir_designs / row["design"])
                dst.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(p, dst)
        src_out = dir_runs / row["run_id"] / "run_area" / OUTPUT_DIR_NAME
        if src_out.exists():
            for p in src_out.rglob("*"):
                if p.is_file() and p.suffix.lower() in SAVE_SUFFIXES:
                    dst = d / target / p.relative_to(src_out)
                    dst.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(p, dst)
        for name in ("check_data.json",):
            src = dir_runs / row["run_id"] / name
            if src.exists():
                shutil.copy2(src, d / name)
        html = next((dir_runs / row["run_id"] / "run_area" / ".pi" / "sessions").glob("*.html"), None)
        if html:
            shutil.copy2(html, d / "session_transcript.html")
        (d / "oracle.json").write_text(json.dumps(row["oracle"], indent=2), encoding="utf-8")
    (out / "summary.json").write_text(json.dumps(rows, indent=2), encoding="utf-8")
    lines = [
        f"# {repo_id} -> {target}",
        "",
        "| design | oracle (orig testbench) | translation checks | syntax | testbench | outputs match | agent wall s | tool calls | tokens in/out | cost USD |",
        "|---|---|---|---|---|---|---|---|---|---|",
    ]
    for r in rows:
        lines.append(
            f"| {r['design']} | {'pass' if r['oracle_passed'] else 'FAIL'} | {'pass' if r['translation_passed'] else 'FAIL'} | "
            f"{'ok' if r['syntax_all_ok'] else 'FAIL'} | {'pass' if r['testbench_ok'] else 'FAIL'} | "
            f"{_match_cell(r.get('output_match'))} | "
            f"{r.get('wall_s', '')} | {r.get('tool_calls', '')} | {r.get('tokens_in', '')}/{r.get('tokens_out', '')} | {r.get('cost_usd', '')} |"
        )
    n = len(rows)
    lines += [
        "",
        f"Designs: {n}. Oracle pass: {sum(r['oracle_passed'] for r in rows)}. Translation pass: {sum(r['translation_passed'] for r in rows)}. "
        f"Total agent cost: {round(sum(float(r.get('cost_usd') or 0) for r in rows), 4)} USD. "
        f"Total agent wall time: {sum(int(r.get('wall_s') or 0) for r in rows)} s.",
        "",
        "Each design folder holds `vitis/` (the extracted original), the target folder (translated output with run.tcl and",
        "translation_report.md), `oracle.json`, `check_data.json`, and the agent session transcript.",
    ]
    (out / "README.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return out


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--repo", required=True, help="GitHub repo id, e.g. UCLA-VAST/HP-FFT-HLS")
    parser.add_argument("--target", default="catapult", choices=sorted(TARGETS))
    parser.add_argument("--model", default="deepseek/deepseek-v4-flash")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--skip-extract", action="store_true", help="reuse an existing extraction run")
    parser.add_argument("--resume", action="store_true", help="reuse translations that already have check_data.json")
    parser.add_argument("--limit", type=int, default=0, help="translate at most N designs (0 = all)")
    parser.add_argument("--only", default="", help="comma-separated design names to translate; others are skipped")
    parser.add_argument("--runs-dir", type=Path, default=DIR_RUNS)
    parser.add_argument("--results-dir", type=Path, default=DIR_RESULTS)
    parser.add_argument("--env", type=Path, default=Path(".env"))
    args = parser.parse_args()

    api_key = check_key(dotenv_values(args.env).get("OPENROUTER_API_KEY"))
    args.runs_dir.mkdir(parents=True, exist_ok=True)

    repo_name = args.repo.split("/")[1]
    if args.skip_extract:
        dir_designs = args.runs_dir / f"extract-{args.repo.replace('/', '__')}" / "run_area" / "output_hls_designs" / repo_name
    else:
        dir_designs = extract(args.repo, args.model, api_key, args.runs_dir)

    designs = sorted(p for p in dir_designs.iterdir() if p.is_dir())
    if args.only:
        wanted = {n.strip() for n in args.only.split(",") if n.strip()}
        designs = [d for d in designs if d.name in wanted]
    if args.limit:
        designs = designs[: args.limit]
    print(f"[translate] {len(designs)} designs from {dir_designs}")

    rows = Parallel(n_jobs=args.jobs)(
        delayed(translate_one)(d, args.target, args.model, api_key, args.runs_dir, args.resume) for d in designs
    )
    rows = sorted(rows, key=lambda r: r["design"])
    out = save_results(args.repo, args.target, dir_designs, args.runs_dir, rows, args.results_dir)
    print((out / "README.md").read_text())
    print(f"[done] results saved to {out}")


if __name__ == "__main__":
    main()
