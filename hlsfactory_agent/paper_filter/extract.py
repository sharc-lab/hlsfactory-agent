from pathlib import Path

from hlsfactory_agent.paper_filter.classify import classify
from hlsfactory_agent.paper_filter.github import check_repo
from hlsfactory_agent.paper_filter.pdf import read_pdf
from hlsfactory_agent.paper_filter.urls import merge_hits, scan_annotations, scan_text


def title_from_filename(path: Path) -> str:
    return path.stem.replace("_", " ")


def extract_candidates(
    pdf_path: Path,
    title: str | None = None,
    github_token: str | None = None,
    github_cache: Path | None = None,
    check_github: bool = True,
    openrouter_key: str | None = None,
) -> dict:
    """PDF -> candidate repos with evidence. Never drops a candidate; labels it."""
    title = title or title_from_filename(pdf_path)
    doc = read_pdf(pdf_path)
    result: dict = {
        "pdf": pdf_path.name,
        "title": title,
        "pages": len(doc.pages),
        "text_chars": doc.n_chars,
        "scanned": doc.scanned,
        "candidates": [],
        "profiles": [],
        "novelty": None,
    }
    if doc.scanned:
        result["note"] = "no text layer; needs OCR (skipped)"
        return result

    scan = scan_text(doc.pages)
    candidates = merge_hits(scan.hits + scan_annotations(doc.links))
    result["profiles"] = scan.profiles

    if check_github:
        for c in candidates:
            c["gh"] = check_repo(c["repo_id"], github_token, github_cache)

    if openrouter_key:
        verdict = classify(title, doc.pages[0], candidates, openrouter_key)
        result["novelty"] = verdict["novelty"]
        result["llm"] = {"model": verdict["model"], "usage": verdict["usage"]}
        for c in candidates:
            role, reason = verdict["roles"].get(c["repo_id"].lower(), ("unsure", "not returned"))
            c["role"], c["role_reason"] = role, reason

    result["candidates"] = candidates
    return result
