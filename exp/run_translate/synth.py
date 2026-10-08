"""Synthesize finished translation runs with the target tool (the synthesis gate).

Runs where the translation checks passed are synthesized in a fresh copy at <run>/synth/;
results go to <run>/synth.json and <runs-dir>/synth_summary.json.

Example:
    python exp/run_translate/synth.py --runs-dir exp/run_translate/runs -j 3
"""

import argparse
import json
import shutil
from concurrent.futures import ThreadPoolExecutor
from dataclasses import asdict, replace
from pathlib import Path

from hlsfactory_agent.spec import TargetSpec
from hlsfactory_agent.synth import run_synth, run_synth_degraded
from hlsfactory_agent.translate import OUTPUT_DIR_NAME, TARGETS

DIR_RUNS = Path(__file__).resolve().parent / "runs"


def find_output(dir_run: Path, target: str) -> Path | None:
    for d in (dir_run / "run_area" / OUTPUT_DIR_NAME, dir_run / target):
        if d.is_dir():
            return d
    return None


def synth_one(dir_run: Path, target: TargetSpec, force: bool, degrade: int = 0) -> dict:
    f_check = dir_run / "check_data.json"
    check = json.loads(f_check.read_text(encoding="utf-8")) if f_check.exists() else {}
    dir_out = find_output(dir_run, target.name)
    if dir_out is None:
        result = {"status": "SKIPPED", "reason": "no translated design"}
    elif target.parse_report is None:
        result = {"status": "SKIPPED", "reason": f"{target.name} has no synthesis step yet"}
    elif not check.get("passed") and not force:
        result = {"status": "SKIPPED", "reason": "translation checks failed"}
    else:
        dir_synth = dir_run / "synth"
        shutil.rmtree(dir_synth, ignore_errors=True)
        shutil.copytree(dir_out, dir_synth)
        first = run_synth(target, dir_synth)
        result = {"synth_target": asdict(first), "synth_timeout_s": target.synth_timeout_s}
        if first.status != "PASS" and degrade:
            result["synth_target_degraded"] = run_synth_degraded(target, dir_out, dir_synth, dir_run / ".synth_cache", rounds=degrade)
    (dir_run / "synth.json").write_text(json.dumps(result, indent=4), encoding="utf-8")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--runs-dir", type=Path, default=DIR_RUNS)
    parser.add_argument("-j", "--jobs", type=int, default=1, help="parallel tool runs (each holds a license)")
    parser.add_argument("--force", action="store_true", help="also synthesize runs whose checks failed")
    parser.add_argument("--only", default="", help="comma-separated design names; others are skipped")
    parser.add_argument("--synth-timeout", type=int, default=None, help="seconds per synthesis; default is the target's own")
    parser.add_argument("--degrade", type=int, default=0, help="recovery rounds after a failed synthesis")
    args = parser.parse_args()
    only = {n for n in args.only.split(",") if n}

    runs = {}
    for d in sorted(args.runs_dir.glob("translate-*")):
        target = next((t for t in TARGETS if d.name.startswith(f"translate-{t}-")), None)
        if d.is_dir() and target and (not only or d.name.split(f"translate-{target}-", 1)[1] in only):
            spec = TARGETS[target]
            runs[d.name] = (d, replace(spec, synth_timeout_s=args.synth_timeout) if args.synth_timeout else spec)

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = {name: pool.submit(synth_one, d, t, args.force, args.degrade) for name, (d, t) in runs.items()}
        summary = {name: f.result() for name, f in futures.items()}

    (args.runs_dir / "synth_summary.json").write_text(json.dumps(summary, indent=4), encoding="utf-8")
    passed = 0
    for name, r in summary.items():
        t = r.get("synth_target", r)
        rescued = [x["step"] for x in r.get("synth_target_degraded", []) if x["status"] == "PASS"]
        ok = t.get("status") == "PASS" or bool(rescued)
        passed += ok
        detail = t.get("failure_class") or t.get("reason") or f"latency={t.get('latency')} area={t.get('area')}"
        print(f"{t.get('status', '?'):8} {name:50} {detail}{' rescued by ' + rescued[0] if rescued else ''}")
    print(f"{passed}/{len(summary)} synthesized")

if __name__ == "__main__":
    main()
