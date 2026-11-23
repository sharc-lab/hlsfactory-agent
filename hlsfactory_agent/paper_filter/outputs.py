"""Write the run outputs: handoff list, review list, summary table, stats (CSV/JSON only)."""

import csv
import json
from collections import Counter
from pathlib import Path


def _flatten(results: list[dict]):
    for r in results:
        for repo in r.get("repos", []):
            yield r, repo


def _dialects(repo: dict) -> str:
    by = (repo.get("scan") or {}).get("by_dialect") or {}
    return ";".join(f"{k}={v}" for k, v in by.items() if v)


def _group(r: dict) -> str:
    return (r.get("meta") or {}).get("group") or ""


def review_reasons(repo: dict) -> list[str]:
    """Why a repo needs a human look (empty: nothing to review)."""
    reasons = []
    flags = repo.get("flags", [])
    if repo["outcome"] == "hls_found" and not repo["handoff"]:
        reasons.append(f"hls_found_but_{repo['confidence']}_confidence")
    for f in ("tapa_only", "tapa_dominant", "weak_hls_markers_only"):
        if f in flags:
            reasons.append(f)
    if repo["outcome"] == "needs_review":
        reasons.append("needs_review")
    return reasons


def build_outputs(run_dir: Path, results: list[dict]) -> dict:
    run_dir = Path(run_dir)
    run_dir.mkdir(parents=True, exist_ok=True)

    # handoff: high-confidence hls_found repos, deduplicated across papers
    handoff: dict[str, dict] = {}
    for r, repo in _flatten(results):
        if not repo["handoff"]:
            continue
        key = repo["repo_id"].lower()
        e = handoff.setdefault(
            key,
            {"repo_id": repo["repo_id"], "papers": [], "provenance": repo["provenance"],
             "confidence": repo["confidence"], "dialects": _dialects(repo),
             "flags": ";".join(repo.get("flags", []))},
        )  # fmt: skip
        e["papers"].append(r["paper_id"])
    ids = sorted(e["repo_id"] for e in handoff.values())
    (run_dir / "handoff_repos.txt").write_text("".join(i + "\n" for i in ids))
    with open(run_dir / "handoff.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["repo_id", "provenance", "confidence", "dialects", "flags", "papers"])
        for e in sorted(handoff.values(), key=lambda e: e["repo_id"].lower()):
            w.writerow([e["repo_id"], e["provenance"], e["confidence"], e["dialects"],
                        e["flags"], ";".join(e["papers"])])  # fmt: skip

    # review list
    review_rows = []
    for r, repo in _flatten(results):
        reasons = review_reasons(repo)
        if reasons:
            quotes = " | ".join(f"{e['type']}: {e['quote'][:100]}" for e in repo["evidence"])
            review_rows.append([
                r["paper_id"], r["title"], repo["repo_id"], repo["provenance"],
                repo["outcome"], repo["confidence"], ";".join(reasons),
                ";".join(repo.get("flags", [])), _dialects(repo), quotes,
            ])  # fmt: skip
    with open(run_dir / "review_list.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["paper_id", "title", "repo_id", "provenance", "outcome", "confidence",
                    "reasons", "flags", "dialects", "evidence"])  # fmt: skip
        w.writerows(review_rows)

    # summary: one row per paper
    with open(run_dir / "summary.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["paper_id", "group", "title", "status", "outcome", "needs_ocr", "repos",
                    "handoff_repos", "problems"])  # fmt: skip
        for r in results:
            repos = r.get("repos", [])
            w.writerow([
                r["paper_id"], _group(r), r.get("title", ""), r["status"], r.get("outcome") or "",
                r.get("needs_ocr", ""), ";".join(x["repo_id"] for x in repos),
                ";".join(x["repo_id"] for x in repos if x["handoff"]),
                len(r.get("problems", [])) or r.get("error", "")[:120].replace("\n", " "),
            ])  # fmt: skip

    stats = {
        "papers": len(results),
        "by_status": dict(Counter(r["status"] for r in results)),
        "by_outcome": dict(Counter(r.get("outcome") or "none" for r in results)),
        "needs_ocr": sum(1 for r in results if r.get("needs_ocr")),
        "handoff_repos": len(ids),
        "review_rows": len(review_rows),
        "papers_with_problems": sum(1 for r in results if r.get("problems")),
    }
    groups = sorted({_group(r) for r in results} - {""})
    if groups:  # e.g. with_repo / without_repo test folders
        stats["by_group"] = {
            g: {
                "papers": sum(1 for r in results if _group(r) == g),
                "by_outcome": dict(Counter(
                    r.get("outcome") or r["status"] for r in results if _group(r) == g)),
                "with_handoff": sum(1 for r in results if _group(r) == g
                                    and any(x["handoff"] for x in r.get("repos", []))),
            }
            for g in groups
        }  # fmt: skip
    (run_dir / "stats.json").write_text(json.dumps(stats, indent=2))
    return stats
