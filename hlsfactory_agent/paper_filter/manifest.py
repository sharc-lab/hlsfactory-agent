"""Find PDFs in a folder and join them to spreadsheet rows by normalized title."""

import hashlib
import re
from dataclasses import asdict, dataclass
from pathlib import Path

import pandas as pd

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
    matched: bool = False  # joined to a spreadsheet row
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
    """Every *.pdf in pdf_dir. Unmatched PDFs fall back to a title from the filename."""
    df = load_sheet(sheet) if sheet else None
    idx = _index(df) if df is not None else {}
    papers = []
    for pdf in sorted(Path(pdf_dir).glob("*.pdf")):
        meta = PaperMeta(
            paper_id=make_paper_id(pdf.stem), pdf=pdf, title=title_from_filename(pdf)
        )
        rows = _lookup(norm_title(pdf.stem), idx)
        if rows and df is not None:
            r = df.iloc[rows[0]]
            meta.title = str(r["Title"])
            meta.authors = _clean(r.get("Authors"))
            year = _clean(r.get("Year"))
            meta.year = int(year) if year is not None else None
            meta.venue = _clean(r.get("Venue"))
            meta.doi = _clean(r.get("DOI"))
            meta.matched = True
            meta.ambiguous = len(rows) > 1
        papers.append(meta)
    return papers
