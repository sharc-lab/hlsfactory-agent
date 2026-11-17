import json

import pymupdf

from hlsfactory_agent.paper_filter.manifest import discover_papers
from hlsfactory_agent.paper_filter.outputs import build_outputs
from hlsfactory_agent.paper_filter.postprocess import finalize
from hlsfactory_agent.paper_filter.runner import RunConfig, run_one

TEXT = ("Our accelerator is described here in detail. The code is available at "
        "https://github.com/acme/widget-hls for reproduction. " * 4)


def make_pdf(path, text=TEXT):
    doc = pymupdf.open()
    page = doc.new_page()
    page.insert_textbox(pymupdf.Rect(40, 40, 550, 800), text, fontsize=9)
    doc.save(path)


def test_dry_run_builds_the_work_folder(tmp_path):
    pdfs = tmp_path / "pdfs"
    pdfs.mkdir()
    make_pdf(pdfs / "Widget_Accelerator_Paper.pdf")
    (meta,) = discover_papers(pdfs)
    cfg = RunConfig(model="m", dry_run=True)
    final = run_one(meta, cfg, tmp_path / "run", snapshot={})
    area = tmp_path / "run" / "work" / meta.paper_id / "run_area"
    assert final["status"] == "prepared"
    for rel in ["paper.pdf", "pages/p01.txt", "meta.json", "prepass.json",
                "known_repos.json", "tools/gh_api.sh", "tools/hls_scan.py", "output"]:
        assert (area / rel).exists(), rel
    prepass = json.loads((area / "prepass.json").read_text())
    assert [c["repo_id"] for c in prepass["candidates"]] == ["acme/widget-hls"]
    assert (tmp_path / "run" / "papers" / f"{meta.paper_id}.json").exists()
    assert "Widget Accelerator Paper" in (area.parent / "prompt.txt").read_text()


def test_scanned_pdf_is_marked_needs_ocr_not_skipped(tmp_path):
    pdfs = tmp_path / "pdfs"
    pdfs.mkdir()
    make_pdf(pdfs / "Blank.pdf", text="x")  # almost no text layer
    (meta,) = discover_papers(pdfs)
    final = run_one(meta, RunConfig(model="m", dry_run=True), tmp_path / "run", {})
    area = tmp_path / "run" / "work" / meta.paper_id / "run_area"
    prepass = json.loads((area / "prepass.json").read_text())
    assert prepass["needs_ocr"] and final["status"] == "prepared"


def test_outputs_from_results(tmp_path):
    pages = ["see https://github.com/acme/widget-hls for the code of our accelerator"]
    ev = [{"type": "paper_own_link", "source": "paper p.1",
           "quote": "https://github.com/acme/widget-hls for the code of our accelerator"}]
    hls = {"classification": "hls_found", "by_dialect": {"vitis": 2}}
    raw = lambda rid, scan: {"repos": [{"repo_id": rid, "provenance": "own_in_paper",
                                       "scan": scan, "evidence": ev}]}
    results = [
        finalize(raw("acme/widget-hls", hls), "paper-a", "A", pages, {}, {}),
        finalize(raw("acme/widget-hls", hls), "paper-b", "B", pages, {}, {}),  # same repo twice
        finalize(raw("acme/other", {"classification": "tapa_only"}), "paper-c", "C", pages, {}, {}),
        finalize({"repos": []}, "paper-d", "D", pages, {}, {}),
        {"paper_id": "paper-e", "title": "E", "status": "timeout", "outcome": None,
         "repos": [], "error": "agent exceeded 5s"},
    ]
    stats = build_outputs(tmp_path, results)
    assert (tmp_path / "handoff_repos.txt").read_text() == "acme/widget-hls\n"
    review = (tmp_path / "review_list.csv").read_text()
    assert "acme/other" in review and "tapa_only" in review
    assert "paper-a;paper-b" in (tmp_path / "handoff.csv").read_text()
    summary = (tmp_path / "summary.csv").read_text()
    assert "paper-d" in summary and "timeout" in summary
    assert stats["handoff_repos"] == 1 and stats["by_outcome"]["not_found"] == 1
