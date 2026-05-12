import time
import requests
import pandas as pd
import re

BASE_URL = "https://api.crossref.org/works"
HEADERS = {"User-Agent": "test_script (justzh2016@gmail.com)"}
keyword = ["high level synthesis", "high-level synthesis", "hls"]

conf = [
    "ISFPGA", "FCCM", "FPL", "FPT", "HEART",
    "ICCAD", "DAC", "ASP-DAC", "DATE", "MLCAD", "ICLAD", "GLVLSI", "HOST",
    "ISCA", "MICRO", "HPCA", "ESWEEK", "MLSYS", "ASPLOS",
    "TRETS", "TCAD",
]

# Common aliases to recognize the venue in publicationTitle (case-insensitive substring)
conf_aliases: dict[str, list[str]] = {
    "ISFPGA": ["fpga", "field-programmable gate arrays"],
    "FCCM": ["fccm", "field-programmable custom computing machines"],
    "FPL": ["fpl", "field-programmable logic and applications"],
    "FPT": ["fpt", "field-programmable technology"],
    "HEART": ["heart", "high-performance embedded architectures and compilers"],  # HEART naming varies
    "ICCAD": ["iccad", "international conference on computer-aided design"],
    "DAC": ["dac", "design automation conference"],
    "ASP-DAC": ["asp-dac", "asia and south pacific design automation"],
    "DATE": ["date", "design, automation & test in europe"],
    "MLCAD": ["mlcad", "machine learning for cad"],
    "ICLAD": ["iclad"],  # may be rare; keep acronym
    "GLVLSI": ["glvlsi", "great lakes symposium on vlsi"],
    "HOST": ["host", "hardware-oriented security and trust"],
    "ISCA": ["isca", "international symposium on computer architecture"],
    "MICRO": ["micro", "international symposium on microarchitecture"],
    "HPCA": ["hpca", "high performance computer architecture"],
    "ESWEEK": ["embedded systems week", "cases", "emsoft", "codes+isss", "esweek"],
    "MLSYS": ["mlsys", "machine learning and systems"],
    "ASPLOS": ["asplos", "architectural support for programming languages and operating systems"],
    "TRETS": ["trets", "transactions on reconfigurable technology and systems"],  # ACM (likely 0 on IEEE)
    "TCAD": ["tcad", "ieee transactions on computer-aided design of integrated circuits and systems"],
}

def get_conf_name(work : dict) -> str | None:

    #method 1: look at event.name in json file
    event = work.get("event") or {}
    if event and "name" in event:
        return event["name"]
    
    #method 2: look for container-title in json
    conf_title = work.get("container-title") or []
    if conf_title:
        return conf_title[0]
    
    return None
    
def normalize_string(s : str) -> str:
    s = s.lower()
    s = s.replace("-", " ")
    s = re.sub(r"\s+", " ", s)  # collapse multiple spaces
    return s.strip()

def check_for_match(title : str, conf_aliases: dict[str, list[str]]) -> str:
    title = normalize_string(title)
    #loop through each conference, see if alias is present in the paper's conference name
    for conf, aliases in conf_aliases.items():
        for alias in aliases:
            alias_normalized = normalize_string(alias)
            if alias_normalized in title:
                return conf
    return ""

rows = []

#loop until no more items fitting parameters or no more new cursor
count = 0
for kw in keyword:
    params = {
        "query" : kw,  #only using keyword "high level synthesis"
        "filter" : "prefix:10.1145,type:proceedings-article",
        "rows" : 1000,
        "cursor" : "*"
    }
    while True:
        response = requests.get(url=BASE_URL, params=params, headers=HEADERS)
        response.raise_for_status()

        data = response.json()

        #json structure: message dict -> items dict -> paper info
        message = data.get("message", {})
        items = message.get("items", [])

        #stop if no more items left
        if not items:
            break

        for x in items:
            titles = x.get("title", []) or []
            title = titles[0] if titles else ""
            subtitle = x.get("subtitle", []) or []
            shorttitle = x.get("short-title", []) or []
            abstract = x.get("abstract", "")

            #using re instead of manual checks
            hls_pattern = re.compile(r"(?:\bHLS\b|\bhigh[\s-]level[\s-]synthesis\b)", re.IGNORECASE)

            # check title, subtitle, shorttitle, and abstract
            if not(
                hls_pattern.search(title)
                or hls_pattern.search(" ".join(subtitle))
                or hls_pattern.search(" ".join(shorttitle))
                or hls_pattern.search(abstract)
            ):
                continue

            # if not (
            #     any(kw in title.lower() for kw in keyword)
            #     or any(any(kw in s.lower() for kw in keyword) for s in subtitle)
            #     or any(any(kw in s.lower() for kw in keyword) for s in shorttitle)
            # ):
            #     continue

            #conference of the paper returned from json
            listedconf = get_conf_name(x)
            #matching conference in our list
            conf = check_for_match(listedconf, conf_aliases)
            if not conf:
                continue
            
            issued = x.get("issued", {}).get("date-parts")[0][0]

            rows.append({
                "Title" : title,
                "Issued" : issued,
                "URL" : x.get("URL"),
                "Conference" : conf
            })
        
        #stop if there are no more pages
        next_cursor = message.get("next-cursor")
        if not next_cursor:
            break
        params["cursor"] = next_cursor

        time.sleep(0.1)
        print(f"scanned cursor {count}: {params["cursor"]}")
        count = count + 1

df = pd.DataFrame(rows)
df.to_excel("Crossref_ACM_Justin.xlsx", index=False)
print(f"Wrote {len(df)} rows to crossref_simple.xlsx")