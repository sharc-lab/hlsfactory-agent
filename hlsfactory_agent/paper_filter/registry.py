"""SQLite registry of discovered repos (one file, standard library, no server).

The host process owns it; containers only ever see a read-only snapshot. Every method opens
its own short-lived connection (WAL mode, busy timeout) so threads and processes can share
the file safely. See CLAUDE.md, chunk 6.
"""

import json
import sqlite3
import time
from contextlib import contextmanager
from pathlib import Path

SCHEMA = """
CREATE TABLE IF NOT EXISTS repos (
    repo_id TEXT PRIMARY KEY,          -- lowercase owner/name
    display_id TEXT NOT NULL,
    status TEXT NOT NULL,              -- seeded | hls_found | repo_no_hls | link_dead | needs_review
    size_kb INTEGER,
    archived INTEGER,
    fork INTEGER,
    scan_json TEXT,
    checked_at REAL,
    source TEXT
);
CREATE TABLE IF NOT EXISTS paper_repos (
    paper_id TEXT NOT NULL,
    repo_id TEXT NOT NULL,
    provenance TEXT,
    outcome TEXT,
    confidence TEXT,
    handoff INTEGER,
    evidence_json TEXT,
    PRIMARY KEY (paper_id, repo_id)
);
CREATE TABLE IF NOT EXISTS papers (
    paper_id TEXT PRIMARY KEY,
    status TEXT,
    outcome TEXT,
    result_path TEXT,
    finished_at REAL
);
CREATE TABLE IF NOT EXISTS search_cache (
    key TEXT PRIMARY KEY,
    status INTEGER,
    body TEXT,
    fetched_at REAL
);
"""

# Repos with these statuses are known: a paper pointing at one need not be re-searched.
# needs_review is excluded because a failed scan deserves another try.
UNKNOWN_STATUSES = {"needs_review"}


def load_seeds(path: Path) -> list[str]:
    out = []
    for line in Path(path).read_text().splitlines():
        line = line.split("#", 1)[0].strip()
        if line:
            out.append(line)
    return out


class Registry:
    def __init__(self, path: Path):
        self.path = Path(path)
        self.path.parent.mkdir(parents=True, exist_ok=True)
        with self._conn() as c:
            c.executescript(SCHEMA)

    @contextmanager
    def _conn(self):
        c = sqlite3.connect(self.path, timeout=30)
        c.row_factory = sqlite3.Row
        try:
            c.execute("PRAGMA journal_mode=WAL")
            c.execute("PRAGMA busy_timeout=30000")
            yield c
            c.commit()
        finally:
            c.close()

    # --- repos ----------------------------------------------------------------------

    def seed(self, repo_ids: list[str], source: str = "seed") -> None:
        with self._conn() as c:
            c.executemany(
                "INSERT OR IGNORE INTO repos(repo_id, display_id, status, checked_at, source)"
                " VALUES (?, ?, 'seeded', ?, ?)",
                [(r.lower(), r, time.time(), source) for r in repo_ids],
            )

    def snapshot(self) -> dict[str, dict]:
        """Known repos as {lowercase id: {status, display_id}} for the agent's read-only copy."""
        with self._conn() as c:
            rows = c.execute("SELECT repo_id, display_id, status FROM repos").fetchall()
        return {
            r["repo_id"]: {"status": r["status"], "display_id": r["display_id"]}
            for r in rows
            if r["status"] not in UNKNOWN_STATUSES
        }

    def get_repo(self, repo_id: str) -> dict | None:
        with self._conn() as c:
            r = c.execute(
                "SELECT * FROM repos WHERE repo_id = ?", (repo_id.lower(),)
            ).fetchone()
        return dict(r) if r else None

    # --- per-paper results ----------------------------------------------------------

    def merge_result(self, final: dict, result_path: str | None = None) -> None:
        """Idempotent: merging the same result twice leaves the same rows."""
        now = time.time()
        with self._conn() as c:
            for repo in final.get("repos", []):
                rid = repo["repo_id"]
                scan = repo.get("scan") or {}
                meta = scan.get("repo_meta") or {}
                c.execute(
                    "INSERT INTO repos(repo_id, display_id, status, size_kb, archived, fork,"
                    " scan_json, checked_at, source) VALUES (?,?,?,?,?,?,?,?,?)"
                    " ON CONFLICT(repo_id) DO UPDATE SET"
                    "  status = CASE WHEN repos.status = 'seeded' THEN repos.status"
                    "                ELSE excluded.status END,"
                    "  size_kb = excluded.size_kb, archived = excluded.archived,"
                    "  fork = excluded.fork, scan_json = excluded.scan_json,"
                    "  checked_at = excluded.checked_at",
                    (
                        rid.lower(),
                        rid,
                        repo["outcome"] if repo["outcome"] != "already_known" else "seeded",
                        meta.get("size_kb"),
                        _b(meta.get("archived")),
                        _b(meta.get("fork")),
                        json.dumps(scan) if scan else None,
                        now,
                        final["paper_id"],
                    ),
                )
                c.execute(
                    "INSERT INTO paper_repos(paper_id, repo_id, provenance, outcome,"
                    " confidence, handoff, evidence_json) VALUES (?,?,?,?,?,?,?)"
                    " ON CONFLICT(paper_id, repo_id) DO UPDATE SET"
                    "  provenance=excluded.provenance, outcome=excluded.outcome,"
                    "  confidence=excluded.confidence, handoff=excluded.handoff,"
                    "  evidence_json=excluded.evidence_json",
                    (
                        final["paper_id"],
                        rid.lower(),
                        repo.get("provenance"),
                        repo["outcome"],
                        repo.get("confidence"),
                        int(bool(repo.get("handoff"))),
                        json.dumps(repo.get("evidence", [])),
                    ),
                )
            c.execute(
                "INSERT INTO papers(paper_id, status, outcome, result_path, finished_at)"
                " VALUES (?,?,?,?,?) ON CONFLICT(paper_id) DO UPDATE SET"
                "  status=excluded.status, outcome=excluded.outcome,"
                "  result_path=excluded.result_path, finished_at=excluded.finished_at",
                (final["paper_id"], final.get("status"), final.get("outcome"), result_path, now),
            )

    def paper_done(self, paper_id: str) -> bool:
        with self._conn() as c:
            r = c.execute(
                "SELECT status FROM papers WHERE paper_id = ?", (paper_id,)
            ).fetchone()
        return bool(r and r["status"] == "ok")

    # --- proxy cache ----------------------------------------------------------------

    def cache_get(self, key: str, ttl_s: float) -> tuple[int, str] | None:
        with self._conn() as c:
            r = c.execute(
                "SELECT status, body, fetched_at FROM search_cache WHERE key = ?", (key,)
            ).fetchone()
        if r and time.time() - r["fetched_at"] <= ttl_s:
            return r["status"], r["body"]
        return None

    def cache_put(self, key: str, status: int, body: str) -> None:
        with self._conn() as c:
            c.execute(
                "INSERT INTO search_cache(key, status, body, fetched_at) VALUES (?,?,?,?)"
                " ON CONFLICT(key) DO UPDATE SET status=excluded.status,"
                " body=excluded.body, fetched_at=excluded.fetched_at",
                (key, status, body, time.time()),
            )


def _b(v: object) -> int | None:
    return None if v is None else int(bool(v))
