"""Build docs/RUN_STATISTICS.md from the per-repo stats `run_repo.py` writes (`<runs>/stats/<repo>.json`) and the
per-design rows under results/: wall time and resources per repository, time per stage, the retry curve, how the
agent's attempts move between failure classes, and the designs that thrash. One function per section.

Example:
    PYTHONPATH=. python exp/run_translate/stats_report.py --runs-dir exp/run_translate/runs/census2 --results-dir exp/run_translate/results/census2
"""
from __future__ import annotations

import argparse
import json
import statistics
from collections import Counter
from pathlib import Path

from hlsfactory_agent.stats import agent_usage, failure_category, retry_stats, summarize_samples

DIR_CURRENT = Path(__file__).resolve().parent
STAGES = ("oracle", "translate", "synth_source", "synth_target", "recovery")
STAGE_NAMES = {"oracle": "oracle (original testbench)", "translate": "pre-pass + agent attempts + checks", "synth_source": "Vitis on the source", "synth_target": "target tool", "recovery": "recovery rounds"}


def _hms(s: float | None) -> str:
    if s is None:
        return "-"
    s = int(s)
    return f"{s // 3600}:{s % 3600 // 60:02d}:{s % 60:02d}"


def _mean(xs: list[float]) -> float | None:
    return round(statistics.mean(xs), 1) if xs else None


def load(dir_runs: Path, dir_results: Path) -> list[dict]:
    """One record per repository: its stats file (if the run wrote one) plus the rows of its latest results folder;
    rows without an `attempts` list (older runs) get it from the saved check_data.json."""
    repos: list[dict] = []
    for res in sorted(dir_results.glob("*-to-*")):
        rows = json.loads((res / "summary.json").read_text(encoding="utf-8")) if (res / "summary.json").exists() else []
        for r in rows:
            if "attempts" not in r and (res / r["design"] / "check_data.json").exists():
                r["attempts"] = json.loads((res / r["design"] / "check_data.json").read_text(encoding="utf-8")).get("attempts", [])
            if r.get("run_id") and (dir_runs / r["run_id"] / "run_data.json").exists():  # every attempt, not only the last one
                u = agent_usage(dir_runs / r["run_id"])
                r.update({k: v for k, v in u.items() if k != "attempts"})
                r["usage_attempts"] = u["attempts"]
        name = res.name.split("-", 3)[3].rsplit("-to-", 1)[0]
        stats_file = next((f for f in (dir_runs / "stats").glob("*.json") if f.stem.endswith("__" + name)), None) if (dir_runs / "stats").exists() else None
        stats = json.loads(stats_file.read_text(encoding="utf-8")) if stats_file else {}
        samples_file = stats_file.with_name(stats_file.stem + ".resources.jsonl") if stats_file else None
        if samples_file and samples_file.exists() and not stats.get("resources"):
            stats["resources"] = summarize_samples([json.loads(l) for l in samples_file.read_text(encoding="utf-8").splitlines() if l.strip()])
        tag = stats.get("repo", "").replace("/", "__") or (stats_file.stem if stats_file else "")
        ext = dir_runs / f"extract-{tag}" if tag else None
        stats["extract_usage"] = agent_usage(ext) if ext and (ext / "run_data.json").exists() else {}
        repos.append({"name": stats.get("repo", name), "results": res.name, "stats": stats, "rows": rows})
    return repos


def section_method(repos: list[dict]) -> str:
    st = next((r["stats"] for r in repos if r["stats"]), {})
    host = st.get("host", {})
    return "\n".join([
        "## 1. How the runs are organised",
        "",
        f"- Host: `{host.get('hostname', '?')}`, {host.get('cpus', '?')} CPUs, {host.get('mem_total_gb', '?')} GB; every tool runs in a container of the `hlsfactory-agent` image, the vendor installs bind-mounted read-only.",
        f"- Repositories run one after another (`run_repo.py`, one process per repo). Inside a repo, `--jobs {st.get('jobs', '?')}` designs run concurrently (joblib workers); each design is sequential: oracle, pre-pass, up to `--attempts {st.get('attempts', '?')}` agent attempts each followed by the static and container checks, then Vitis on the source and {st.get('target', 'the target')} on the translation ({st.get('synth_timeout') or 'target default'} s budget), then up to {st.get('degrade', '?')} recovery rounds.",
        "- A design therefore holds at most one agent container and one tool container at a time; the host sees at most `jobs` of each. The resource columns below are host-wide samples every 30 s (1-minute load, memory in use, running containers of the image).",
        "- Retry mechanics: after a failed attempt the agent gets the original prompt plus a `Previous attempt failed` section listing the harness's findings (static-check failures, fenced edits with the pre-pass version to restore, compile errors, the testbench diff); the next attempt edits the same output folder. A design passes when the static checks and the container testbench both pass; the retry loop stops at the first pass.",
        "- Stage time is wall time inside the design's worker, so `translate` includes the agent's own wall time and the checks; the per-attempt split (`agent_s`, `check_s`) is in each row's `attempts` list.",
    ])


