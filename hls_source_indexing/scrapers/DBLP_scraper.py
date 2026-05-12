# dblp_hls_scraper.py
import time
import re
import html
from typing import Dict, List, Tuple, Any

import requests
import pandas as pd
from requests.adapters import HTTPAdapter
from urllib3.util.retry import Retry

# ---------- Config ----------
QUERY_VARIANTS = [
    'high-level synthesis',
    '"high-level synthesis"',
    'high level synthesis',
    '"high level synthesis"',
    'HLS',
]

HITS_PER_PAGE = 200
REQUEST_DELAY = 0.7

UA = ("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
      "(KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36")

DBLP_BASES = [
    "https://dblp.org",
    "https://dblp.uni-trier.de",
    "https://dblp2.uni-trier.de",
]

VENUES = [
    "ISFPGA", "FCCM", "FPL", "FPT", "HEART",
    "ICCAD", "DAC", "ASP-DAC", "DATE", "MLCAD", "ICLAD", "GLVLSI", "HOST",
    "ISCA", "MICRO", "HPCA", "ESWEEK", "MLSYS", "ASPLOS",
    "TRETS", "TCAD",
]

VENUE_ALIASES: Dict[str, List[str]] = {
    "ISFPGA": ["fpga", "field-programmable gate arrays"],
    "FCCM": ["fccm", "field-programmable custom computing machines"],
    "FPL": ["fpl", "field-programmable logic and applications"],
    "FPT": ["fpt", "field-programmable technology"],
    "HEART": ["heart", "highly efficient accelerators and reconfigurable technologies",
              "high-performance embedded architectures and compilers"],
    "ICCAD": ["iccad", "international conference on computer-aided design"],
    "DAC": ["dac", "design automation conference"],
    "ASP-DAC": ["asp-dac", "asia and south pacific design automation conference"],
    "DATE": ["date", "design, automation & test in europe", "design, automation, and test in europe"],
    "MLCAD": ["mlcad", "machine learning for cad"],
    "ICLAD": ["iclad"],
    "GLVLSI": ["glvlsi", "great lakes symposium on vlsi"],
    "HOST": ["host", "hardware-oriented security and trust"],
    "TCAD": [
        "ieee transactions on computer-aided design of integrated circuits and systems",
        "ieee trans. comput. aided des. integr. circuits syst.",
        "tcad"
    ],
    "ISCA": ["isca", "international symposium on computer architecture"],
    "MICRO": ["micro", "international symposium on microarchitecture"],
    "HPCA": ["hpca", "high performance computer architecture"],
    "ESWEEK": ["embedded systems week", "cases", "emsoft", "codes+isss", "codes/isss", "esweek"],
    "MLSYS": ["mlsys", "machine learning and systems"],
    "ASPLOS": ["asplos", "architectural support for programming languages and operating systems"],
    "TRETS": ["acm transactions on reconfigurable technology and systems",
              "acm trans. reconfigurable technol. syst.", "trets"],
}

# ---------- Helpers ----------
def coerce_text(x: Any) -> str:
    """Turn DBLP field (str/list/dict/None) into a flat string."""
    if isinstance(x, str):
        return x
    if isinstance(x, list):
        parts: List[str] = []
        for el in x:
            if isinstance(el, str):
                parts.append(el)
            elif isinstance(el, dict):
                parts.append(el.get("text") or el.get("name") or "")
        return " ; ".join(p for p in parts if p)
    if isinstance(x, dict):
        return x.get("text") or x.get("name") or ""
    return "" if x is None else str(x)

def _norm(x: Any) -> str:
    return coerce_text(x).strip().lower()

def venue_matches(venue_key: str, venue_text: Any) -> bool:
    """Return True if venue_text contains an alias for venue_key (robust to lists/dicts)."""
    pt = _norm(venue_text)
    if not pt:
        return False
    aliases = VENUE_ALIASES.get(venue_key, [venue_key.lower()])
    for alias in aliases:
        a = (alias or "").strip().lower()
        if not a:
            continue
        # short alpha aliases (<=4 chars) use word-boundary regex to avoid partials (e.g., micro vs microscopy)
        if len(a) <= 4 and a.isalpha():
            if re.search(rf"\b{re.escape(a)}\b", pt):
                return True
        else:
            if a in pt:
                return True
    return False

def make_session() -> requests.Session:
    s = requests.Session()
    s.headers.update({"User-Agent": UA})
    retry = Retry(
        total=5, connect=5, read=5,
        backoff_factor=1.0,
        status_forcelist=[429, 500, 502, 503, 504],
        allowed_methods={"GET"},
        raise_on_status=False,
    )
    s.mount("https://", HTTPAdapter(max_retries=retry))
    return s

SESSION = make_session()

