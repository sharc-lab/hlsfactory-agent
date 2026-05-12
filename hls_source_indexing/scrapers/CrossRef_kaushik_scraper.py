import requests
import os
import pandas as pd
import re

BASE_URL = "https://api.crossref.org/works"
HEADERS = {"User-Agent": "test_script (kaushik.chandana@gmail.com)"}

keywords = ["HLS", "high level synthesis", "high-level-synthesis", "high-level synthesis"]
rows = []
seen_dois = set()

validation_patterns = [
    r'\bHLS\b',  # HLS as whole word
    r'\bhigh[\s-]level[\s-]synthesis\b',  # high level/high-level synthesis
]

venues = {
    "ISFPGA": ["ISFPGA", "FPGA", "Field-Programmable Gate Arrays", "Field Programmable Gate Arrays"],
    "FCCM": ["FCCM", "Field-Programmable Custom Computing", "Custom Computing Machines"],
    "FPL": ["FPL", "Field Programmable Logic", "Field-Programmable Logic"],
    "FPT": ["FPT", "Field-Programmable Technology", "Field Programmable Technology"],
    "HEART": ["HEART", "Highly Efficient Accelerators", "Reconfigurable Technologies"],
    "TRETS": ["TRETS", "Reconfigurable Technology", "Transactions on Reconfigurable"],
    "ICCAD": ["ICCAD", "Computer-Aided Design", "Computer Aided Design", "international conference on computer aided design"],
    "DAC": ["DAC", "Design Automation Conference", "Design Automation"],
    "ASP-DAC": ["ASP-DAC", "Asia and South Pacific", "Design Automation Conference"],
    "DATE": ["DATE", "Design, Automation and Test", "Design Automation Test Europe"],
    "MLCAD": ["MLCAD", "Machine Learning for CAD", "ML for CAD"],
    "ICLAD": ["ICLAD"],
    "GLVLSI": ["GLVLSI", "Great Lakes", "Great Lakes Symposium", "VLSI"],
    "HOST": ["HOST", "Hardware Oriented Security", "Security and Trust"],
    "TCAD": ["TCAD", "Computer-Aided Design", "Transactions on Computer-Aided Design"],
    "ISCA": ["ISCA", "Computer Architecture", "International Symposium on Computer Architecture"],
    "MICRO": ["MICRO", "Microarchitecture", "International Symposium on Microarchitecture"],
    "HPCA": ["HPCA", "High Performance Computer Architecture", "High-Performance Computer"],
    "ESWEEK": ["ESWEEK", "Embedded Systems Week", "CASES", "CODES", "EMSOFT"],
    "MLSYS": ["MLSYS", "Machine Learning and Systems", "ML and Systems"],
    "ASPLOS": ["ASPLOS", "Architectural Support", "Programming Languages and Operating Systems"]
}

def contains_hls_term(title, abstract=""):
    """Check if title or abstract contains exact HLS-related terms"""
    search_text = f"{title} {abstract}"
    for pattern in validation_patterns:
        if re.search(pattern, search_text, re.IGNORECASE):
            return True
    return False

for keyword in keywords:
    offset = 0
    while True:
        params = {
            "query" : keyword,
            "filter": "prefix:10.1145",
            "rows" : 200,
            "offset" : offset
        }

        response = requests.get(BASE_URL, params=params, headers=HEADERS)
        print(f"{response.status_code} Offset: {offset}")
        response.raise_for_status()
        
        data = response.json()
        items = data.get("message", {}).get("items", [])

        if not items:
            break

        for item in items:
            if item.get("type") != "proceedings-article":
                continue

            container_titles = item.get("container-title", [])

            if container_titles:
                container_title = container_titles[0]
            else:
                container_title = ""

            matched_venue = None
            for venue_abbrev, venue_keywords in venues.items():
                for word in venue_keywords:
                    if word.upper() in container_title.upper():
                        matched_venue = venue_abbrev
                        break
                
            if not matched_venue:
                continue

            title = item.get("title", [""])[0]
            abstract = item.get("abstract", "")
            doi = item.get("DOI", "")
            year = item.get("")
            
            # CRITICAL: Only include if title or abstract contains HLS terms
            if not contains_hls_term(title, abstract):
                continue
            
            # Skip duplicates
            if doi in seen_dois:
                continue
            seen_dois.add(doi)

            url = item.get("URL", "")

            authors_list = item.get("author", [])
            authors = []

            for author in authors_list:
                firstName = author.get("given", "")
                lastName = author.get("family", "")
                fullName = f"{firstName} {lastName}".strip()
                if fullName:
                    authors.append(fullName)
            authors_str = ", ".join(authors)

            rows.append({
                "Title" : title,
                "Authors" : authors_str,
                "DOI" : doi,
                "URL" : url,
                "Venue" : matched_venue,
                "Full_Venue" : container_title
            })
        offset += 200
        if offset > 1200:
            break

df = pd.DataFrame(rows)
df.to_excel("Crossref_ACM_kaushik_Results.xlsx", index = False)
print(f"Wrote {len(df)} rows to Crossref_ACM_Results.xlsx")