def _oracle_cause(r: dict) -> str:
    o = r.get("oracle") or {}
    b, run = o.get("build") or {}, o.get("run") or {}
    if "404" in str(o.get("error", "")):
        return "harness (container lost)"
    if b.get("exit_code") not in (0, None):
        return "testbench does not link" if "linker" in (b.get("output") or "") else "testbench does not build"
    if run.get("exit_code") not in (0, None):
        return "testbench crashes" if run.get("exit_code") in (134, 136, 139) else "testbench reports a mismatch"
    return "other"


def _vitis_cause(r: dict) -> str:
    sy = r.get("synth_source") or {}
    e = sy.get("first_error") or ""
    if sy.get("status") == "TIMEOUT":
        return "timeout"
    if sy.get("failure_class") == "no-report" or "Top function not found" in e or "Unknown option" in e:
        return "extracted script broken"
    if "file not found" in e:
        return "missing include"
    return "source rejected by Vitis"


def _checks_cause(r: dict) -> str:
    return failure_category(r.get("static_failures") or []) if r.get("syntax_all_ok") and r.get("testbench_ok") else ("translation does not compile" if not r.get("syntax_all_ok") else "translated testbench mismatch")


def section_funnel(repos: list[dict]) -> str:
    """The stage-by-stage count with one line of causes per stage, from the rows' own records."""
    rows = [x for r in repos for x in r["rows"]]
    n = len(rows)
    def fmt(c: Counter) -> str:
        return "; ".join(f"{v} {k}" for k, v in c.most_common(4))
    oracle = Counter(_oracle_cause(r) for r in rows if not r.get("oracle_passed"))
    vitis = Counter(_vitis_cause(r) for r in rows if (r.get("synth_source") or {}).get("status") != "PASS")
    compiled = [r for r in rows if r.get("syntax_all_ok")]
    matched = [r for r in compiled if r.get("testbench_ok")]
    checks = [r for r in rows if r.get("translation_passed")]
    chk = Counter(_checks_cause(r) for r in matched if not r.get("translation_passed"))
    cat = Counter(((r.get("synth_target") or {}).get("failure_class") or (r.get("synth_target") or {}).get("status")) for r in checks if (r.get("synth_target") or {}).get("status") != "PASS")
    direct = sum(1 for r in checks if (r.get("synth_target") or {}).get("status") == "PASS")
    rescued = sum(1 for r in checks if any(x.get("status") == "PASS" for x in r.get("synth_target_degraded") or []))
    lines = ["## 2. Funnel and why designs are lost", "",
             "Oracle and Vitis judge the extracted original and are recorded, not used as filters; the translation stages run on every extracted design.", "",
             "| stage | passed | lost, by cause |", "|---|---|---|",
             f"| extracted | {n} | repositories with nothing standalone to extract are listed with 0 designs |",
             f"| original testbench passes (oracle) | {n - sum(oracle.values())} | {fmt(oracle)} (extraction defects: signature mismatches, missing headers, stream-model testbenches run outside the simulator) |",
             f"| Vitis synthesizes the original | {n - sum(vitis.values())} | {fmt(vitis)} (scripts with unexpanded variables or a wrong top, include trees not copied, 600 s budget) |",
             f"| translation compiles | {len(compiled)} | {n - len(compiled)} fail the clang check after the agent's last attempt |",
             f"| translated testbench matches | {len(matched)} | {len(compiled) - len(matched)} differ from the original's output or crash |",
             f"| all static checks | {len(checks)} | {fmt(chk)} (fence = agent edited outside the residue list; dialect = Vitis constructs left; missing-file = no report or driver) |",
             f"| target synthesizes, direct | {direct} | {fmt(cat)} (tool limits named by the tool's own message) |",
             f"| after recovery rounds | {direct + rescued} | +{rescued} rescued by a log-driven edit |"]
    return "\n".join(lines)


