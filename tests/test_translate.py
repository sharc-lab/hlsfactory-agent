from pathlib import Path

import pytest

from hlsfactory_agent.translate import (
    CATAPULT,
    OUTPUT_DIR_NAME,
    build_translate_prompt,
    check_translated_design,
    find_top_from_synth_tcl,
    get_target,
    render_catapult_run_tcl,
)

GOOD_KERNEL = """#include "ac_fixed.h"
#include "ac_channel.h"
typedef ac_fixed<16, 8, true> data_t;
#pragma hls_design top
void mac(ac_channel<data_t> &in, ac_channel<data_t> &out) {
#pragma hls_pipeline_init_interval 1
    for (int i = 0; i < 8; i++) {
        out.write(in.read());
    }
}
"""

GOOD_TB = """#include "ac_fixed.h"
int main() { return 0; }
"""


def make_good_design(root: Path) -> Path:
    out = root / OUTPUT_DIR_NAME
    out.mkdir()
    (out / "mac.cpp").write_text(GOOD_KERNEL)
    (out / "testbench.cpp").write_text(GOOD_TB)
    (out / "run.tcl").write_text(render_catapult_run_tcl("mac", ["mac.cpp"], "testbench.cpp"))
    (out / "translation_report.md").write_text("# report\n")
    return out


def test_catapult_target_is_registered():
    t = get_target("catapult")
    assert t is CATAPULT
    assert "ap_int<N>" in t.type_map
    assert any("hls_pipeline_init_interval" in (r.target or "") for r in t.pragma_rules)
    assert any(r.target is None and "ARRAY_PARTITION" in r.source for r in t.pragma_rules)


def test_unknown_target_raises():
    with pytest.raises(ValueError):
        get_target("intel")


def test_prompt_contains_the_essentials():
    p = build_translate_prompt("vitis_mac", CATAPULT)
    for needle in (
        "ac_int<N, true>",
        "ac_channel<T>",
        "hls_pipeline_init_interval",
        "BEFORE the `for`",
        "#pragma hls_design top",
        "/workspace/run_area/ac_types_include",
        "translation_report.md",
        "run.tcl",
        "go extract",
        "timeout 30s",
        "vitis_mac",
    ):
        assert needle in p, needle


def test_render_run_tcl_has_required_tokens():
    tcl = render_catapult_run_tcl("mac", ["mac.cpp", "util.cpp"], "testbench.cpp", clock_period_ns=4.0)
    for token in CATAPULT.driver_required_tokens:
        assert token in tcl
    assert "solution file add ./mac.cpp -type C++" in tcl
    assert "solution file add ./util.cpp -type C++" in tcl
    assert "testbench.cpp -type C++ -exclude true" in tcl
    assert "-DESIGN_HIERARCHY mac" in tcl
    assert "-CLOCK_PERIOD 4.0" in tcl


def test_find_top_from_synth_tcl(tmp_path: Path):
    (tmp_path / "synth.tcl").write_text("open_project p\nset_top   mac\nadd_files mac.cpp\n")
    assert find_top_from_synth_tcl(tmp_path) == "mac"
    assert find_top_from_synth_tcl(tmp_path / "nope") is None


def test_check_passes_on_good_design(tmp_path: Path):
    out = make_good_design(tmp_path)
    r = check_translated_design(out, CATAPULT)
    assert r["passed"], r["failures"]
    assert r["leftovers"] == []


def test_check_flags_vitis_leftovers(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "mac.cpp").write_text(GOOD_KERNEL.replace("ac_fixed<16, 8, true>", "ap_fixed<16, 8>"))
    r = check_translated_design(out, CATAPULT)
    assert not r["passed"]
    assert r["leftovers"] and r["leftovers"][0]["file"] == "mac.cpp"
    assert any("dialect" in f for f in r["failures"])


def test_check_ignores_commented_out_pragmas_but_records_them(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "mac.cpp").write_text(GOOD_KERNEL + "\n    // #pragma HLS array_partition variable=x complete\n")
    r = check_translated_design(out, CATAPULT)
    assert r["passed"], r["failures"]
    assert r["leftovers"] == []
    assert len(r["commented_leftovers"]) == 1
    assert r["commented_leftovers"][0]["line"] > 1


def test_check_flags_leftover_pragma_in_header(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "mac.h").write_text("#pragma HLS INLINE\n")
    r = check_translated_design(out, CATAPULT)
    assert not r["passed"]
    assert r["leftovers"][0]["file"] == "mac.h"


def test_check_flags_missing_driver_and_report(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "run.tcl").unlink()
    (out / "translation_report.md").unlink()
    r = check_translated_design(out, CATAPULT)
    assert not r["passed"]
    assert "missing required file run.tcl" in r["failures"]
    assert "missing required file translation_report.md" in r["failures"]


def test_check_flags_missing_top_marker(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "mac.cpp").write_text(GOOD_KERNEL.replace("#pragma hls_design top\n", ""))
    r = check_translated_design(out, CATAPULT)
    assert not r["passed"]
    assert r["checks"]["has_top_marker"] is False


def test_check_flags_incomplete_driver(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "run.tcl").write_text("go analyze\n")
    r = check_translated_design(out, CATAPULT)
    assert not r["passed"]
    assert r["checks"]["driver_complete"] is False


def test_check_missing_output_dir(tmp_path: Path):
    r = check_translated_design(tmp_path / "nothing", CATAPULT)
    assert r["passed"] is False if "passed" in r else True
    assert r["checks"]["output_exists"] is False
