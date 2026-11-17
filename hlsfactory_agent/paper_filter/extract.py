from pathlib import Path

from hlsfactory_agent.paper_filter.github import check_repo
from hlsfactory_agent.paper_filter.pdf import PdfDoc, read_pdf
from hlsfactory_agent.paper_filter.urls import merge_hits, scan_annotations, scan_text


def title_from_filename(path: Path) -> str:
    return path.stem.replace("_", " ")


def run_prepass(
    pdf_path: Path,
    title: str | None = None,
    github_token: str | None = None,
    github_cache: Path | None = None,
    check_github: bool = True,
) -> tuple[dict, PdfDoc]:
    """PDF -> candidate repos with evidence, plus the parsed PDF.

    Never drops a candidate; labels it. A scanned PDF (no text layer) is not OCRed: it is
    marked `needs_ocr` and the caller still runs the search path from metadata.
    """
    title = title or title_from_filename(pdf_path)
    doc = read_pdf(pdf_path)
    result: dict = {
        "pdf": pdf_path.name,
        "title": title,
        "pages": len(doc.pages),
        "text_chars": doc.n_chars,
        "scanned": doc.scanned,
        "needs_ocr": doc.scanned,
        "candidates": [],
        "profiles": [],
    }
    if doc.scanned:
        result["note"] = "no text layer; needs OCR (skipped); search from metadata only"
        return result, doc

    scan = scan_text(doc.pages)
    candidates = merge_hits(scan.hits + scan_annotations(doc.links))
    result["profiles"] = scan.profiles

    if check_github:
        for c in candidates:
            c["gh"] = check_repo(c["repo_id"], github_token, github_cache)

    result["candidates"] = candidates
    return result, doc


def extract_candidates(
    pdf_path: Path,
    title: str | None = None,
    github_token: str | None = None,
    github_cache: Path | None = None,
    check_github: bool = True,
) -> dict:
    """Same as run_prepass but returns only the result dict."""
    return run_prepass(pdf_path, title, github_token, github_cache, check_github)[0]
