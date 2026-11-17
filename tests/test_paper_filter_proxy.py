import json
import threading
import time
import urllib.error
import urllib.request
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

import pytest

from hlsfactory_agent.paper_filter.proxy import (
    ProxyConfig, ProxyServer, SlotLimiter, category_for)
from hlsfactory_agent.paper_filter.registry import Registry


class FakeGitHub:
    """Stands in for api.github.com; counts requests and records auth headers."""

    def __init__(self):
        self.hits = []
        outer = self

        class H(BaseHTTPRequestHandler):
            def log_message(self, *a):
                pass

            def do_GET(self):  # noqa: N802
                outer.hits.append((self.path, self.headers.get("Authorization")))
                code = 404 if "missing" in self.path else 200
                body = json.dumps({"path": self.path}).encode()
                self.send_response(code)
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)

        self.httpd = ThreadingHTTPServer(("127.0.0.1", 0), H)
        self.url = f"http://127.0.0.1:{self.httpd.server_address[1]}"
        threading.Thread(target=self.httpd.serve_forever, daemon=True).start()

    def stop(self):
        self.httpd.shutdown()


@pytest.fixture
def env(tmp_path):
    gh = FakeGitHub()
    reg = Registry(tmp_path / "r.sqlite")
    cfg = ProxyConfig(secret="s3cret", token="tok", upstream=gh.url,
                      limits_per_min={"search": 600, "code_search": 600, "core": 6000, "web": 600},
                      budgets={"search": 3, "code_search": 2, "core": 50, "web": 2},
                      log_path=tmp_path / "calls.jsonl")
    proxy = ProxyServer(cfg, reg).start()
    yield proxy, gh, tmp_path
    proxy.shutdown()
    gh.stop()


def get(proxy, path, secret="s3cret", paper="p1"):
    req = urllib.request.Request(f"http://127.0.0.1:{proxy.port}{path}",
                                 headers={"X-Proxy-Secret": secret, "X-Paper-Id": paper})
    try:
        with urllib.request.urlopen(req) as r:
            return r.status, json.loads(r.read()), r.headers.get("X-Cache")
    except urllib.error.HTTPError as e:
        return e.code, json.loads(e.read()), e.headers.get("X-Cache")


def test_category_routing():
    assert category_for("search/code?q=x") == "code_search"
    assert category_for("search/repositories?q=x") == "search"
    assert category_for("repos/a/b") == "core"


def test_slot_limiter_spaces_calls():
    lim = SlotLimiter(per_min=600)  # 0.1 s apart
    t0 = time.monotonic()
    [lim.acquire() for _ in range(5)]
    assert time.monotonic() - t0 >= 0.35


def test_token_is_added_upstream_and_response_passed_through(env):
    proxy, gh, _ = env
    status, body, cache = get(proxy, "/github/repos/a/b")
    assert status == 200 and body["path"] == "/repos/a/b" and cache == "miss"
    assert gh.hits[0][1] == "Bearer tok"


def test_identical_query_is_served_from_cache(env):
    proxy, gh, _ = env
    get(proxy, "/github/search/repositories?q=InTAR")
    status, _, cache = get(proxy, "/github/search/repositories?q=InTAR", paper="other")
    assert status == 200 and cache == "hit" and len(gh.hits) == 1


def test_404_is_cached_but_errors_are_not(env):
    proxy, gh, _ = env
    assert get(proxy, "/github/repos/missing/x")[0] == 404
    assert get(proxy, "/github/repos/missing/x")[2] == "hit"


def test_bad_secret_is_rejected(env):
    proxy, gh, _ = env
    assert get(proxy, "/github/repos/a/b", secret="nope")[0] == 403
    assert gh.hits == []


def test_per_paper_budget_is_enforced_even_for_cache_hits(env):
    proxy, gh, _ = env
    codes = [get(proxy, "/github/search/code?q=title", paper="p1")[0] for _ in range(3)]
    assert codes == [200, 200, 429]
    assert get(proxy, "/github/search/code?q=title", paper="p2")[0] == 200  # separate budget
    assert len(gh.hits) == 1


def test_web_search_without_provider_is_501(env):
    proxy, _, _ = env
    assert get(proxy, "/web/search?q=x")[0] == 501


def test_web_provider_plugs_in(tmp_path):
    class P:
        def search(self, q):
            return {"results": [q]}

    reg = Registry(tmp_path / "r.sqlite")
    proxy = ProxyServer(ProxyConfig(secret="s", upstream="http://127.0.0.1:1"), reg,
                        web_provider=P()).start()
    try:
        assert get(proxy, "/web/search?q=hello", secret="s")[1] == {"results": ["hello"]}
    finally:
        proxy.shutdown()


def test_concurrent_callers_are_throttled_not_failed(env):
    proxy, gh, _ = env
    results = []
    threads = [threading.Thread(target=lambda i=i: results.append(
        get(proxy, f"/github/search/repositories?q=q{i}", paper=f"p{i}")[0])) for i in range(12)]
    t0 = time.monotonic()
    [t.start() for t in threads]
    [t.join() for t in threads]
    assert results == [200] * 12
    assert time.monotonic() - t0 >= 1.0  # 12 calls at 600/min cannot finish faster


def test_calls_are_logged(env):
    proxy, _, tmp = env
    get(proxy, "/github/repos/a/b")
    lines = [json.loads(line) for line in (tmp / "calls.jsonl").read_text().splitlines()]
    assert lines[0]["category"] == "core" and lines[0]["status"] == 200
