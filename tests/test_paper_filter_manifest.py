import pandas as pd

from hlsfactory_agent.paper_filter.manifest import discover_papers, make_paper_id, norm_title


def make(tmp_path, titles, files):
    pd.DataFrame({"Title": titles, "Authors": ["A, B"] * len(titles),
                  "Year": [2025] * len(titles), "Venue": ["V"] * len(titles),
                  "DOI": ["10.1/x"] * len(titles)}).to_csv(tmp_path / "s.csv", index=False)
    pdfs = tmp_path / "pdfs"
    pdfs.mkdir()
    for f in files:
        (pdfs / f).write_bytes(b"%PDF-1.4")
    return pdfs, tmp_path / "s.csv"


def test_exact_normalized_title_match(tmp_path):
    pdfs, sheet = make(tmp_path, ["AutoNTT: Automatic Architecture Design"],
                       ["AutoNTT_Automatic_Architecture_Design.pdf"])
    (p,) = discover_papers(pdfs, sheet)
    assert p.matched and p.title == "AutoNTT: Automatic Architecture Design"
    assert p.year == 2025 and p.doi == "10.1/x" and not p.ambiguous


def test_truncated_filename_prefix_match(tmp_path):
    full = "A Very Long Paper Title About High Level Synthesis of Accelerators on FPGAs"
    stem = "_".join(full.split()[:9])
    pdfs, sheet = make(tmp_path, [full, "Something else entirely different"], [stem + ".pdf"])
    (p,) = discover_papers(pdfs, sheet)
    assert p.matched and p.title == full


def test_unmatched_falls_back_to_filename(tmp_path):
    pdfs, sheet = make(tmp_path, ["Other"], ["Unknown_Paper_Name.pdf"])
    (p,) = discover_papers(pdfs, sheet)
    assert not p.matched and p.title == "Unknown Paper Name" and p.doi is None


def test_duplicate_titles_are_flagged_ambiguous(tmp_path):
    pdfs, sheet = make(tmp_path, ["Same Title", "same title!"], ["Same_Title.pdf"])
    (p,) = discover_papers(pdfs, sheet)
    assert p.matched and p.ambiguous


def test_no_sheet_still_works(tmp_path):
    pdfs, _ = make(tmp_path, ["x"], ["a_b.pdf"])
    (p,) = discover_papers(pdfs, None)
    assert p.title == "a b" and not p.matched


def test_long_ids_are_truncated_and_unique():
    a, b = make_paper_id("x" * 200), make_paper_id("x" * 199 + "y")
    assert len(a) < 100 and a != b
    assert norm_title("A-b: C!") == "abc"


def test_subfolders_become_groups(tmp_path):
    pdfs = tmp_path / "pdfs"
    for sub, name in (("with_repo", "A_Paper.pdf"), ("without_repo", "B_Paper.pdf"), (".", "C.pdf")):
        (pdfs / sub).mkdir(parents=True, exist_ok=True)
        (pdfs / sub / name).write_bytes(b"%PDF-1.4")
    groups = {p.paper_id: p.group for p in discover_papers(pdfs)}
    assert groups == {"A_Paper": "with_repo", "B_Paper": "without_repo", "C": None}


def test_same_filename_in_two_folders_is_an_error(tmp_path):
    import pytest
    pdfs = tmp_path / "pdfs"
    for sub in ("with_repo", "without_repo"):
        (pdfs / sub).mkdir(parents=True)
        (pdfs / sub / "Same.pdf").write_bytes(b"%PDF-1.4")
    with pytest.raises(ValueError, match="duplicate paper id"):
        discover_papers(pdfs)
