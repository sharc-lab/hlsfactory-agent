"""Find PDFs in a folder tree and join them to spreadsheet rows.

Join order: normalized title from the filename; then the filename as a DOI (publishers such
as ACM name downloads after the DOI suffix, e.g. 3706628.3708871.pdf for
10.1145/3706628.3708871; a full DOI with "_" for "/" also works); then the title in the PDF's
own metadata. Unmatched PDFs keep a title from the metadata or, failing that, the filename.

PDFs may sit in subfolders (e.g. pdfs/with_repo/, pdfs/without_repo/); the subfolder path is
kept as the paper's `group` so outputs can be broken down by it.
"""

import hashlib
import re
from dataclasses import asdict, dataclass
from pathlib import Path

import pandas as pd
import pymupdf

MIN_PREFIX = 40  # chars of normalized title needed for the truncated-filename fallback
MAX_ID_CHARS = 80


def norm_title(s: object) -> str:
    return re.sub(r"[^a-z0-9]+", "", str(s).lower())


def title_from_filename(path: Path) -> str:
    return path.stem.replace("_", " ")


def make_paper_id(stem: str) -> str:
    """Filesystem-safe id; long stems are truncated plus a short hash to stay unique."""
    if len(stem) <= MAX_ID_CHARS:
        return stem
    return f"{stem[:MAX_ID_CHARS]}-{hashlib.sha1(stem.encode()).hexdigest()[:8]}"


@dataclass
class PaperMeta:
    paper_id: str
    pdf: Path
    title: str
    authors: str | None = None
    year: int | None = None
    venue: str | None = None
    doi: str | None = None
    group: str | None = None  # subfolder under the PDF root, e.g. "with_repo"; None at top level
    matched: bool = False  # joined to a spreadsheet row
    matched_by: str | None = None  # "title" | "doi" | "pdf_title"
    ambiguous: bool = False  # several rows share the normalized title; first was used

    def to_json(self) -> dict:
        d = asdict(self)
        d["pdf"] = str(self.pdf)
        return d


def _clean(v: object):
    return None if v is None or pd.isna(v) else v


def load_sheet(path: Path) -> pd.DataFrame:
    path = Path(path)
    return pd.read_csv(path) if path.suffix.lower() == ".csv" else pd.read_excel(path)


def _index(df: pd.DataFrame) -> dict[str, list[int]]:
    idx: dict[str, list[int]] = {}
    for i, t in enumerate(df["Title"]):
        idx.setdefault(norm_title(t), []).append(i)
    return idx


def norm_doi(s: object) -> str:
    s = str(s).strip().lower()
    return re.sub(r"^(https?://(dx\.)?doi\.org/|doi:)", "", s)


def _doi_index(df: pd.DataFrame) -> tuple[dict[str, list[int]], dict[str, list[int]]]:
    """(full DOI -> rows, DOI suffix after the first '/' -> rows)."""
    full: dict[str, list[int]] = {}
    suffix: dict[str, list[int]] = {}
    if "DOI" not in df:
        return full, suffix
    for i, d in enumerate(df["DOI"]):
        if pd.isna(d) or "/" not in str(d):
            continue
        doi = norm_doi(d)
        full.setdefault(doi, []).append(i)
        suffix.setdefault(doi.split("/", 1)[1], []).append(i)
    return full, suffix


def _lookup_doi(stem: str, full: dict, suffix: dict, df: pd.DataFrame) -> list[int]:
    s = stem.strip().lower()
    if s.startswith("10.") and "_" in s:
        return full.get(s.replace("_", "/", 1), [])
    rows = suffix.get(s, [])
    # the same suffix under two publishers would be a guess: no match
    return rows if len({norm_doi(df.iloc[i]["DOI"]) for i in rows}) == 1 else []


def pdf_metadata_title(pdf: Path) -> str | None:
    """Title from the PDF's metadata, if it looks like a real title."""
    try:
        with pymupdf.open(pdf) as doc:
            t = (doc.metadata or {}).get("title") or ""
    except Exception:
        return None
    t = " ".join(t.split())
    bad = t.lower() in {"untitled", "title", "paper"} or t.lower().endswith((".doc", ".docx", ".pdf", ".dvi"))
    return t if len(t) >= 12 and not bad else None


def _lookup(key: str, idx: dict[str, list[int]]) -> list[int]:
    if key in idx:
        return idx[key]
    if len(key) >= MIN_PREFIX:  # filename may be truncated by the OS or the downloader
        cands = [
            k
            for k in idx
            if len(k) >= MIN_PREFIX and (k.startswith(key) or key.startswith(k))
        ]
        if len(cands) == 1:
            return idx[cands[0]]
    return []


def discover_papers(pdf_dir: Path, sheet: Path | None = None) -> list[PaperMeta]:
    """Every *.pdf under pdf_dir (recursive), joined to the sheet (see module docstring).

    Paper ids come from filename stems, so the same filename in two subfolders is an error.
    """
    pdf_dir = Path(pdf_dir)
    df = load_sheet(sheet) if sheet else None
    idx = _index(df) if df is not None else {}
    doi_full, doi_suffix = _doi_index(df) if df is not None else ({}, {})
    papers = []
    seen: dict[str, Path] = {}
    for pdf in sorted(pdf_dir.rglob("*.pdf")):
        paper_id = make_paper_id(pdf.stem)
        if paper_id in seen:
            raise ValueError(f"duplicate paper id {paper_id!r}: {seen[paper_id]} and {pdf}")
        seen[paper_id] = pdf
        rel_dir = pdf.parent.relative_to(pdf_dir).as_posix()
        meta = PaperMeta(
            paper_id=paper_id, pdf=pdf, title=title_from_filename(pdf),
            group=None if rel_dir == "." else rel_dir,
        )  # fmt: skip
        rows, how = _lookup(norm_title(pdf.stem), idx), "title"
        if not rows and df is not None:
            rows, how = _lookup_doi(pdf.stem, doi_full, doi_suffix, df), "doi"
        md_title = None
        if not rows:
            md_title = pdf_metadata_title(pdf)
            if md_title:
                rows, how = _lookup(norm_title(md_title), idx), "pdf_title"
        if rows and df is not None:
            r = df.iloc[rows[0]]
            meta.title = str(r["Title"])
            meta.authors = _clean(r.get("Authors"))
            year = _clean(r.get("Year"))
            meta.year = int(year) if year is not None else None
            meta.venue = _clean(r.get("Venue"))
            meta.doi = _clean(r.get("DOI"))
            meta.matched = True
            meta.matched_by = how
            meta.ambiguous = len(rows) > 1
        elif md_title:  # not in the sheet: the PDF's own title beats e.g. "3702005"
            meta.title = md_title
        papers.append(meta)
    return papers
