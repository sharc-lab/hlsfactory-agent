"""Run the extraction step over a folder of PDFs.

    python exp/paper_filter/run_extract.py [--pdfs DIR] [--out DIR] [--no-github]

Writes one JSON per PDF to --out; PDFs that already have an output are skipped.
"""

import argparse
import json
from pathlib import Path

from hlsfactory_agent.paper_filter.extract import extract_candidates
from hlsfactory_agent.paper_filter.github import load_token

DIR_CURRENT = Path(__file__).resolve().parent

parser = argparse.ArgumentParser()
parser.add_argument("--pdfs", type=Path, default=DIR_CURRENT / "pdfs")
parser.add_argument("--out", type=Path, default=DIR_CURRENT / "runs")
parser.add_argument("--no-github", action="store_true", help="skip GitHub existence checks")
parser.add_argument("--force", action="store_true", help="redo PDFs that already have output")
args = parser.parse_args()

args.out.mkdir(parents=True, exist_ok=True)
token = load_token()
if not args.no_github and token is None:
    print("warning: no GITHUB_TOKEN set; unauthenticated GitHub limit is 60 requests/hour")

for pdf in sorted(args.pdfs.glob("*.pdf")):
    out_file = args.out / (pdf.stem + ".json")
    if out_file.exists() and not args.force:
        print(f"skip {pdf.name} (done)")
        continue
    result = extract_candidates(
        pdf,
        github_token=token,
        github_cache=args.out / "_github_cache",
        check_github=not args.no_github,
    )
    out_file.write_text(json.dumps(result, indent=2))
    print(f"{pdf.name[:40]}: {[c['repo_id'] for c in result['candidates']]}")
