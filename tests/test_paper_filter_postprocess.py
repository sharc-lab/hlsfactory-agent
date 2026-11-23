from hlsfactory_agent.paper_filter.postprocess import finalize, repo_outcome

PAGES = ["AutoNTT will be released soon at https://github.com/SFU-HiAccel/AutoNTT. I. INTRO",
         "[6] Microsoft SEAL (release 3.2), https://github.com/Microsoft/SEAL, Feb. 2019"]
PREPASS = {"candidates": [
    {"repo_id": "SFU-HiAccel/AutoNTT", "in_references": False},
    {"repo_id": "Microsoft/SEAL", "in_references": True}]}
HLS = {"classification": "hls_found", "repo_meta": {"fork": False}, "by_dialect": {"vitis": 3}}
NOHLS = {"classification": "repo_no_hls"}


def raw_repo(rid, scan, ev, prov="own_in_paper"):
    return {"repo_id": rid, "provenance": prov, "scan": scan, "evidence": ev}


GOOD_EV = [{"type": "paper_own_link", "source": "paper p.1",
            "quote": "AutoNTT will be released soon at https://github.com/SFU-HiAccel/AutoNTT"}]


def run(raw, snapshot=None):
    return finalize(raw, "p1", "T", PAGES, PREPASS, snapshot or {})


def test_high_confidence_hls_repo_is_handed_off():
    out = run({"repos": [raw_repo("SFU-HiAccel/AutoNTT", HLS, GOOD_EV)]})
    r = out["repos"][0]
    assert (r["outcome"], r["confidence"], r["handoff"]) == ("hls_found", "high", True)
    assert out["outcome"] == "hls_found"


def test_cited_reference_repo_never_becomes_own_evidence():
    ev = [{"type": "paper_own_link", "source": "paper p.2", "quote": "Microsoft SEAL (release 3.2)"}]
    out = run({"repos": [raw_repo("Microsoft/SEAL", NOHLS, ev)]})
    r = out["repos"][0]
    assert r["confidence"] == "low" and not r["handoff"] and r["evidence"] == []
    assert out["outcome"] == "repo_no_hls" and out["problems"]


def test_medium_confidence_is_not_handed_off():
    ev = [{"type": "name_matches_system", "source": "github_api", "quote": "repo name is AutoNTT"},
          {"type": "dates_fit", "source": "github_api", "quote": "pushed 2025-07-16"}]
    r = run({"repos": [raw_repo("x/AutoNTT", HLS, ev, "own_found_by_search")]})["repos"][0]
    assert r["confidence"] == "medium" and not r["handoff"]


def test_known_repo_is_already_known_not_handed_off():
    snap = {"sfu-hiaccel/autontt": {"status": "seeded", "display_id": "SFU-HiAccel/AutoNTT"}}
    out = run({"repos": [raw_repo("SFU-HiAccel/AutoNTT", None, GOOD_EV)]}, snap)
    assert out["repos"][0]["outcome"] == "already_known" and not out["repos"][0]["handoff"]
    assert out["outcome"] == "already_known"


def test_tapa_only_is_flagged_and_never_handed_off():
    scan = {"classification": "tapa_only"}
    r = run({"repos": [raw_repo("SFU-HiAccel/AutoNTT", scan, GOOD_EV)]})["repos"][0]
    assert r["outcome"] == "repo_no_hls" and "tapa_only" in r["flags"] and not r["handoff"]


def test_missing_scan_goes_to_review_not_handoff():
    r = run({"repos": [raw_repo("SFU-HiAccel/AutoNTT", None, GOOD_EV)]})["repos"][0]
    assert r["outcome"] == "needs_review" and not r["handoff"]


def test_dead_link():
    assert repo_outcome({"classification": "link_dead"}, False)[0] == "link_dead"


def test_invalid_repo_id_and_bad_provenance():
    out = run({"repos": [raw_repo("not a repo", HLS, GOOD_EV),
                         raw_repo("SFU-HiAccel/AutoNTT", HLS, GOOD_EV, "mystery")]})
    assert len(out["repos"]) == 1 and out["repos"][0]["confidence"] == "low"
    assert any("invalid repo id" in p for p in out["problems"])


def test_paper_level_outcomes_without_repos():
    assert run({"repos": [], "no_artifact": True})["outcome"] == "no_artifact"
    assert run({"repos": []})["outcome"] == "not_found"


def test_third_party_demo_is_flagged():
    ev = [{"type": "paper_names_repo", "source": "paper p.1",
           "quote": "AutoNTT will be released soon at https://github.com/SFU-HiAccel/AutoNTT"}]
    r = run({"repos": [raw_repo("SFU-HiAccel/AutoNTT", HLS, ev, "third_party_demo")]})["repos"][0]
    assert "third_party" in r["flags"] and r["handoff"]


def test_tapa_dominant_repo_is_not_handed_off():
    scan = {"classification": "hls_found", "mixed_tapa": True, "tapa_dominant": True}
    r = run({"repos": [raw_repo("SFU-HiAccel/AutoNTT", scan, GOOD_EV)]})["repos"][0]
    assert r["outcome"] == "hls_found" and "tapa_dominant" in r["flags"] and not r["handoff"]


def test_weak_markers_only_goes_to_review():
    r = run({"repos": [raw_repo("SFU-HiAccel/AutoNTT", {"classification": "weak_hls_only"}, GOOD_EV)]})["repos"][0]
    assert r["outcome"] == "needs_review" and "weak_hls_markers_only" in r["flags"] and not r["handoff"]


def test_tcl_and_archive_outcomes():
    assert repo_outcome({"classification": "hls_tcl_only"}, False) == ("hls_found", ["hls_via_tcl"])
    assert repo_outcome({"classification": "archive_only"}, False) == ("needs_review", ["hls_maybe_in_archive"])
