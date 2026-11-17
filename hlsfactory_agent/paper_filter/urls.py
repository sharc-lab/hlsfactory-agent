"""Find GitHub repo references in extracted paper text. Pure functions, no network."""

import re
from dataclasses import dataclass, field

# Path segments after github.com/ that are site pages, not repo owners.
RESERVED_OWNERS = {
    "features", "sponsors", "topics", "orgs", "settings", "marketplace",
    "about", "pricing", "login", "join", "explore", "collections", "events",
    "apps", "enterprise", "security", "notifications", "pulls", "issues",
}  # fmt: skip

GITHUB_RE = re.compile(
    r"github\.com/(?P<owner>[A-Za-z0-9][A-Za-z0-9-]*)"
    r"(?:/(?P<repo>[A-Za-z0-9_.-]*))?"
)
TRAILING_PUNCT = ".,;:)]}>\"'"
REFERENCES_HEADING_RE = re.compile(r"^\s*(?:[IVX]+\.\s*)?references?\s*$", re.I | re.M)
CONTEXT_CHARS = 300


@dataclass
class Hit:
    repo_id: str  # "owner/name"
    raw: str
    page: int
    found_via: str  # "text" | "annotation"
    context: str = ""
    in_references: bool | None = None  # None: unknown (annotations)
    repaired: bool = False  # URL was rejoined across a line break


@dataclass
class UrlScan:
    hits: list[Hit] = field(default_factory=list)
    profiles: list[str] = field(default_factory=list)  # github.com/<owner> only


def clean_repo_name(name: str) -> str:
    name = name.rstrip(TRAILING_PUNCT)
    if name.endswith(".git"):
        name = name[:-4]
    return name


def _repair_line_break(text: str, end: int) -> tuple[str, bool]:
    """If a URL ends the line on '/', '-' or '_', glue on the next line's first token."""
    if end >= len(text) or text[end] != "\n" or text[end - 1] not in "/-_":
        return "", False
    nxt = text[end + 1 :].split(None, 1)
    return (nxt[0], True) if nxt else ("", False)


def references_start(pages: list[str]) -> tuple[int, int] | None:
    """(page index, char offset) of the last 'References' heading, if any."""
    found = None
    for i, t in enumerate(pages):
        for m in REFERENCES_HEADING_RE.finditer(t):
            found = (i, m.start())
    return found


def _context(text: str, start: int, end: int) -> str:
    lo = max(0, start - CONTEXT_CHARS // 2)
    hi = min(len(text), end + CONTEXT_CHARS // 2)
    return " ".join(text[lo:hi].split())


def parse_github_ref(s: str) -> tuple[str, str | None] | None:
    m = GITHUB_RE.search(s)
    if not m or m["owner"].lower() in RESERVED_OWNERS:
        return None
    repo = clean_repo_name(m["repo"]) if m["repo"] else None
    return m["owner"], repo or None


def scan_text(pages: list[str]) -> UrlScan:
    scan = UrlScan()
    ref_start = references_start(pages)
    for pi, text in enumerate(pages):
        for m in GITHUB_RE.finditer(text):
            end = m.end()
            raw = m.group(0)
            extra, repaired = _repair_line_break(text, end)
            if repaired:
                raw += extra
                end += 1 + len(extra)
            parsed = parse_github_ref(raw)
            if parsed is None:
                continue
            owner, repo = parsed
            if repo is None:
                if owner not in scan.profiles:
                    scan.profiles.append(owner)
                continue
            in_refs = ref_start is not None and (pi, m.start()) > ref_start
            scan.hits.append(
                Hit(
                    repo_id=f"{owner}/{repo}",
                    raw=raw.rstrip(TRAILING_PUNCT),
                    page=pi + 1,
                    found_via="text",
                    context=_context(text, m.start(), end),
                    in_references=in_refs,
                    repaired=repaired,
                )
            )
    return scan


def scan_annotations(links: list[tuple[int, str]]) -> list[Hit]:
    hits = []
    for page, uri in links:
        parsed = parse_github_ref(uri)
        if parsed is None or parsed[1] is None:
            continue
        hits.append(
            Hit(
                repo_id=f"{parsed[0]}/{parsed[1]}",
                raw=uri,
                page=page,
                found_via="annotation",
            )
        )
    return hits


def merge_hits(hits: list[Hit]) -> list[dict]:
    """Dedupe by case-insensitive repo_id; keep every page and source."""
    merged: dict[str, dict] = {}
    for h in hits:
        key = h.repo_id.lower()
        if key not in merged:
            merged[key] = {
                "repo_id": h.repo_id,
                "raws": [],
                "found_via": [],
                "pages": [],
                "context": "",
                "_refs": [],
                "repaired": False,
            }
        e = merged[key]
        if h.raw not in e["raws"]:
            e["raws"].append(h.raw)
        if h.found_via not in e["found_via"]:
            e["found_via"].append(h.found_via)
        if h.page not in e["pages"]:
            e["pages"].append(h.page)
        if h.context and not e["context"]:
            e["context"] = h.context
        if h.in_references is not None:
            e["_refs"].append(h.in_references)
        e["repaired"] = e["repaired"] or h.repaired
    for e in merged.values():
        refs = e.pop("_refs")
        e["in_references"] = all(refs) if refs else None
    return list(merged.values())
