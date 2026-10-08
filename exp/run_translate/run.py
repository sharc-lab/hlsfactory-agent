"""Run the translation agent on one or more standalone Vitis HLS design folders.

Example:
    python exp/run_translate/run.py --target catapult
    python exp/run_translate/run.py --design exp/run_translate/designs/vitis_mac --model deepseek/deepseek-v4-flash
"""

import argparse
import json
from pathlib import Path

from dotenv import dotenv_values

from dataclasses import asdict, replace

from hlsfactory_agent.synth import run_synth_cached, run_synth_degraded
from hlsfactory_agent.targets.vitis import VITIS
from hlsfactory_agent.translate import OUTPUT_DIR_NAME, TARGETS, HLSTranslationRun, repair_top
from hlsfactory_agent.utils import check_key

DIR_CURRENT = Path(__file__).resolve().parent
DIR_DESIGNS = DIR_CURRENT / "designs"
DIR_RUNS = DIR_CURRENT / "runs"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--design", action="append", type=Path, help="design folder; repeatable; default: all under designs/")
    parser.add_argument("--target", default="catapult", choices=sorted(TARGETS))
    parser.add_argument("--model", default="deepseek/deepseek-v4-flash")
    parser.add_argument("--runs-dir", type=Path, default=DIR_RUNS)
    parser.add_argument("--env", type=Path, default=Path(".env"))
    parser.add_argument("--attempts", type=int, default=1, help="agent attempts per design; a failed check feeds the retry prompt")
    parser.add_argument("--no-translate", action="store_true", help="path A only: extract, scan, feasibility, oracle; no pre-pass, agent or checks")
    parser.add_argument("--synth", nargs="?", const="both", default="none", choices=("both", "source", "target", "none"),
                        help="off by default; `--synth` alone synthesizes the source with Vitis and the translation with the target tool, in containers with the tools mounted read-only; cached by content")
    parser.add_argument("--agent-tools", action="store_true", help="also mount the vendor tools into the agent's own container (off: the agent can never call a tool)")
    parser.add_argument("--require-source-synth", action="store_true", help="skip translating a design whose Vitis synthesis did not pass")
    parser.add_argument("--synth-timeout", type=int, default=None, help="seconds per target synthesis; default is the target's own (Catapult 1200)")
    parser.add_argument("--degrade", type=int, default=2, help="recovery rounds after a failed target synthesis (0 = off); each round applies one edit the tool's log parameterizes")
    args = parser.parse_args()

    api_key = check_key(dotenv_values(args.env).get("OPENROUTER_API_KEY"))

    designs = args.design or sorted(p for p in DIR_DESIGNS.iterdir() if p.is_dir())
    args.runs_dir.mkdir(parents=True, exist_ok=True)

    summary = {}
    cache = args.runs_dir / ".synth_cache"
    target = TARGETS[args.target]
    if args.synth_timeout:
        target = replace(target, synth_timeout_s=args.synth_timeout)
    for dir_design in designs:
        run_id = f"translate-{args.target}-{dir_design.name}"
        entry: dict = {"checks_passed": None, "synth_source": {"status": "not run"}, "synth_target": {"status": "not run"}}
        entry["top"] = repair_top(dir_design)  # extraction may have named a top that does not exist
        summary[run_id] = entry
        if args.synth in ("both", "source"):
            src = run_synth_cached(VITIS, dir_design, args.runs_dir / "synth_source" / dir_design.name, cache)
            entry["synth_source"] = asdict(src)
            if args.require_source_synth and src.status != "PASS":
                entry["skipped"] = f"source synthesis {src.status}"
                continue
        if args.no_translate:
            entry["skipped"] = "translation off"
            continue
        run = HLSTranslationRun(
            run_id=run_id,
            dir_design=dir_design,
            dir_work=args.runs_dir / run_id,
            model_name=args.model,
            api_key=api_key,
            target=args.target,
            attempts=args.attempts,
            agent_tools=args.agent_tools,
        )
        check = run.run()
        entry.update(
            checks_passed=check["passed"],
            static_failures=check["static"]["failures"],
            syntax_all_ok=check["container"]["syntax_all_ok"],
            testbench_ok=check["container"]["testbench_ok"],
        )
        if args.synth in ("both", "target") and check["passed"] and target.parse_report:
            dir_run = args.runs_dir / run_id
            tgt = run_synth_cached(target, dir_run / "run_area" / OUTPUT_DIR_NAME, dir_run / "synth", cache)
            entry["synth_target"] = asdict(tgt)
            if tgt.status == "FAILED" and args.degrade:
                entry["synth_target_degraded"] = run_synth_degraded(target, dir_run / "run_area" / OUTPUT_DIR_NAME, dir_run / "synth", cache, rounds=args.degrade)
            (dir_run / "synth.json").write_text(json.dumps({k: entry[k] for k in ("synth_target", "synth_target_degraded") if k in entry}, indent=4), encoding="utf-8")

    (args.runs_dir / "summary.json").write_text(json.dumps(summary, indent=4))
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
