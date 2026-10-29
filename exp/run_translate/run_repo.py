"""Full loop on one repository: extract with HLSFactory-Agent, oracle-check each design,
translate each design to a target with the translation agent, run the checks, and save the
outputs under results/ so they can be synthesized elsewhere.

Example:
    PYTHONPATH=. python exp/run_translate/run_repo.py --repo UCLA-VAST/HP-FFT-HLS --target catapult --jobs 4
    PYTHONPATH=. python exp/run_translate/run_repo.py --repo UCLA-VAST/HP-FFT-HLS --skip-extract   # reuse extraction
    PYTHONPATH=. python exp/run_translate/run_repo.py --repo UCLA-VAST/HP-FFT-HLS --skip-extract --oracle-only \\
        --repair-extraction --repo-clone /path/to/clones   # rerun only the original testbenches, repaired
"""

import argparse
import json
import shutil
import time
from datetime import date, datetime
from pathlib import Path

from dotenv import dotenv_values
from joblib import Parallel, delayed

from hlsfactory_agent.core import HLSFactoryAgentRun
from hlsfactory_agent.extraction_repair import repair_extraction
from dataclasses import asdict, replace

from hlsfactory_agent.stats import ResourceSampler, StageTimer, agent_usage, host_info
from hlsfactory_agent.synth import run_synth_cached, run_synth_degraded
from hlsfactory_agent.targets.vitis import VITIS
from hlsfactory_agent.translate import (
    OUTPUT_DIR_NAME,
    TARGETS,
    HLSTranslationRun,
    _rmtree_robust,
    check_translated_design,
    compare_outputs,
    get_target,
    normalize_vitis_script,
    repair_top,
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
    """Wall time, tokens, cost and tool calls summed over every agent attempt of the run."""
    u = agent_usage(dir_run)
    return {k: u[k] for k in ("wall_s", "tokens_in", "tokens_out", "cost_usd", "tool_calls")}


def translate_one(
    dir_design: Path, target: str, model: str, api_key: str, dir_runs: Path, resume: bool = False, attempts: int = 1,
    synth: str = "none", degrade: int = 0, synth_timeout: int | None = None, oracle_timeout: int = 120,
    reoracle: bool = False, oracle_only: bool = False,
) -> dict:
    run_id = f"translate-{target}-{dir_design.name}"
    dir_run = dir_runs / run_id
    t0 = time.monotonic()
    row: dict = {"design": dir_design.name, "run_id": run_id, "top": repair_top(dir_design), "script": normalize_vitis_script(dir_design)}
    timer = StageTimer()
    oracle_kw = {"timeout_s": oracle_timeout, "dir_scratch": dir_runs / ".oracle_scratch"}
    try:
        if oracle_only:  # the original testbench only: no agent; an earlier translation's checks ride along
            with timer("oracle"):
                oracle = run_oracle_check(dir_design, **oracle_kw)
            dir_run.mkdir(parents=True, exist_ok=True)
            (dir_run / "oracle.json").write_text(json.dumps(oracle, indent=2), encoding="utf-8")
            row["resumed"] = (dir_run / "check_data.json").exists()
            check = json.loads((dir_run / "check_data.json").read_text(encoding="utf-8")) if row["resumed"] else {
                "passed": False, "static": {"passed": False, "failures": ["not run"]},
                "container": {"syntax_all_ok": False, "testbench_ok": False}, "attempts_used": 0,
            }
        elif resume and (dir_run / "check_data.json").exists() and (dir_run / "oracle.json").exists():
            if reoracle:
                with timer("oracle"):
                    oracle = run_oracle_check(dir_design, **oracle_kw)
                (dir_run / "oracle.json").write_text(json.dumps(oracle, indent=2), encoding="utf-8")
            else:
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
            with timer("oracle"):
                oracle = run_oracle_check(dir_design, **oracle_kw)
            with timer("translate"):
                check = HLSTranslationRun(run_id, dir_design, dir_run, model, api_key, target=target, attempts=attempts).run()
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
                "attempts_used": check.get("attempts_used", 1),
                "attempts": check.get("attempts", []),
                "error": None,
            }
        )
        row.update(session_stats(dir_run))
        spec = get_target(target)
        if synth_timeout:
            spec = replace(spec, synth_timeout_s=synth_timeout)
        cache = dir_runs / ".synth_cache"
        with timer("synth_source"):
            row["synth_source"] = asdict(run_synth_cached(VITIS, dir_design, dir_run / "synth_source", cache)) if synth in ("both", "source") else {"status": "not run"}
        row["synth_target"] = {"status": "not run"}
        if synth in ("both", "target") and check["passed"] and spec.parse_report:
            with timer("synth_target"):
                tgt = run_synth_cached(spec, dir_run / "run_area" / OUTPUT_DIR_NAME, dir_run / "synth", cache)
            row["synth_target"] = asdict(tgt)
            if tgt.status == "FAILED" and degrade:
                with timer("recovery"):
                    row["synth_target_degraded"] = run_synth_degraded(spec, dir_run / "run_area" / OUTPUT_DIR_NAME, dir_run / "synth", cache, rounds=degrade)
            (dir_run / "synth.json").write_text(json.dumps({k: row[k] for k in ("synth_target", "synth_target_degraded") if k in row}, indent=4), encoding="utf-8")
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
    row["stage_s"] = timer.seconds
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
    categories: dict[str, int] = {}
    for row in rows:
        f = dir_runs / row["run_id"] / "run_area" / OUTPUT_DIR_NAME / "rewrite_log.json"
        if not f.exists():
            continue
        try:
            counts = json.loads(f.read_text(encoding="utf-8")).get("category_counts", {})
        except json.JSONDecodeError:  # the agent may edit the log it was given
            counts = {}
        for k, v in counts.items():
            categories[k] = categories.get(k, 0) + v

    n = len(rows)
    lines += [
        "",
        f"Designs: {n}. Oracle pass: {sum(r['oracle_passed'] for r in rows)}. Translation pass: {sum(r['translation_passed'] for r in rows)}. "
        f"Total agent cost: {round(sum(float(r.get('cost_usd') or 0) for r in rows), 4)} USD. "
        f"Total agent wall time: {sum(int(r.get('wall_s') or 0) for r in rows)} s.",
        "",
        "Pragma categories across designs: "
        + ", ".join(f"{k} {v}" for k, v in sorted(categories.items()))
        + "."
        if categories
        else "",
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
    parser.add_argument("--attempts", type=int, default=1, help="agent attempts per design; failures feed a retry prompt")
    parser.add_argument("--synth", nargs="?", const="both", default="none", choices=("both", "source", "target", "none"),
                        help="off by default; `--synth` alone runs Vitis on the source and the target tool on the translation")
    parser.add_argument("--degrade", type=int, default=2, help="recovery rounds after a failed target synthesis (0 = off)")
    parser.add_argument("--synth-timeout", type=int, default=None, help="seconds per target synthesis; default is the target's own")
    parser.add_argument("--oracle-timeout", type=int, default=120, help="seconds the original testbench may run")
    parser.add_argument("--reoracle", action="store_true", help="with --resume: run the original testbench again instead of reusing oracle.json")
    parser.add_argument("--oracle-only", action="store_true", help="run only the original testbenches; no translation agent")
    parser.add_argument("--repair-extraction", action="store_true",
                        help="before the oracle, repair the extracted designs in place (script source set, escaped includes, "
                             "files recovered from --repo-clone); logs to <design>/extraction_repair.json")
    parser.add_argument("--repo-clone", type=Path, default=None,
                        help="a clone of the repository, or a folder of clones named owner__repo, for --repair-extraction")
    parser.add_argument("--runs-dir", type=Path, default=DIR_RUNS)
    parser.add_argument("--results-dir", type=Path, default=DIR_RESULTS)
    parser.add_argument("--env", type=Path, default=Path(".env"))
    args = parser.parse_args()

    api_key = check_key(dotenv_values(args.env).get("OPENROUTER_API_KEY"))
    args.runs_dir.mkdir(parents=True, exist_ok=True)

    repo_name = args.repo.split("/")[1]
    tag = args.repo.replace("/", "__")
    dir_stats = args.runs_dir / "stats"
    sampler = ResourceSampler(dir_stats / f"{tag}.resources.jsonl")
    sampler.start()
    started, timer = datetime.now(), StageTimer()
    if args.skip_extract:
        dir_designs = args.runs_dir / f"extract-{tag}" / "run_area" / "output_hls_designs" / repo_name
    else:
        with timer("extract"):
            dir_designs = extract(args.repo, args.model, api_key, args.runs_dir)

    designs = sorted(p for p in dir_designs.iterdir() if p.is_dir())
    if args.only:
        wanted = {n.strip() for n in args.only.split(",") if n.strip()}
        designs = [d for d in designs if d.name in wanted]
    if args.limit:
        designs = designs[: args.limit]
    print(f"[translate] {len(designs)} designs from {dir_designs}")
    if args.repair_extraction:
        clone = args.repo_clone
        if clone is not None and (clone / tag).is_dir():
            clone = clone / tag
        with timer("repair"):
            for d in designs:
                log = repair_extraction(d, clone)
                inc = log["includes"]
                print(f"[repair] {d.name}: sources={log['sources']['sources']} dropped={list(log['sources']['dropped'])} "
                      f"includes rewritten={len(inc['rewritten'])} copied={len(inc['copied'])} unresolved={len(inc['unresolved'])} "
                      f"data copied={len(log.get('data', {}).get('copied', {}))}")

    with timer("translate"):
        rows = Parallel(n_jobs=args.jobs)(
            delayed(translate_one)(d, args.target, args.model, api_key, args.runs_dir, args.resume, args.attempts, args.synth, args.degrade,
                                   args.synth_timeout, args.oracle_timeout, args.reoracle, args.oracle_only)
            for d in designs
        )
    rows = sorted(rows, key=lambda r: r["design"])
    sampler.stop()
    stats = {
        "repo": args.repo, "target": args.target, "model": args.model, "started": started.isoformat(timespec="seconds"),
        "ended": datetime.now().isoformat(timespec="seconds"), "wall_s": round((datetime.now() - started).total_seconds()),
        "jobs": args.jobs, "attempts": args.attempts, "synth": args.synth, "degrade": args.degrade, "synth_timeout": args.synth_timeout,
        "oracle_timeout": args.oracle_timeout, "oracle_only": args.oracle_only, "repair_extraction": args.repair_extraction,
        "designs": len(designs), "extract_reused": args.skip_extract, "stage_s": timer.seconds, "host": host_info(),
        "resources": sampler.summary(), "design_seconds": {r["design"]: r.get("seconds") for r in rows},
    }
    (dir_stats / f"{tag}.json").write_text(json.dumps(stats, indent=2), encoding="utf-8")
    out = save_results(args.repo, args.target, dir_designs, args.runs_dir, rows, args.results_dir)
    print((out / "README.md").read_text())
    print(f"[done] results saved to {out}")


if __name__ == "__main__":
    main()
