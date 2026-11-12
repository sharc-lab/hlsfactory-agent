"""Run the parameterization agent on one or more standalone Vitis HLS design folders.

Example:
    python exp/run_parameterize/run.py
    python exp/run_parameterize/run.py --design exp/run_translate/designs/vitis_mac --model deepseek/deepseek-v4-flash
"""

import argparse
import json
from pathlib import Path

from dotenv import dotenv_values

from hlsfactory_agent.parameterize import HLSParameterizationRun
from hlsfactory_agent.utils import check_key

DIR_CURRENT = Path(__file__).resolve().parent
DIR_DESIGNS = DIR_CURRENT / "designs"
DIR_RUNS = DIR_CURRENT / "runs"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        "--design", action="append", type=Path, help="design folder; repeatable; default: all under designs/"
    )
    parser.add_argument("--model", default="deepseek/deepseek-v4-flash")
    parser.add_argument("--runs-dir", type=Path, default=DIR_RUNS)
    parser.add_argument("--env", type=Path, default=Path(".env"))
    parser.add_argument(
        "--attempts", type=int, default=1, help="agent attempts per design; a failed check feeds the retry prompt"
    )
    parser.add_argument("--max-points", type=int, default=8, help="cap on design-space points, explicit or implicit")
    parser.add_argument("--agent-timeout", type=int, default=60 * 30, help="seconds per agent attempt")
    args = parser.parse_args()

    api_key = check_key(dotenv_values(args.env).get("OPENROUTER_API_KEY"))

    designs = args.design or sorted(p for p in DIR_DESIGNS.iterdir() if p.is_dir())
    if not designs:
        raise SystemExit(f"No design folders found under {DIR_DESIGNS}; pass --design explicitly.")
    args.runs_dir.mkdir(parents=True, exist_ok=True)

    summary: dict[str, dict] = {}
    for dir_design in designs:
        run_id = f"parameterize-{dir_design.name}"
        run = HLSParameterizationRun(
            run_id=run_id,
            dir_design=dir_design,
            dir_work=args.runs_dir / run_id,
            model_name=args.model,
            api_key=api_key,
            attempts=args.attempts,
            max_design_points=args.max_points,
            agent_timeout_s=args.agent_timeout,
        )
        check = run.run()
        summary[run_id] = {
            "passed": check.get("passed"),
            "failures": check.get("static", {}).get("failures", []),
            "attempts_used": check.get("attempts_used"),
            "n_points": len(check.get("static", {}).get("points", [])),
            "summary": check.get("summary"),
        }

    (args.runs_dir / "summary.json").write_text(json.dumps(summary, indent=4, default=str))
    print(json.dumps(summary, indent=2, default=str))


if __name__ == "__main__":
    main()
