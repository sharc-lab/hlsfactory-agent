import json

import requests

OPENROUTER_URL = "https://openrouter.ai/api/v1/chat/completions"
DEFAULT_MODEL = "deepseek/deepseek-v4-flash"
FIRST_PAGE_CHARS = 6000

NOVELTY_LABELS = ("new_design", "tool_or_method", "survey", "unsure")
ROLES = ("own", "cited", "unsure")

SYSTEM = """You help build a dataset of open-source HLS (high-level synthesis) hardware designs.
Given an academic paper's first page and the GitHub links found in it, answer two questions.

1. novelty: does the paper present a NEW hardware design/accelerator whose source code could exist
   (new_design), or is it mainly a tool/method/framework evaluated on existing benchmarks
   (tool_or_method), or a survey/position paper (survey)? Answer "unsure" whenever the first page
   does not clearly settle it. Do not guess.
2. For each candidate link: is it the authors' OWN code for this paper (own), a repo of some other
   work the paper merely cites/uses/compares against (cited), or you cannot tell (unsure).
   A link in the references section is usually cited. A phrase like "our code is available at" or
   "will be released at" means own, even if not yet released.

Reply with JSON only:
{"novelty": {"label": "...", "reason": "..."},
 "candidates": [{"repo_id": "owner/name", "role": "own|cited|unsure", "reason": "..."}]}"""


def build_user_message(title: str, first_page: str, candidates: list[dict]) -> str:
    lines = [f"TITLE: {title}", "", "FIRST PAGE:", first_page[:FIRST_PAGE_CHARS], "", "CANDIDATE LINKS:"]
    if not candidates:
        lines.append("(none found)")
    for c in candidates:
        gh = c.get("gh") or {}
        lines.append(
            f"- {c['repo_id']} | pages {c['pages']} | in_references={c['in_references']} "
            f"| exists={gh.get('exists')} | created={gh.get('created')}\n  context: {c['context']}"
        )
    return "\n".join(lines)


def parse_response(content: str, candidates: list[dict]) -> dict:
    """Defensive parse; anything malformed becomes 'unsure' so the paper keeps flowing to search."""
    out = {"novelty": {"label": "unsure", "reason": "unparsed"}, "roles": {}}
    try:
        start, end = content.index("{"), content.rindex("}") + 1
        data = json.loads(content[start:end])
    except (ValueError, json.JSONDecodeError):
        return out
    nov = data.get("novelty") or {}
    if nov.get("label") in NOVELTY_LABELS:
        out["novelty"] = {"label": nov["label"], "reason": nov.get("reason", "")}
    for c in data.get("candidates") or []:
        if isinstance(c, dict) and c.get("role") in ROLES:
            out["roles"][str(c.get("repo_id", "")).lower()] = (c["role"], c.get("reason", ""))
    return out


def classify(
    title: str,
    first_page: str,
    candidates: list[dict],
    api_key: str,
    model: str = DEFAULT_MODEL,
) -> dict:
    r = requests.post(
        OPENROUTER_URL,
        headers={"Authorization": f"Bearer {api_key}"},
        json={
            "model": model,
            "temperature": 0,
            "response_format": {"type": "json_object"},
            "messages": [
                {"role": "system", "content": SYSTEM},
                {"role": "user", "content": build_user_message(title, first_page, candidates)},
            ],
        },
        timeout=120,
    )
    r.raise_for_status()
    body = r.json()
    parsed = parse_response(body["choices"][0]["message"]["content"], candidates)
    parsed["model"] = model
    parsed["usage"] = body.get("usage")
    return parsed
