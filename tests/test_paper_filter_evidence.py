from hlsfactory_agent.paper_filter.evidence import clean_evidence, score, verify_quote

PAGES = ["AutoNTT will be released soon at https://github.com/SFU-HiAccel/AutoNTT. I. INTRO",
         "[6] Microsoft SEAL (release 3.2), https://github.com/Microsoft/SEAL, Feb. 2019"]


def test_quote_check_tolerates_line_breaks_and_hyphenation():
    pages = ["our accelera-\ntor code is avail-\nable at"]
    assert verify_quote("our accelerator code is available at", pages)
    assert not verify_quote("totally different sentence here", pages)


def test_unverifiable_paper_quote_is_dropped():
    items = [{"type": "paper_own_link", "quote": "the code is on github, trust me", "source": "paper p.1"}]
    kept, problems = clean_evidence(items, PAGES)
    assert kept == [] and "not found in the PDF" in problems[0]


def test_reference_only_link_cannot_be_own_evidence():
    items = [{"type": "paper_own_link", "quote": "Microsoft SEAL (release 3.2)", "source": "paper p.2"}]
    kept, problems = clean_evidence(items, PAGES, repo_in_references_only=True)
    assert kept == [] and "references" in problems[0]


def test_paper_link_types_must_cite_the_paper():
    items = [{"type": "paper_own_link", "quote": "some readme text here", "source": "readme"}]
    assert clean_evidence(items, PAGES)[0] == []


def test_missing_quote_and_unknown_type_are_problems():
    items = [{"type": "readme_has_title", "quote": "", "source": "readme"},
             {"type": "made_up", "quote": "long enough quote", "source": "readme"}]
    kept, problems = clean_evidence(items, PAGES)
    assert kept == [] and len(problems) == 2


def test_score_rubric():
    assert score({"paper_own_link"}) == "high"
    assert score({"readme_has_title"}) == "high"
    assert score({"name_matches_system", "owner_matches_affiliation"}) == "high"
    assert score({"name_matches_system", "dates_fit"}) == "medium"
    assert score({"name_matches_system"}) == "low"
    assert score({"owner_matches_affiliation"}) == "low"
    assert score({"dates_fit"}) == "low"
    assert score(set()) == "low"


def test_negative_evidence_caps_at_medium_unless_paper_gives_the_link():
    assert score({"readme_has_title", "is_fork"}) == "medium"
    assert score({"paper_own_link", "is_fork"}) == "high"
