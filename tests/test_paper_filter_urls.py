from hlsfactory_agent.paper_filter.urls import (
    Hit,
    merge_hits,
    parse_github_ref,
    scan_annotations,
    scan_text,
)


def ids(scan):
    return [h.repo_id for h in scan.hits]


def test_footnote_number_glued_to_url():
    pages = ["temporal) executions [8]–[11]\n1https://github.com/OswaldHe/InTAR\nmore"]
    assert ids(scan_text(pages)) == ["OswaldHe/InTAR"]


def test_trailing_punctuation_and_git_suffix():
    pages = ["see https://github.com/foo/bar.git), and github.com/a/b-c."]
    assert ids(scan_text(pages)) == ["foo/bar", "a/b-c"]


def test_line_break_after_slash_is_repaired():
    pages = ["code at https://github.com/owner/\nrepo-name for details"]
    scan = scan_text(pages)
    assert ids(scan) == ["owner/repo-name"]
    assert scan.hits[0].repaired


def test_line_break_after_hyphen_is_repaired():
    pages = ["https://github.com/owner/my-\ntool next"]
    assert ids(scan_text(pages)) == ["owner/my-tool"]


def test_plain_sentence_end_is_not_repaired():
    pages = ["see https://github.com/owner/repo\nNext sentence"]
    scan = scan_text(pages)
    assert ids(scan) == ["owner/repo"]
    assert not scan.hits[0].repaired


def test_profile_only_link_is_not_a_repo():
    scan = scan_text(["our group: https://github.com/SFU-HiAccel"])
    assert scan.hits == []
    assert scan.profiles == ["SFU-HiAccel"]


def test_site_pages_are_ignored():
    assert scan_text(["https://github.com/features/actions"]).hits == []


def test_references_flag():
    pages = ["intro https://github.com/own/repo", "REFERENCES\n[1] https://github.com/Microsoft/SEAL, 2019"]
    hits = {h.repo_id: h for h in scan_text(pages).hits}
    assert hits["own/repo"].in_references is False
    assert hits["Microsoft/SEAL"].in_references is True


def test_annotation_does_not_override_references_flag():
    text_hit = Hit("a/b", "github.com/a/b", 9, "text", in_references=True)
    ann_hit = scan_annotations([(9, "https://github.com/A/B")])[0]
    (m,) = merge_hits([text_hit, ann_hit])
    assert m["in_references"] is True
    assert m["found_via"] == ["text", "annotation"]


def test_annotation_only_reference_status_unknown():
    (m,) = merge_hits(scan_annotations([(1, "https://github.com/a/b/tree/main/x")]))
    assert m["repo_id"] == "a/b"
    assert m["in_references"] is None


def test_parse_non_github():
    assert parse_github_ref("https://gitlab.com/a/b") is None
