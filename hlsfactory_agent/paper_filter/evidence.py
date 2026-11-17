"""Evidence items and the deterministic confidence score.

The agent reports evidence (type, quote, source); this code decides confidence. The model's
own confidence is never used. See CLAUDE.md, chunk 4.
"""

import re

STRONG = {
    "paper_own_link",  # paper body gives the link as the authors' own code
    "paper_names_repo",  # paper body names this repo as a design it builds on / evaluates
    "readme_has_title",  # README has the exact paper title
    "readme_has_doi_or_arxiv",  # README has the paper's DOI / arXiv id / BibTeX
    "git_log_authors_match",  # commit author names match the paper's authors
    "contents_match_paper",  # repo contents match the paper (benchmark dirs, kernels)
}
MEDIUM = {
    "name_matches_system",  # repo name equals a distinctive system name from the paper
    "owner_matches_affiliation",  # owner/org matches an author's affiliation
}
WEAK = {
    "dates_fit",  # created/pushed around the paper's year
    "description_matches_abstract",
}
NEGATIVE = {
    "generic_name",
    "is_fork",
    "predates_paper",
}
ALL_TYPES = STRONG | MEDIUM | WEAK | NEGATIVE
PAPER_LINK_TYPES = {"paper_own_link", "paper_names_repo"}
MIN_QUOTE_CHARS = 8

LEVELS = ["low", "medium", "high"]


def _squash(s: str) -> str:
    """Lowercase alphanumerics only: tolerant of line breaks, hyphenation and spacing."""
    return re.sub(r"[^a-z0-9]+", "", s.lower())


def verify_quote(quote: str, pages: list[str]) -> bool:
    q = _squash(quote)
    return len(q) >= MIN_QUOTE_CHARS and any(q in _squash(p) for p in pages)


def clean_evidence(
    items: object,
    pages: list[str],
    repo_in_references_only: bool = False,
) -> tuple[list[dict], list[str]]:
    """Keep only well-formed, verifiable evidence. Returns (kept, problems)."""
    kept: list[dict] = []
    problems: list[str] = []
    if not isinstance(items, list):
        return kept, ["evidence is not a list"]
    for it in items:
        if not isinstance(it, dict):
            problems.append("evidence item is not an object")
            continue
        etype, quote = it.get("type"), str(it.get("quote") or "").strip()
        source = str(it.get("source") or "")
        if etype not in ALL_TYPES:
            problems.append(f"unknown evidence type {etype!r}")
            continue
        if len(quote) < MIN_QUOTE_CHARS:
            problems.append(f"{etype}: missing or too short quote")
            continue
        item = {"type": etype, "quote": quote, "source": source, "page": it.get("page")}
        if source.lower().startswith("paper"):
            if not verify_quote(quote, pages):
                problems.append(f"{etype}: quote not found in the PDF text")
                continue
            if etype in PAPER_LINK_TYPES and repo_in_references_only:
                problems.append(f"{etype}: link only appears in the references")
                continue
            item["verified"] = True
        elif etype in PAPER_LINK_TYPES:
            problems.append(f"{etype}: must be sourced from the paper")
            continue
        kept.append(item)
    return kept, problems


def score(types: set[str]) -> str:
    """high = any strong, or name + owner match; medium = name match + a weak signal."""
    if types & STRONG or {"name_matches_system", "owner_matches_affiliation"} <= types:
        level = 2
    elif "name_matches_system" in types and types & WEAK:
        level = 1
    else:
        level = 0
    if types & NEGATIVE and not types & PAPER_LINK_TYPES:
        level = min(level, 1)
    return LEVELS[level]
