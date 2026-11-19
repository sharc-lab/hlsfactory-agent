"""Turn the agent's raw result.json into a final, trustworthy per-paper record.

The agent supplies repos, evidence (with quotes) and the HLS scan output. This code
verifies quotes against the PDF text, computes confidence, decides each repo's outcome from
the scan, and decides whether it is handed off.
"""

import re

from hlsfactory_agent.paper_filter.evidence import clean_evidence, score

REPO_ID_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9-]*/[A-Za-z0-9_.-]+$")
PROVENANCES = {"own_in_paper", "own_found_by_search", "third_party_demo"}

# Paper-level outcome priority (first match wins), then no_artifact / not_found.
PAPER_PRIORITY = ["hls_found", "needs_review", "already_known", "repo_no_hls", "link_dead"]


def repo_outcome(scan: dict | None, known: bool) -> tuple[str, list[str]]:
    """(outcome, flags) for one repo from its scan and registry status."""
    flags: list[str] = []
    if known:
        return "already_known", flags
    if not scan:
        return "needs_review", ["no_scan"]
    cls = scan.get("classification")
    meta = scan.get("repo_meta") or {}
    if meta.get("archived"):
        flags.append("archived")
    if meta.get("fork"):
        flags.append("fork")
    if scan.get("mixed_tapa"):
        flags.append("mixed_tapa")
    if scan.get("tapa_dominant"):
        flags.append("tapa_dominant")
    if scan.get("partial_scan"):
        flags.append("partial_scan")
    if cls == "hls_found":
        return "hls_found", flags
    if cls == "tapa_only":
        return "repo_no_hls", flags + ["tapa_only"]
    if cls == "repo_no_hls":
        return "repo_no_hls", flags
    if cls == "link_dead":
        return "link_dead", flags
    if cls == "weak_hls_only":
        return "needs_review", flags + ["weak_hls_markers_only"]
    return "needs_review", flags + [f"scan_{cls or 'missing'}"]


def finalize(
    raw: dict,
    paper_id: str,
    title: str,
    pages: list[str],
    prepass: dict,
    snapshot: dict,
) -> dict:
    problems: list[str] = []
    refs_only = {
        c["repo_id"].lower(): c.get("in_references") is True
        for c in prepass.get("candidates", [])
    }
    repos: list[dict] = []
    seen: set[str] = set()
    for r in raw.get("repos") or []:
        rid = str(r.get("repo_id", "")).strip()
        if not REPO_ID_RE.match(rid):
            problems.append(f"dropped invalid repo id {rid!r}")
            continue
        key = rid.lower()
        if key in seen:
            problems.append(f"duplicate repo {rid}")
            continue
        seen.add(key)

        provenance = r.get("provenance")
        if provenance not in PROVENANCES:
            problems.append(f"{rid}: bad provenance {provenance!r}; capped to low")
            provenance = "unknown"
        evidence, ev_problems = clean_evidence(
            r.get("evidence"), pages, repo_in_references_only=refs_only.get(key, False)
        )
        problems += [f"{rid}: {p}" for p in ev_problems]
        confidence = score({e["type"] for e in evidence})
        if provenance == "unknown":
            confidence = "low"

        scan = r.get("scan") if isinstance(r.get("scan"), dict) else None
        outcome, flags = repo_outcome(scan, key in snapshot)
        if provenance == "third_party_demo":
            flags.append("third_party")
        handoff = (
            outcome == "hls_found"
            and confidence == "high"
            and "tapa_only" not in flags
            and "tapa_dominant" not in flags
        )
        repos.append(
            {
                "repo_id": rid,
                "provenance": provenance,
                "outcome": outcome,
                "confidence": confidence,
                "flags": flags,
                "handoff": handoff,
                "evidence": evidence,
                "scan": scan,
                "notes": r.get("notes", ""),
            }
        )

    outcomes = {r["outcome"] for r in repos}
    outcome = next((o for o in PAPER_PRIORITY if o in outcomes), None)
    if outcome is None:
        outcome = "no_artifact" if raw.get("no_artifact") else "not_found"
    return {
        "paper_id": paper_id,
        "title": title,
        "status": "ok",
        "outcome": outcome,
        "needs_ocr": bool(prepass.get("needs_ocr")),
        "repos": repos,
        "no_artifact_reason": raw.get("no_artifact_reason"),
        "searches": raw.get("searches") or [],
        "summary": raw.get("summary", ""),
        "problems": problems,
    }
