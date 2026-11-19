from dataclasses import dataclass, field
from pathlib import Path

import pymupdf

MIN_CHARS_PER_PAGE = 100


@dataclass
class PdfDoc:
    pages: list[str]
    links: list[tuple[int, str]] = field(default_factory=list)  # (1-indexed page, uri)

    @property
    def n_chars(self) -> int:
        return sum(len(p) for p in self.pages)

    @property
    def scanned(self) -> bool:
        """No usable text layer; needs OCR, which we skip for now."""
        return not self.pages or self.n_chars / len(self.pages) < MIN_CHARS_PER_PAGE


def read_pdf(path: Path) -> PdfDoc:
    pages: list[str] = []
    links: list[tuple[int, str]] = []
    with pymupdf.open(path) as doc:
        for i, page in enumerate(doc, start=1):
            pages.append(page.get_text())
            links.extend(
                (i, link["uri"]) for link in page.get_links() if link.get("uri")
            )
    return PdfDoc(pages=pages, links=links)
