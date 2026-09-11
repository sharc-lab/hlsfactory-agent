from pathlib import Path

from hlsfactory_agent.rewrite import rewrite_design
from hlsfactory_agent.translate import CATAPULT, build_translate_prompt, check_translated_design

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = ROOT / "exp" / "run_translate" / "designs" / "vitis_mac"
EXPECTED = ROOT / "exp" / "run_translate" / "expected" / "catapult_mac"


def _norm(p: Path) -> str:
    return "\n".join(" ".join(l.split()) for l in p.read_text(encoding="utf-8").splitlines() if l.strip())


def test_prepass_reproduces_hand_translation_of_fixture(tmp_path: Path):
    log = rewrite_design(FIXTURE, tmp_path / "out", CATAPULT, top="mac")
    for name in ("mac.h", "mac.cpp", "testbench.cpp"):
        assert _norm(tmp_path / "out" / name) == _norm(EXPECTED / name), name
    assert log["residue"] == []
    assert {d["kind"] for d in log["pragmas_dropped"]} == {"interface", "array_partition"}
    assert len(log["pragmas_moved"]) == 1 and "hls_pipeline_init_interval 1" in log["pragmas_moved"][0]["after"]
    assert log["top_marker"]["inserted"] is True
    assert not (tmp_path / "out" / "synth.tcl").exists()
    assert (tmp_path / "out" / "rewrite_log.json").exists()


def test_prepass_maps_explicit_modes_and_flags_unknown_ones(tmp_path: Path):
    d = tmp_path / "in"
    d.mkdir()
    (d / "k.cpp").write_text(
        '#include "ap_fixed.h"\n'
        "typedef ap_fixed<8, 4, AP_RND, AP_SAT> a_t;\n"
        "typedef ap_fixed<8, 4, AP_RND_ZERO, AP_SAT_SYM> b_t;\n"
        "void top(a_t x) { ap_uint<3> y = x.range(2, 0); }\n",
        encoding="utf-8",
    )
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    text = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "ac_fixed<8, 4, true, AC_RND, AC_SAT> a_t" in text
    assert "ac_int<3, false> y" in text
    assert any("AP_RND_ZERO" in r["reason"] for r in log["residue"])
    assert any(".range(" in r["reason"] for r in log["residue"])


def test_prepass_keeps_symbolic_unroll_factor_and_flags_it(tmp_path: Path):
    d = tmp_path / "in"
    d.mkdir()
    (d / "k.cpp").write_text(
        "void top(int a[16]) {\n"
        "    for (int i = 0; i < 16; i++) {\n"
        "#pragma HLS UNROLL factor=UF\n"
        "        a[i] += 1;\n"
        "    }\n"
        "}\n",
        encoding="utf-8",
    )
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    text = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "#pragma hls_unroll UF" in text
    assert text.index("#pragma hls_unroll UF") < text.index("for (int i")
    assert any("UF" in r["reason"] for r in log["residue"])


def test_prepass_output_passes_static_check(tmp_path: Path):
    rewrite_design(FIXTURE, tmp_path / "out", CATAPULT, top="mac")
    (tmp_path / "out" / "run.tcl").write_text("solution file add\ngo analyze\ngo extract\n", encoding="utf-8")
    (tmp_path / "out" / "translation_report.md").write_text("pipeline interface array_partition\n", encoding="utf-8")
    r = check_translated_design(tmp_path / "out", CATAPULT)
    assert r["passed"], r["failures"]


def test_prompt_with_prepass_points_at_rewrite_log():
    p = build_translate_prompt("k", CATAPULT, prepass=True)
    assert "rewrite_log.json" in p and "residue" in p
    assert "mechanical pre-pass" in p
