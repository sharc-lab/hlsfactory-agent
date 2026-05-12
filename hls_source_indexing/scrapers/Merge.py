import re
import pandas as pd
from pathlib import Path

# Input files produced by your scrapers
IEEE_FILE   = "HLS_Papers_IEEE.xlsx"
DBLP_FILE   = "HLS_Papers_DBLP.xlsx"
XREF_FILE   = "Crossref_ACM_Justin.xlsx"  # ACM-only Crossref

OUT_FILE    = "HLS_Papers_Merged.xlsx"
STATS_FILE  = "dedupe_stats.txt"

def extract_doi(s: str) -> str:
    """
    Extract a DOI from a string/URL if present.
    Handles 'https://doi.org/<doi>' or bare DOI-like strings.
    """
    if not isinstance(s, str):
        return ""
    s = s.strip()
    # From DOI URL
    m = re.search(r"(10\.\d{4,9}/\S+)", s, re.IGNORECASE)
    if m:
        # strip trailing punctuation or HTML noise
        doi = m.group(1).rstrip(" .),;]")
        return doi
    return ""

def normalize_title(t: str) -> str:
    """
    Normalize title for fuzzy-ish matching:
    - lowercase, remove punctuation (keep alnum + spaces)
    - collapse whitespace
    """
    if not isinstance(t, str):
        return ""
    t = t.lower()
    t = re.sub(r"[^a-z0-9\s]", " ", t)
    t = re.sub(r"\s+", " ", t).strip()
    return t

def prefer_link(row_candidates):
    """
    Given multiple rows that represent the same paper, pick a 'best' link:
    1) DOI link if any
    2) IEEE document link if any
    3) Anything else non-empty
    """
    # try DOI link
    for r in row_candidates:
        if isinstance(r.get("Link"), str) and r["Link"].startswith("https://doi.org/"):
            return r["Link"]
    # try IEEE
    for r in row_candidates:
        if isinstance(r.get("Link"), str) and "ieeexplore.ieee.org" in r["Link"]:
            return r["Link"]
    # fallback: any non-empty
    for r in row_candidates:
        if isinstance(r.get("Link"), str) and r["Link"].strip():
            return r["Link"]
    return ""

def coalesce(values):
    """Return the first non-empty string."""
    for v in values:
        if isinstance(v, str) and v.strip():
            return v
    return ""

