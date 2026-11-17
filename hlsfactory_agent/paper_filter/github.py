import json
import os
from pathlib import Path

import requests
from dotenv import dotenv_values

API = "https://api.github.com/repos/{repo_id}"


def load_token(env_path: Path = Path(".env")) -> str | None:
    """GITHUB_TOKEN from the environment or .env; empty/placeholder values count as unset."""
    token = os.environ.get("GITHUB_TOKEN") or dotenv_values(env_path).get(
        "GITHUB_TOKEN"
    )
    if not token or token.startswith("your_"):
        return None
    return token


def check_repo(repo_id: str, token: str | None, cache_dir: Path | None = None) -> dict:
    """Existence/rename/metadata for one repo. Cached on disk (errors are not cached)."""
    cache_file = None
    if cache_dir is not None:
        cache_dir.mkdir(parents=True, exist_ok=True)
        cache_file = cache_dir / (repo_id.lower().replace("/", "__") + ".json")
        if cache_file.exists():
            return json.loads(cache_file.read_text())

    headers = {"Accept": "application/vnd.github+json"}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    try:
        r = requests.get(API.format(repo_id=repo_id), headers=headers, timeout=20)
    except requests.RequestException as e:
        return {"checked": False, "error": str(e)}

    if r.status_code == 200:
        d = r.json()
        result = {
            "checked": True,
            "exists": True,
            "full_name": d["full_name"],
            "renamed_to": d["full_name"] if d["full_name"].lower() != repo_id.lower() else None,
            "language": d.get("language"),
            "created": d.get("created_at"),
            "pushed": d.get("pushed_at"),
            "archived": d.get("archived"),
            "fork": d.get("fork"),
            "stars": d.get("stargazers_count"),
            "description": d.get("description"),
        }
    elif r.status_code == 404:
        result = {"checked": True, "exists": False}
    else:
        # 403/429 rate limit, 5xx, etc.: report, don't cache.
        return {"checked": False, "error": f"HTTP {r.status_code}", "authenticated": bool(token)}

    if cache_file is not None:
        cache_file.write_text(json.dumps(result, indent=2))
    return result
