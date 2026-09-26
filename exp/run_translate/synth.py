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
from dataclasses import asdict
from pathlib import Path

from hlsfactory_agent.synth import run_synth
from hlsfactory_agent.translate import OUTPUT_DIR_NAME, TARGETS

DIR_RUNS = Path(__file__).resolve().parent / "runs"


def find_output(dir_run: Path, target: str) -> Path | None:
    for d in (dir_run / "run_area" / OUTPUT_DIR_NAME, dir_run / target):
        if d.is_dir():
            return d
    return None


def synth_one(dir_run: Path, target: str, force: bool) -> dict:
    f_check = dir_run / "check_data.json"
    check = json.loads(f_check.read_text(encoding="utf-8")) if f_check.exists() else {}
    dir_out = find_output(dir_run, target)
    if dir_out is None:
        result = {"status": "SKIPPED", "reason": "no translated design"}
    elif TARGETS[target].parse_report is None:
        result = {"status": "SKIPPED", "reason": f"{target} has no synthesis step yet"}
    elif not check.get("passed") and not force:
        result = {"status": "SKIPPED", "reason": "translation checks failed"}
    else:
        dir_synth = dir_run / "synth"
        shutil.rmtree(dir_synth, ignore_errors=True)
        shutil.copytree(dir_out, dir_synth)
        result = asdict(run_synth(TARGETS[target], dir_synth))
    (dir_run / "synth.json").write_text(json.dumps(result, indent=4), encoding="utf-8")
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--runs-dir", type=Path, default=DIR_RUNS)
    parser.add_argument("-j", "--jobs", type=int, default=1, help="parallel tool runs (each holds a license)")
    parser.add_argument("--force", action="store_true", help="also synthesize runs whose checks failed")
    args = parser.parse_args()

    runs = {}
    for d in sorted(args.runs_dir.glob("translate-*")):
        target = next((t for t in TARGETS if d.name.startswith(f"translate-{t}-")), None)
        if d.is_dir() and target:
            runs[d.name] = (d, target)

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = {name: pool.submit(synth_one, d, t, args.force) for name, (d, t) in runs.items()}
        summary = {name: f.result() for name, f in futures.items()}

    (args.runs_dir / "synth_summary.json").write_text(json.dumps(summary, indent=4), encoding="utf-8")
    for name, r in summary.items():
        detail = r.get("failure_class") or r.get("reason") or f"latency={r.get('latency')} area={r.get('area')}"
        print(f"{r['status']:8} {name:50} {detail}")
    passed = sum(r["status"] == "PASS" for r in summary.values())
    print(f"{passed}/{len(summary)} synthesized")


if __name__ == "__main__":
    main()
