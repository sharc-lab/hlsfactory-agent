import threading

from hlsfactory_agent.paper_filter.registry import Registry, load_seeds


def final(paper_id="p1", outcome="hls_found", repo="Foo/Bar"):
    return {"paper_id": paper_id, "status": "ok", "outcome": outcome, "repos": [{
        "repo_id": repo, "provenance": "own_in_paper", "outcome": outcome,
        "confidence": "high", "handoff": outcome == "hls_found", "evidence": [],
        "scan": {"repo_meta": {"size_kb": 12, "archived": False, "fork": False}}}]}


def test_seed_snapshot_and_case_insensitive_ids(tmp_path):
    reg = Registry(tmp_path / "r.sqlite")
    reg.seed(["OswaldHe/InTAR"])
    snap = reg.snapshot()
    assert snap["oswaldhe/intar"]["status"] == "seeded"
    assert snap["oswaldhe/intar"]["display_id"] == "OswaldHe/InTAR"


def test_merge_is_idempotent(tmp_path):
    reg = Registry(tmp_path / "r.sqlite")
    reg.merge_result(final())
    reg.merge_result(final())
    assert reg.get_repo("foo/bar")["status"] == "hls_found"
    assert reg.paper_done("p1")
    with reg._conn() as c:
        assert c.execute("SELECT COUNT(*) FROM paper_repos").fetchone()[0] == 1


def test_seeded_status_is_not_overwritten(tmp_path):
    reg = Registry(tmp_path / "r.sqlite")
    reg.seed(["Foo/Bar"])
    reg.merge_result(final(outcome="repo_no_hls"))
    assert reg.get_repo("foo/bar")["status"] == "seeded"


def test_needs_review_repos_are_not_in_snapshot(tmp_path):
    reg = Registry(tmp_path / "r.sqlite")
    reg.merge_result(final(outcome="needs_review"))
    assert "foo/bar" not in reg.snapshot()


def test_concurrent_merges_lose_nothing(tmp_path):
    reg = Registry(tmp_path / "r.sqlite")
    errors = []

    def work(i):
        try:
            reg.merge_result(final(paper_id=f"p{i}", repo=f"Owner/Repo{i}"))
        except Exception as e:  # pragma: no cover
            errors.append(e)

    ts = [threading.Thread(target=work, args=(i,)) for i in range(20)]
    [t.start() for t in ts]
    [t.join() for t in ts]
    assert not errors and len(reg.snapshot()) == 20


def test_cache_ttl(tmp_path):
    reg = Registry(tmp_path / "r.sqlite")
    reg.cache_put("k", 200, "{}")
    assert reg.cache_get("k", 60) == (200, "{}")
    assert reg.cache_get("k", -1) is None


def test_load_seeds_ignores_comments(tmp_path):
    f = tmp_path / "s.txt"
    f.write_text("# c\nA/b\n\nC/d  # trailing\n")
    assert load_seeds(f) == ["A/b", "C/d"]


def test_already_known_result_does_not_turn_repo_into_seed(tmp_path):
    reg = Registry(tmp_path / "r.sqlite")
    reg.merge_result(final(paper_id="p1", outcome="hls_found"))
    reg.merge_result(final(paper_id="p2", outcome="already_known"))
    assert reg.get_repo("foo/bar")["status"] == "hls_found"


def test_rerun_does_not_see_its_own_finds_as_known(tmp_path):
    reg = Registry(tmp_path / "r.sqlite")
    reg.seed(["Seed/Repo"])
    reg.merge_result(final(paper_id="p1", repo="Own/Repo"))
    reg.merge_result(final(paper_id="p1", repo="Shared/Repo"))
    reg.merge_result(final(paper_id="p2", repo="Shared/Repo"))
    snap = reg.snapshot(exclude_papers={"p1"})
    assert "own/repo" not in snap
    assert "shared/repo" in snap and "seed/repo" in snap
    assert "own/repo" in reg.snapshot()
