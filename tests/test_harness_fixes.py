from pathlib import Path

from hlsfactory_agent.rewrite import _rewrite_types, outside_strings
from hlsfactory_agent.targets.catapult import CATAPULT
from hlsfactory_agent.translate import FIXED_SEED, compare_outputs, seed_random_sources
from hlsfactory_agent.translate import testbench_strings_kept as strings_kept


def test_seed_random_sources(tmp_path: Path):
    tb = tmp_path / "testbench.cpp"
    tb.write_text("std::random_device rd;\nstd::mt19937 g(rd());\nint s = std::random_device{}();\nsrand(time(NULL));\n", encoding="utf-8")
    assert seed_random_sources(tmp_path) == ["testbench.cpp"]
    text = tb.read_text(encoding="utf-8")
    assert f"std::mt19937 rd({FIXED_SEED});" in text and f"int s = {FIXED_SEED}u" in text and f"srand({FIXED_SEED})" in text
    assert "random_device" not in text.replace("was std::random_device", "")
    assert seed_random_sources(tmp_path) == []


def test_prepass_leaves_string_literals():
    log = {"residue": [], "types": []}
    line = 'printf("type ap_fixed<32, 4> x\\n"); ap_fixed<32, 4> v; ap_uint<8> "ap_int<3>" ap_int<3> w;'
    out = _rewrite_types(line, CATAPULT, log, "tb.cpp", 1)
    assert out == 'printf("type ap_fixed<32, 4> x\\n"); ac_fixed<32, 4, true> v; ac_int<8, false> "ap_int<3>" ac_int<3, true> w;'
    assert outside_strings('a "b" c', str.upper) == 'A "b" C'


def test_testbench_strings_kept(tmp_path: Path):
    inp, out = tmp_path / "in", tmp_path / "out"
    inp.mkdir(); out.mkdir()
    (inp / "synth.tcl").write_text("add_files k.cpp\nadd_files -tb tb.cpp\n", encoding="utf-8")
    (inp / "k.cpp").write_text('const char *k = "kernel only";\n', encoding="utf-8")
    (inp / "tb.cpp").write_text('#include "ap_int.h"\n// "comment"\nprintf("Data type: ap_fixed<32, 4>\\n"); printf("PASS\\\\n"); cout << "x = " << x;\n', encoding="utf-8")
    (out / "testbench.cpp").write_text('printf("Data type: ac_fixed<32, 4, true>\\n"); printf("PASS\\n"); printf("x = %d", x);\n', encoding="utf-8")
    assert strings_kept(inp, out) == ["Data type: ap_fixed<32, 4>\\n", "PASS\\\\n"]
    (out / "testbench.cpp").write_text('printf("Data type: ap_fixed<32, 4>\\n"); printf("PASS\\\\n"); printf("x = %d", x);\n', encoding="utf-8")
    assert strings_kept(inp, out) == []


def test_compare_outputs_ignores_ap_int_warnings():
    orig = "WARNING: Bitsize mismatch for ap_[u]int|=ap_[u]int.\nresult 3\nPASS\n"
    assert compare_outputs(orig, "result 3\nPASS\n")["identical"] is True
