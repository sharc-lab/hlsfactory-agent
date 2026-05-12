import requests
import pandas as pd
import re

BASE_URL = "https://api.crossref.org/works"
headers = {"User-Agent": "basic-scraper (mailto:jayimperatori12@gmail.com)"}

# Use just one broad keyword to avoid duplicates
keywords = [
    "HLS OR \"High Level Synthesis\" OR \"High-level synthesis\""
]

# conferences (all will later sort to only ACM using the doi)
conference_filters = {
    "ISFPGA": ["ISFPGA", "ACM/SIGDA International Symposium on Field Programmable Gate Arrays"],
    "FCCM": ["FCCM", "IEEE Symposium on Field-Programmable Custom Computing Machines"],
    "FPL": ["FPL", "Field Programmable Logic and Applications"],
    "FPT": ["FPT", "International Conference on Field-Programmable Technology"],
    "HEART": ["HEART", "International Symposium on Highly Efficient Accelerators and Reconfigurable Technologies"],
    "TRETS": ["TRETS", "ACM Transactions on Reconfigurable Technology and Systems"],
    "ICCAD": ["ICCAD", "IEEE/ACM International Conference on Computer-Aided Design"],
    "DAC": ["DAC", "Design Automation Conference"],
    "ASP-DAC": ["ASP-DAC", "Asia and South Pacific Design Automation Conference"],
    "DATE": ["DATE", "Design, Automation & Test in Europe Conference"],
    "MLCAD": ["MLCAD", "Machine Learning for CAD"],
    "ICLAD": ["ICLAD", "International Conference on Learning for Advanced Design"],
    "GLVLSI": ["GLVLSI", "Great Lakes Symposium on VLSI"],
    "HOST": ["HOST", "IEEE International Symposium on Hardware Oriented Security and Trust"],
    "TCAD": ["TCAD", "IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems"],
    "ISCA": ["ISCA", "International Symposium on Computer Architecture"],
    "MICRO": ["MICRO", "IEEE/ACM International Symposium on Microarchitecture"],
    "HPCA": ["HPCA", "IEEE International Symposium on High-Performance Computer Architecture"],
    "ESWEEK": ["ESWEEK", "Embedded Systems Week"],
    "MLSYS": ["MLSYS", "Conference on Machine Learning and Systems"],
    "ASPLOS": ["ASPLOS", "International Conference on Architectural Support for Programming Languages and Operating Systems"]
}

exact_phrases = ["hls", "high level synthesis", "high-level synthesis"]

# store all results here
all_records = []

for keyword in keywords:
    offset = 0
    rows = 100

    while True:
        params = {
            "query": keyword,              # search keyword anywhere in metadata
            "filter": "prefix:10.1145",    # ACM-only filter
            "rows": rows,
            "offset": offset
        }

        response = requests.get(BASE_URL, params=params, headers=headers, timeout=30)

        if response.status_code != 200:
            print(f"[WARN] keyword=({keyword}) offset={offset} -> HTTP {response.status_code}")
            break

        data = response.json()
        items = data.get("message", {}).get("items", [])
        total = data.get("message", {}).get("total-results", 0)

        if offset == 0:
            print(f"Keyword=({keyword}) : {total} ACM results")

        if not items:
            break

        for item in items:
            if item.get("type") not in ["proceedings-article", "journal-article"]:
                continue

            title = item.get("title", [""])[0]
            doi = item.get("DOI", "")
            url = item.get("URL", "")
            year = item.get("issued", {}).get("date-parts", [[None]])[0][0]
            container = item.get("container-title", [""])[0].lower()

            # Get abstract
            abstract = item.get("abstract", "")

            # Get subject/keywords if available
            subjects = " ".join(item.get("subject", [])).lower()

            # Combine title, abstract, and subjects for searching
            combined_text = (title + " " + abstract + " " + subjects).lower()

            # Check for exact phrases in combined metadata
            if not any(re.search(rf"\b{re.escape(p)}\b", combined_text) for p in exact_phrases):
                continue

            # Match conference
            matched_conf = None
            for conf, aliases in conference_filters.items():
                if any(alias.lower() in container for alias in aliases):
                    matched_conf = conf
                    break

            # Only save if matched to one of our conferences
            if matched_conf:
                all_records.append({
                    "Keyword": keyword,
                    "Conference": matched_conf,
                    "Year": year,
                    "Title": title,
                    "DOI": doi,
                    "URL": url,
                    "Container Title": container
                })

        offset += rows
        if offset >= total:   # stop if we’ve collected all
            break

# Save results to Excel
df = pd.DataFrame(all_records)
print(f"✅ Done. Saved {len(all_records)} ACM-only results to crossref_results.xlsx")

print(f"\nDone. Collected {len(all_records)} ACM records.")

# save to Excel
df.to_excel("crossref_results.xlsx", index=False)

print("Results saved to crossref_results.xlsx")