def main():
    stats = []

    # --- Load IEEE
    df_ieee = pd.read_excel(IEEE_FILE)
    # Ensure expected columns
    for col in ["Title", "Authors", "Year", "Link", "Venue"]:
        if col not in df_ieee.columns:
            df_ieee[col] = ""
    df_ieee["Source"] = "IEEE"
    df_ieee["DOI"] = df_ieee["Link"].apply(extract_doi)
    df_ieee["norm_title"] = df_ieee["Title"].apply(normalize_title)

    # --- Load DBLP
    df_dblp = pd.read_excel(DBLP_FILE)
    for col in ["Title", "Authors", "Year", "Link", "Venue"]:
        if col not in df_dblp.columns:
            df_dblp[col] = ""
    df_dblp["Source"] = "DBLP"
    df_dblp["DOI"] = df_dblp["Link"].apply(extract_doi)
    df_dblp["norm_title"] = df_dblp["Title"].apply(normalize_title)

    # --- Load Crossref (ACM)
    df_xref = pd.read_excel(XREF_FILE)
    # Crossref columns differ slightly; align to common schema
    # Expected Crossref cols: Title, Issued, URL, Conference
    if "Title" not in df_xref.columns:
        df_xref["Title"] = ""
    df_xref = df_xref.rename(columns={
        "Issued": "Year",
        "URL": "Link",
        "Conference": "Venue"
    })
    if "Authors" not in df_xref.columns:
        df_xref["Authors"] = ""  # Crossref response above didn't include authors
    df_xref["Source"] = "Crossref-ACM"
    df_xref["DOI"] = df_xref["Link"].apply(extract_doi)
    df_xref["norm_title"] = df_xref["Title"].apply(normalize_title)

    # --- Concatenate
    cols = ["Title", "Authors", "Year", "Link", "Venue", "Source", "DOI", "norm_title"]
    df_all = pd.concat([df_ieee[cols], df_dblp[cols], df_xref[cols]], ignore_index=True)

    # Coerce Year to string for grouping; keep a numeric copy for reporting
    df_all["Year"] = df_all["Year"].astype(str).str.extract(r"(\d{4})", expand=False).fillna("")
    before = len(df_all)

    # --- Dedupe pass 1: by DOI (non-empty)
    with_doi = df_all[df_all["DOI"].str.len() > 0].copy()
    no_doi   = df_all[df_all["DOI"].str.len() == 0].copy()

    # Group by DOI and merge rows
    merged_rows = []
    for doi, grp in with_doi.groupby("DOI", dropna=False):
        # Choose best link, and coalesce other fields
        link = prefer_link(grp.to_dict("records"))
        title = coalesce(grp["Title"])
        authors = coalesce(grp["Authors"])
        year = coalesce(grp["Year"])
        venue = coalesce(grp["Venue"])
        # Keep sources to know provenance (optional)
        source = ";".join(sorted(set(grp["Source"])))
        merged_rows.append({
            "Title": title,
            "Authors": authors,
            "Year": year,
            "Link": link if link else f"https://doi.org/{doi}",
            "Venue": venue,
            "Source": source,
            "DOI": doi,
            "norm_title": normalize_title(title),
        })

    df_merged_doi = pd.DataFrame(merged_rows, columns=cols)

    # --- Dedupe pass 2: remaining rows without DOI -> by (norm_title, Year)
    if not no_doi.empty:
        no_doi["key2"] = list(zip(no_doi["norm_title"], no_doi["Year"]))
        merged_rows2 = []
        for key2, grp in no_doi.groupby("key2", dropna=False):
            link = prefer_link(grp.to_dict("records"))
            title = coalesce(grp["Title"])
            authors = coalesce(grp["Authors"])
            year = coalesce(grp["Year"])
            venue = coalesce(grp["Venue"])
            source = ";".join(sorted(set(grp["Source"])))
            doi = coalesce(grp["DOI"])  # empty in this branch, but keep structure
            merged_rows2.append({
                "Title": title,
                "Authors": authors,
                "Year": year,
                "Link": link,
                "Venue": venue,
                "Source": source,
                "DOI": doi,
                "norm_title": normalize_title(title),
            })
        df_merged_key2 = pd.DataFrame(merged_rows2, columns=cols)
    else:
        df_merged_key2 = pd.DataFrame(columns=cols)

    # --- Final combine
    df_final = pd.concat([df_merged_doi, df_merged_key2], ignore_index=True)

    # Sort for readability
    df_final["sort_year"] = pd.to_numeric(df_final["Year"], errors="coerce")
    df_final = df_final.sort_values(["sort_year", "norm_title"], ascending=[False, True]).drop(columns=["sort_year"])

    after = len(df_final)

    # --- Write outputs
    df_final.drop(columns=["norm_title"]).to_excel(OUT_FILE, index=False)

    deduped = before - after
    stats.append(f"Input rows (all sources concatenated): {before}")
    stats.append(f"Output rows (after dedupe): {after}")
    stats.append(f"Total removed as duplicates: {deduped}")

    # Source breakdown (optional)
    src_counts = df_all["Source"].value_counts().to_dict()
    stats.append("Input source counts: " + ", ".join([f"{k}={v}" for k, v in src_counts.items()]))

    Path(STATS_FILE).write_text("\n".join(stats), encoding="utf-8")
    print("\n".join(stats))
    print(f"Saved merged file -> {OUT_FILE}")
    print(f"Wrote stats -> {STATS_FILE}")

if __name__ == "__main__":
    main()