def section_per_repo(repos: list[dict]) -> str:
    lines = ["## 3. Per repository: time, resources, parallelism", "",
             "| repository | designs | model | wall | extract | per design mean / max | oracle | translate | Vitis | target | recovery | load1 mean / max | mem GB mean / max | containers mean / max | extract $ | agent $ | total $ |",
             "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for r in repos:
        st, rows = r["stats"], r["rows"]
        res = st.get("resources", {})
        secs = [x.get("seconds") for x in rows if x.get("seconds") is not None]
        stage = {k: _mean([x["stage_s"][k] for x in rows if x.get("stage_s", {}).get(k) is not None]) for k in STAGES}
        extract = "reused" if st.get("extract_reused") else _hms(st.get("stage_s", {}).get("extract"))
        cost = round(sum(float(x.get("cost_usd") or 0) for x in rows), 2)
        ecost = round(float((st.get("extract_usage") or {}).get("cost_usd") or 0), 2)
        lines.append(f"| {r['name']} | {len(rows)} | {st.get('model', '-').split('/')[-1]} | {_hms(st.get('wall_s'))} | {extract} | {_hms(_mean(secs))} / {_hms(max(secs) if secs else None)} | "
                     + " | ".join(_hms(stage[k]) for k in STAGES)
                     + f" | {res.get('load1_mean', '-')} / {res.get('load1_max', '-')} | {res.get('mem_used_gb_mean', '-')} / {res.get('mem_used_gb_max', '-')} | {res.get('containers_mean', '-')} / {res.get('containers_max', '-')} | {ecost}{'*' if st.get('extract_reused') else ''} | {cost} | {round(ecost + cost, 2)} |")
    allrows = [x for r in repos for x in r["rows"]]
    total_wall = sum(r["stats"].get("wall_s") or 0 for r in repos)
    cost = sum(float(x.get("cost_usd") or 0) for x in allrows)
    tin, tout = sum(int(x.get("tokens_in") or 0) for x in allrows), sum(int(x.get("tokens_out") or 0) for x in allrows)
    passed = sum(1 for x in allrows if x.get("translation_passed")); synth = sum(1 for x in allrows if (x.get("synth_target") or {}).get("status") == "PASS")
    ecost = sum(float((r["stats"].get("extract_usage") or {}).get("cost_usd") or 0) for r in repos)
    lines.append(f"| **total, {len(repos)} repositories** | {len(allrows)} | | {_hms(total_wall)} | | {_hms(_mean([x['seconds'] for x in allrows if x.get('seconds') is not None]))} / | | | | | | | | | {ecost:.2f} | {cost:.2f} | {ecost + cost:.2f} |")
    lines += ["", f"Total wall time over the repositories with a stats record: {_hms(total_wall)}. Stage columns are per-design means; `translate` is the pre-pass, every agent attempt and its checks. "
              "`extract $` is the extraction agent's API cost (* = extraction reused from the previous pass, cost incurred then); `agent $` covers every translation attempt.",
              f"API cost ${ecost + cost:.2f} in total; the translation agent's ${cost:.2f} is {tin / 1e6:.1f} M input and {tout / 1e6:.2f} M output tokens, "
              f"${cost / len(allrows):.3f} per design, ${cost / passed:.3f} per design passing the checks, ${cost / synth:.3f} per design synthesizing in the target." if allrows and passed and synth else ""]
    return "\n".join(lines)


def section_stages(repos: list[dict]) -> str:
    rows = [x for r in repos for x in r["rows"] if x.get("stage_s")]
    lines = ["## 4. Time per stage, all designs", "", "| stage | designs | mean | median | max | share of design time |", "|---|---|---|---|---|---|"]
    total = sum(sum(x["stage_s"].values()) for x in rows) or 1
    for k in STAGES:
        xs = [x["stage_s"][k] for x in rows if x["stage_s"].get(k) is not None]
        if xs:
            lines.append(f"| {STAGE_NAMES[k]} | {len(xs)} | {_hms(_mean(xs))} | {_hms(statistics.median(xs))} | {_hms(max(xs))} | {100 * sum(xs) / total:.0f}% |")
    att = [a for x in rows for a in (x.get("attempts") or []) if "agent_s" in a]
    if att:
        lines += ["", f"Per attempt (n={len(att)}): agent {_hms(_mean([a['agent_s'] for a in att]))} mean, checks {_hms(_mean([a['check_s'] for a in att]))} mean."]
    if not rows:
        lines += ["", "No stage timings yet: the rows predate the instrumentation."]
    return "\n".join(lines)


def retry_table(rows: list[dict]) -> list[str]:
    rs = retry_stats(rows)
    n, fp = rs["designs"], rs["first_pass"]
    budget = max([len(x.get("attempts") or []) for x in rows] or [0])
    lines = ["", "",
             f"{n} designs with an attempts record, budget {budget} attempt(s). `still failing` is the share that had not passed before attempt k; `rescued` is the share of those that attempt k turned into a pass.",
             "", "| attempt | pass first at this attempt | cumulative passed | still failing before it | rescued by it |", "|---|---|---|---|---|"]
    cum, failing = 0, n
    for k in range(1, budget + 1):
        got = fp.get(k, 0)
        lines.append(f"| {k} | {got} | {cum + got} / {n} ({100 * (cum + got) / n:.0f}%) | {failing} | {100 * got / failing:.0f}% |" if failing else f"| {k} | 0 | {cum} / {n} | 0 | - |")
        cum += got
        failing -= got
    best = max(((k, fp.get(k, 0)) for k in range(2, budget + 1)), key=lambda t: t[1], default=None)
    if best and best[1]:
        lines += ["", f"Among retries, attempt {best[0]} rescues the most designs ({best[1]}); {len(rs['never'])} never pass within the budget."]
    return lines


def section_stage_cost(repos: list[dict]) -> str:
    """API dollars and machine time per stage. Only the two agents cost API money; everything else is container time."""
    rows = [x for r in repos for x in r["rows"]]
    rs = retry_stats(rows)
    ext_cost = sum(float((r["stats"].get("extract_usage") or {}).get("cost_usd") or 0) for r in repos)
    ext_wall = sum(float((r["stats"].get("extract_usage") or {}).get("wall_s") or 0) for r in repos)
    att_cost: dict[int, float] = {}
    att_wall: dict[int, float] = {}
    for x in rows:
        for u in x.get("usage_attempts") or []:
            k = int(u.get("attempt") or 1)
            att_cost[k] = att_cost.get(k, 0.0) + float(u.get("cost_usd") or 0)
            att_wall[k] = att_wall.get(k, 0.0) + float(u.get("wall_s") or 0)
    def stage_sum(k: str) -> float:
        return sum(float(x["stage_s"].get(k) or 0) for x in rows if x.get("stage_s"))
    check_wall = sum(float(a.get("check_s") or 0) for x in rows for a in (x.get("attempts") or []))
    lines = ["## 5. Cost and time per stage", "", "| stage | API cost $ | share of API cost | machine time | notes |", "|---|---|---|---|---|"]
    total_api = ext_cost + sum(att_cost.values()) or 1
    lines.append(f"| extraction (HLSFactory-Agent) | {ext_cost:.2f} | {100 * ext_cost / total_api:.0f}% | {_hms(ext_wall)} | one agent session per repository; reused extractions carry the cost of the pass that made them |")
    for k in sorted(att_cost):
        got = rs["first_pass"].get(k, 0)
        per = f"${att_cost[k] / got:.3f} per design it made pass" if got else "no design first passed here"
        lines.append(f"| translation, attempt {k} | {att_cost[k]:.2f} | {100 * att_cost[k] / total_api:.0f}% | {_hms(att_wall[k])} | {sum(1 for x in rows if any(int(u.get('attempt') or 1) == k for u in x.get('usage_attempts') or []))} designs reached it; {per} |")
    lines.append(f"| oracle + static and container checks | 0 | 0% | {_hms(stage_sum('oracle') + check_wall)} | g++ and the testbench in the container |")
    lines.append(f"| Vitis synthesis of the source | 0 | 0% | {_hms(stage_sum('synth_source'))} | tool container; cached by content hash |")
    lines.append(f"| target synthesis | 0 | 0% | {_hms(stage_sum('synth_target'))} | tool container; only designs that passed the checks |")
    lines.append(f"| recovery rounds | 0 | 0% | {_hms(stage_sum('recovery'))} | one tool run per round |")
    lines += ["", f"API cost in total: ${total_api:.2f}. Machine time is the sum over designs (three run concurrently), so it exceeds the wall clock."]
    return "\n".join(lines)


def section_retries(repos: list[dict], baseline: list[dict] | None = None) -> str:
    lines = ["## 6. Retry curve: at which attempt designs pass the checks", ""] + retry_table([x for r in repos for x in r["rows"]])[2:]
    if baseline:
        lines += ["", "Baseline (the previous pass of the same repositories):", ""] + retry_table([x for r in baseline for x in r["rows"]])[2:]
    return "\n".join(lines)


def section_iteration(repos: list[dict]) -> str:
    rows = [x for r in repos for x in r["rows"]]
    rs = retry_stats(rows)
    cats = Counter(failure_category(a.get("failures") or []) for x in rows for a in (x.get("attempts") or []) if not a.get("passed"))
    lines = ["## 7. How attempts move between failure classes", "",
             "Failure class of a failed attempt (from the harness message the retry prompt carries) and what the next attempt did with it: `pass`, `same` (the same class again), or another class.", "",
             "| failed attempts by class | count |", "|---|---|"] + [f"| {c} | {k} |" for c, k in cats.most_common()]
    lines += ["", "| transition | count |", "|---|---|"] + [f"| {t} | {k} |" for t, k in sorted(rs["transitions"].items(), key=lambda kv: -kv[1])]
    return "\n".join(lines)


def section_thrashing(repos: list[dict]) -> str:
    rows = [x for r in repos for x in r["rows"]]
    rs = retry_stats(rows)
    by_cat = Counter(t["category"] for t in rs["thrashing"])
    lines = ["## 8. Thrashing designs", "",
             f"{len(rs['never'])} designs never pass within the budget; {len(rs['thrashing'])} of them fail every attempt with the identical failure list (the retry changed nothing the checks could see).", ""]
    if by_cat:
        lines += ["| class | thrashing designs |", "|---|---|"] + [f"| {c} | {k} |" for c, k in by_cat.most_common()] + ["", "| design | attempts | class | failure |", "|---|---|---|---|"]
        lines += [f"| {t['design']} | {t['attempts']} | {t['category']} | {t['failure']} |" for t in sorted(rs["thrashing"], key=lambda t: (t["category"], t["design"]))]
    return "\n".join(lines)


def section_synthesis(repos: list[dict]) -> str:
    rows = [x for r in repos for x in r["rows"]]
    st = Counter((x.get("synth_target") or {}).get("status", "not run") for x in rows)
    cls = Counter((x.get("synth_target") or {}).get("failure_class") for x in rows if (x.get("synth_target") or {}).get("status") == "FAILED")
    steps = Counter(); rescued = Counter()
    for x in rows:
        for d in x.get("synth_target_degraded") or []:
            steps[d.get("step", "?").split(":")[0]] += 1
            if d.get("status") == "PASS":
                rescued[d.get("step", "?").split(":")[0]] += 1
    lines = ["## 9. Synthesis and recovery", "", "| target synthesis | designs |", "|---|---|"] + [f"| {k} | {v} |" for k, v in st.most_common()]
    lines += ["", "| failure class | designs |", "|---|---|"] + [f"| {k} | {v} |" for k, v in cls.most_common()]
    lines += ["", "| recovery step | applied | rescued |", "|---|---|---|"] + [f"| {k} | {v} | {rescued.get(k, 0)} |" for k, v in steps.most_common()]
    return "\n".join(lines)


def build(repos: list[dict], title: str, baseline: list[dict] | None = None) -> str:
    parts = [f"# {title}", "", "Generated by `exp/run_translate/stats_report.py`; numbers come from `summary.json` under results/ and `stats/*.json` under the runs directory.", ""]
    for section in (section_method, section_funnel, section_per_repo, section_stages, section_stage_cost, section_retries, section_iteration, section_thrashing, section_synthesis):
        parts += [section(repos, baseline) if section is section_retries else section(repos), ""]
    return "\n".join(parts)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--runs-dir", type=Path, default=DIR_CURRENT / "runs" / "census2")
    parser.add_argument("--results-dir", type=Path, default=DIR_CURRENT / "results" / "census2")
    parser.add_argument("--out", type=Path, default=DIR_CURRENT.parent.parent / "docs" / "RUN_STATISTICS.md")
    parser.add_argument("--title", default="Run statistics: census second pass")
    parser.add_argument("--baseline-results", type=Path, default=None, help="results dir of a previous pass; its retry curve is shown for comparison")
    args = parser.parse_args()
    repos = load(args.runs_dir, args.results_dir)
    baseline = load(args.runs_dir, args.baseline_results) if args.baseline_results else None
    args.out.write_text(build(repos, args.title, baseline), encoding="utf-8")
    print(f"{args.out}: {len(repos)} repositories, {sum(len(r['rows']) for r in repos)} designs")


if __name__ == "__main__":
    main()