def fetch_dblp_page(base: str, query: str, offset: int, h: int = HITS_PER_PAGE) -> Tuple[int, List[Dict[str, Any]]]:
    params = {"q": query, "h": h, "f": offset, "format": "json"}
    r = SESSION.get(f"{base}/search/publ/api", params=params, timeout=(8, 30))
    r.raise_for_status()
    j = r.json()
    result = j.get("result", {})
    hits = result.get("hits", {})
    total_str = hits.get("@total", "0")
    try:
        total = int(total_str)
    except Exception:
        total = 0
    hit_list = hits.get("hit", [])
    if isinstance(hit_list, dict):
        hit_list = [hit_list]
    return total, hit_list

def fetch_all_pages_for_query(query: str) -> List[Dict[str, Any]]:
    last_err = None
    for base in DBLP_BASES:
        try:
            offset = 0
            total, hits = fetch_dblp_page(base, query, offset, HITS_PER_PAGE)
            print(f"[{base}] total for {query!r}: {total}")
            rows: List[Dict[str, Any]] = []
            while True:
                rows.extend(hits)
                if len(hits) < HITS_PER_PAGE or len(rows) >= total:
                    break
                offset += HITS_PER_PAGE
                time.sleep(REQUEST_DELAY)
                _, hits = fetch_dblp_page(base, query, offset, HITS_PER_PAGE)
            return rows
        except requests.RequestException as e:
            last_err = e
            print(f"[WARN] {base} failed for {query!r}: {e}")
            time.sleep(1.0)
            continue
    if last_err:
        raise last_err
    return []

def extract_info_from_hit(hit: Dict[str, Any]) -> Dict[str, str]:
    info = hit.get("info", {}) if isinstance(hit.get("info"), dict) else {}

    # Title (strip HTML tags/entities)
    raw_title = coerce_text(info.get("title", ""))
    title = html.unescape(re.sub(r"<[^>]+>", "", raw_title)).strip()

    # Authors (DBLP "authors" can be dict/list/str)
    names: List[str] = []
    authors_obj = info.get("authors", {})
    if isinstance(authors_obj, dict):
        author_field = authors_obj.get("author")
        if isinstance(author_field, list):
            for a in author_field:
                if isinstance(a, dict):
                    nm = a.get("text") or a.get("name")
                    if nm:
                        names.append(nm)
                elif isinstance(a, str):
                    names.append(a)
        elif isinstance(author_field, dict):
            nm = author_field.get("text") or author_field.get("name")
            if nm:
                names.append(nm)
        elif isinstance(author_field, str):
            names.append(author_field)
    authors = ", ".join(names)

    # Year
    year = coerce_text(info.get("year", "")).strip()

    # Venue: normalize across 'venue' / 'booktitle' / 'journal' (can be list/dict)
    raw_venue = info.get("venue")
    if not raw_venue:
        raw_venue = info.get("booktitle")
    if not raw_venue:
        raw_venue = info.get("journal")
    venue = coerce_text(raw_venue).strip()

    # Link: prefer 'ee' (electronic edition), otherwise 'url' (DBLP page)
    link = ""
    ee = info.get("ee")
    if isinstance(ee, list) and ee:
        link = coerce_text(ee[0])
    elif isinstance(ee, str):
        link = ee
    if not link:
        link = coerce_text(info.get("url", ""))

    return {"Title": title, "Authors": authors, "Year": year, "Link": link, "Venue": venue}

# ---------- Main ----------
def main():
    all_rows: List[Dict[str, str]] = []

    for q in QUERY_VARIANTS:
        print(f"\n=== Query: {q!r} ===")
        hits = fetch_all_pages_for_query(q)
        print(f"Fetched {len(hits)} raw hits for {q!r}")
        for hit in hits:
            row = extract_info_from_hit(hit)
            if any(venue_matches(vk, row["Venue"]) for vk in VENUES):
                all_rows.append(row)
                # Show a couple samples early on
                if len(all_rows) <= 3:
                    print("  sample:", row["Year"], "-", row["Venue"], ":", row["Title"][:80])

    # De-dup by (Title, Venue, Year, Link)
    seen = set()
    deduped: List[Dict[str, str]] = []
    for r in all_rows:
        key = (
            r["Title"].strip().lower(),
            re.sub(r"\s+", " ", r["Venue"].strip().lower()),
            r["Year"].strip(),
            r["Link"].strip().lower(),
        )
        if key not in seen:
            seen.add(key)
            deduped.append(r)

    df = pd.DataFrame(deduped, columns=["Title", "Authors", "Year", "Link", "Venue"])
    df.to_excel("hls_papers_dblp.xlsx", index=False)
    print(f"\nDone. Saved {len(deduped)} rows to hls_papers_dblp.xlsx")

if __name__ == "__main__":
    main()
