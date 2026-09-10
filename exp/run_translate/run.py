"""Run the translation agent on one or more standalone Vitis HLS design folders.

Example:
    python exp/run_translate/run.py --target catapult
    python exp/run_translate/run.py --design exp/run_translate/designs/vitis_mac --model deepseek/deepseek-v4-flash
"""

import argparse
import json
from pathlib import Path

from dotenv import dotenv_values

from hlsfactory_agent.translate import TARGETS, HLSTranslationRun
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
    args = parser.parse_args()

    api_key = check_key(dotenv_values(args.env).get("OPENROUTER_API_KEY"))

    designs = args.design or sorted(p for p in DIR_DESIGNS.iterdir() if p.is_dir())
    args.runs_dir.mkdir(parents=True, exist_ok=True)

    summary = {}
    for dir_design in designs:
        run_id = f"translate-{args.target}-{dir_design.name}"
        run = HLSTranslationRun(
            run_id=run_id,
            dir_design=dir_design,
            dir_work=args.runs_dir / run_id,
            model_name=args.model,
            api_key=api_key,
            target=args.target,
        )
        check = run.run()
        summary[run_id] = {
            "passed": check["passed"],
            "static_failures": check["static"]["failures"],
            "syntax_all_ok": check["container"]["syntax_all_ok"],
            "testbench_ok": check["container"]["testbench_ok"],
        }

    (args.runs_dir / "summary.json").write_text(json.dumps(summary, indent=4))
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
