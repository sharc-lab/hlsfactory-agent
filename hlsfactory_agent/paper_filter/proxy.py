"""Host-side proxy for GitHub (and web search) calls from the agent containers.

Adds the token, enforces rate limits by WAITING (not failing), caches identical queries
across agents, enforces a per-paper search budget, and logs every call. Runs as a thread in
the orchestrator. See CLAUDE.md, chunk 6.

    containers --curl--> http://host.docker.internal:PORT/github/<path>  --> api.github.com
                                                          /web/search?q=...
"""

import hmac
import json
import subprocess
import sys
import threading
import time
import urllib.error
import urllib.parse
import urllib.request
from dataclasses import dataclass, field
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

from hlsfactory_agent.paper_filter.registry import Registry

GITHUB_API = "https://api.github.com"


@dataclass
class ProxyConfig:
    secret: str
    token: str | None = None
    upstream: str = GITHUB_API
    # requests per minute, kept under GitHub's documented 30 / 9 per minute and 5000 per hour
    limits_per_min: dict = field(
        default_factory=lambda: {"search": 27, "code_search": 8, "core": 75, "web": 30}
    )
    # per-paper caps on calls (cache hits count too, so a loop cannot hide behind the cache)
    budgets: dict = field(
        default_factory=lambda: {"search": 12, "code_search": 4, "core": 80, "web": 3}
    )
    cache_ttl_s: float = 24 * 3600
    max_retries: int = 3
    max_wait_s: float = 120.0
    log_path: Path | None = None


class SlotLimiter:
    """Hands out evenly spaced time slots; callers sleep until their slot."""

    def __init__(self, per_min: float):
        self.interval = 60.0 / per_min
        self._next = 0.0
        self._lock = threading.Lock()

    def acquire(self) -> float:
        with self._lock:
            now = time.monotonic()
            slot = max(now, self._next)
            self._next = slot + self.interval
        wait = slot - now
        if wait > 0:
            time.sleep(wait)
        return max(wait, 0.0)


class NullProvider:
    """Default web search provider: none configured. The agent must cope without it."""

    def search(self, query: str) -> dict:
        raise NotImplementedError


def category_for(path: str) -> str:
    p = path.lstrip("/")
    if p.startswith("search/code"):
        return "code_search"
    if p.startswith("search/"):
        return "search"
    return "core"


class _HTTPServer(ThreadingHTTPServer):
    # The default listen backlog is 5; dozens of agents connecting at once would be refused.
    request_queue_size = 256
    daemon_threads = True


class ProxyServer:
    def __init__(
        self,
        cfg: ProxyConfig,
        registry: Registry,
        host: str = "127.0.0.1",
        port: int = 0,
        web_provider=None,
    ):
        self.cfg = cfg
        self.registry = registry
        self.web = web_provider or NullProvider()
        self.limiters = {k: SlotLimiter(v) for k, v in cfg.limits_per_min.items()}
        self.counts: dict[tuple[str, str], int] = {}
        self._lock = threading.Lock()
        self._log_lock = threading.Lock()
        self.httpd = _HTTPServer((host, port), _make_handler(self))
        self.port = self.httpd.server_address[1]
        self._thread = threading.Thread(target=self.httpd.serve_forever, daemon=True)

    def start(self) -> "ProxyServer":
        self._thread.start()
        return self

    def shutdown(self) -> None:
        self.httpd.shutdown()
        self.httpd.server_close()

    def over_budget(self, paper_id: str, category: str) -> bool:
        limit = self.cfg.budgets.get(category)
        with self._lock:
            n = self.counts.get((paper_id, category), 0) + 1
            self.counts[(paper_id, category)] = n
        return limit is not None and n > limit

    def usage(self, paper_id: str) -> dict:
        with self._lock:
            return {c: n for (p, c), n in self.counts.items() if p == paper_id}

    def log(self, **entry) -> None:
        if not self.cfg.log_path:
            return
        entry["ts"] = round(time.time(), 3)
        with self._log_lock, open(self.cfg.log_path, "a") as f:
            f.write(json.dumps(entry) + "\n")

    def fetch_upstream(self, url: str, category: str) -> tuple[int, str, float]:
        """GET with the token, waiting out the limiter and retrying rate-limit responses."""
        waited = self.limiters[category].acquire()
        headers = {"Accept": "application/vnd.github+json", "User-Agent": "paper-filter"}
        if self.cfg.token:
            headers["Authorization"] = f"Bearer {self.cfg.token}"
        for attempt in range(self.cfg.max_retries + 1):
            req = urllib.request.Request(url, headers=headers)
            try:
                with urllib.request.urlopen(req, timeout=60) as r:
                    return r.status, r.read().decode(), waited
            except urllib.error.HTTPError as e:
                body = e.read().decode(errors="replace")
                retry_after = e.headers.get("Retry-After")
                reset = e.headers.get("X-RateLimit-Reset")
                limited = e.code in (403, 429) and (
                    retry_after is not None
                    or e.headers.get("X-RateLimit-Remaining") == "0"
                )
                if limited and attempt < self.cfg.max_retries:
                    if retry_after is not None:
                        delay = float(retry_after)
                    elif reset is not None:
                        delay = max(float(reset) - time.time(), 1.0)
                    else:
                        delay = 30.0
                    delay = min(delay, self.cfg.max_wait_s)
                    time.sleep(delay)
                    waited += delay
                    continue
                return e.code, body, waited
            except (urllib.error.URLError, TimeoutError) as e:
                if attempt < self.cfg.max_retries:
                    time.sleep(2 ** attempt)
                    continue
                return 502, json.dumps({"error": f"upstream unreachable: {e}"}), waited
        return 502, json.dumps({"error": "retries exhausted"}), waited


def _make_handler(server: ProxyServer):
    class Handler(BaseHTTPRequestHandler):
        protocol_version = "HTTP/1.1"

        def log_message(self, *args):  # silence default stderr logging
            pass

        def _reply(self, status: int, body: str, extra: dict | None = None) -> None:
            data = body.encode()
            self.send_response(status)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(data)))
            for k, v in (extra or {}).items():
                self.send_header(k, str(v))
            self.end_headers()
            self.wfile.write(data)

        def do_GET(self):  # noqa: N802
            secret = self.headers.get("X-Proxy-Secret", "")
            if not hmac.compare_digest(secret, server.cfg.secret):
                return self._reply(403, json.dumps({"error": "bad or missing secret"}))
            paper_id = self.headers.get("X-Paper-Id", "unknown")
            parsed = urllib.parse.urlsplit(self.path)

            if parsed.path.startswith("/github/"):
                rel = parsed.path[len("/github/"):]
                shown = rel + (f"?{parsed.query}" if parsed.query else "")  # for the log
                category = category_for(rel)
                url = f"{server.cfg.upstream}/{rel}" + (
                    f"?{parsed.query}" if parsed.query else ""
                )
            elif parsed.path == "/web/search":
                category, url = "web", None
            else:
                return self._reply(404, json.dumps({"error": "unknown route"}))

            if server.over_budget(paper_id, category):
                server.log(paper=paper_id, category=category, path=self.path, status=429,
                           budget_exhausted=True)  # fmt: skip
                return self._reply(
                    429,
                    json.dumps({
                        "error": "budget_exhausted",
                        "category": category,
                        "limit": server.cfg.budgets.get(category),
                        "hint": "stop searching this way and report what you have",
                    }),
                )  # fmt: skip

            if category == "web":
                query = urllib.parse.parse_qs(parsed.query).get("q", [""])[0]
                server.limiters["web"].acquire()
                try:
                    out = server.web.search(query)
                except NotImplementedError:
                    server.log(paper=paper_id, category=category, status=501)
                    return self._reply(
                        501, json.dumps({"error": "no web search provider configured"})
                    )
                except Exception as e:  # provider failure must not crash the proxy
                    server.log(paper=paper_id, category=category, status=502, error=str(e))
                    return self._reply(502, json.dumps({"error": f"provider error: {e}"}))
                server.log(paper=paper_id, category=category, status=200, query=query)
                return self._reply(200, json.dumps(out))

            cached = server.registry.cache_get(url, server.cfg.cache_ttl_s)
            if cached:
                status, body = cached
                server.log(paper=paper_id, category=category, path=shown, status=status,
                           cached=True)  # fmt: skip
                return self._reply(status, body, {"X-Cache": "hit"})
            status, body, waited = server.fetch_upstream(url, category)
            if status in (200, 404):
                server.registry.cache_put(url, status, body)
            server.log(paper=paper_id, category=category, path=shown, status=status,
                       cached=False, waited_s=round(waited, 2))  # fmt: skip
            return self._reply(status, body, {"X-Cache": "miss"})

    return Handler


def bridge_address() -> str:
    """Address to bind so containers can reach the proxy without exposing it to the LAN.

    Linux: the Docker bridge gateway (usually 172.17.0.1). Elsewhere, or on any failure:
    127.0.0.1 (Docker Desktop forwards host.docker.internal to the host's loopback).
    """
    if sys.platform == "linux":
        try:
            out = subprocess.run(
                ["docker", "network", "inspect", "bridge", "-f",
                 "{{(index .IPAM.Config 0).Gateway}}"],
                capture_output=True, text=True, timeout=15,
            )  # fmt: skip
            addr = out.stdout.strip()
            if out.returncode == 0 and addr:
                return addr
        except (OSError, subprocess.TimeoutExpired):
            pass
    return "127.0.0.1"
