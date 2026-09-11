from pathlib import Path

import pytest

from hlsfactory_agent.translate import (
    CATAPULT,
    OUTPUT_DIR_NAME,
    build_retry_prompt,
    build_translate_prompt,
    check_translated_design,
    compare_outputs,
    find_top_from_synth_tcl,
    get_target,
    render_catapult_run_tcl,
)

ROOT_FIXTURE = Path(__file__).resolve().parents[1] / "exp" / "run_translate" / "designs" / "vitis_mac"

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


def test_reconciliation_flags_pragma_kind_missing_from_report(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "translation_report.md").write_text("| mac.cpp | `#pragma HLS PIPELINE II=1` | moved |\n", encoding="utf-8")
    scan = {"counts": {"pragmas": {"pipeline": 1, "array_partition": 2}}}
    r = check_translated_design(out, CATAPULT, scan=scan)
    assert not r["passed"]
    assert r["unaccounted_pragma_kinds"] == ["array_partition"]
    assert r["checks"]["report_covers_all_pragma_kinds"] is False


def test_reconciliation_accepts_report_naming_every_kind(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "translation_report.md").write_text(
        "| mac.cpp | `#pragma HLS PIPELINE II=1` | moved |\n| mac.cpp | all 2 ARRAY_PARTITION pragmas | DROPPED |\n",
        encoding="utf-8",
    )
    scan = {"counts": {"pragmas": {"pipeline": 1, "array_partition": 2}}}
    r = check_translated_design(out, CATAPULT, scan=scan)
    assert r["passed"], r["failures"]
    assert r["unaccounted_pragma_kinds"] == []


def test_check_without_scan_is_unchanged(tmp_path: Path):
    out = make_good_design(tmp_path)
    r = check_translated_design(out, CATAPULT)
    assert r["passed"] and "report_covers_all_pragma_kinds" not in r["checks"]


def test_compare_outputs_identical_after_normalization():
    r = compare_outputs("a  b\n(-0.0000,0.0000)\nPASS \n", "a b\n(0.0000,-0.0000)\n\nPASS\n")
    assert r["identical"] and r["differing"] == 0


def test_compare_outputs_reports_differences():
    r = compare_outputs("x=1\nPASS\n", "x=2\nPASS\n")
    assert not r["identical"] and r["differing"] == 1 and r["sample"] == ["x=1 | x=2"]


def test_compare_outputs_counts_missing_lines():
    r = compare_outputs("a\nb\nc\n", "a\n")
    assert r["differing"] == 2 and r["original_lines"] == 3 and r["translated_lines"] == 1


def test_retry_prompt_carries_failures_and_tool_output():
    check = {
        "static": {"failures": ["missing required file run.tcl"], "unaccounted_pragma_kinds": ["dataflow"]},
        "container": {
            "syntax_check": {"a.cpp": {"exit_code": 1, "output": "a.cpp:3: error: unknown type"}},
            "testbench_build": {"exit_code": 1, "output": "ld: undefined reference"},
            "testbench_run": {"exit_code": None, "output": "not run: build failed"},
        },
    }
    p = build_retry_prompt("BASE", check)
    assert p.startswith("BASE")
    for needle in (
        "Previous attempt failed",
        "missing required file run.tcl",
        "dataflow",
        "unknown type",
        "undefined reference",
        "Do not start over",
    ):
        assert needle in p, needle
    assert "not run: build failed" not in p


def test_fixer_loop_retries_until_checks_pass(tmp_path: Path, monkeypatch):
    """Attempt 1 leaves the top marker out; the harness feeds the failure back; attempt 2 fixes it."""
    import hlsfactory_agent.translate as t

    class FakeContainer:
        def exec_run(self, *a, **k):
            return 0, b""

        def stop(self):
            pass

        def remove(self, force=False):
            pass

    class FakeClient:
        class containers:
            @staticmethod
            def run(**kwargs):
                return FakeContainer()

    prompts: list[str] = []

    def fake_agent(self, container, prompt_text, dir_run_area, run_data, attempt):
        prompts.append(prompt_text)
        out = dir_run_area / OUTPUT_DIR_NAME
        kernel = GOOD_KERNEL if attempt == 2 else GOOD_KERNEL.replace("#pragma hls_design top\n", "")
        (out / "mac.cpp").write_text(kernel, encoding="utf-8")
        (out / "testbench.cpp").write_text(GOOD_TB, encoding="utf-8")
        (out / "run.tcl").write_text(render_catapult_run_tcl("mac", ["mac.cpp"], "testbench.cpp"), encoding="utf-8")
        (out / "translation_report.md").write_text("pipeline interface array_partition\n", encoding="utf-8")
        run_data["sessions"].append({"attempt": attempt, "file": None, "exit_code": 0, "events": 0})
        run_data["session_data"] = None

    def fake_container_checks(container, target):
        return {
            "cpp_files": ["mac.cpp", "testbench.cpp"],
            "syntax_check": {},
            "syntax_all_ok": True,
            "testbench_build": {"exit_code": 0, "output": ""},
            "testbench_run": {"exit_code": 0, "output": "PASS"},
            "testbench_ok": True,
        }

    monkeypatch.setattr(t.docker, "from_env", lambda: FakeClient())
    monkeypatch.setattr(t.HLSTranslationRun, "_run_agent_once", fake_agent)
    monkeypatch.setattr(t, "run_container_checks", fake_container_checks)

    design = ROOT_FIXTURE
    run = t.HLSTranslationRun("loop-test", design, tmp_path / "work", "model", "key", target="catapult", prepass=False, attempts=3)
    check = run.run()
    assert check["passed"]
    assert check["attempts_used"] == 2
    assert [a["passed"] for a in check["attempts"]] == [False, True]
    assert "Previous attempt failed" in prompts[1] and "hls_design top" in prompts[1]
    assert "Previous attempt failed" not in prompts[0]


def test_xlscc_target_registered():
    from hlsfactory_agent.translate import XLSCC

    assert get_target("xlscc") is XLSCC
    assert XLSCC.top_marker == "#pragma hls_top"
    assert XLSCC.driver_file == "run_xlscc.sh"
    assert any("__xls_channel" in v for v in XLSCC.type_map.values())
    assert XLSCC.include_dir_names == ["ac_types_include", "xls_emu_include"]


def test_xlscc_driver_has_three_tools():
    from hlsfactory_agent.translate import render_xlscc_driver

    d = render_xlscc_driver("fft", ["fft.cpp"])
    for tok in ("xlscc fft.cpp --top fft", "opt_main", "codegen_main", "--pipeline_stages=1"):
        assert tok in d


def test_xlscc_prompt_mentions_channel_top_and_both_include_dirs():
    from hlsfactory_agent.translate import XLSCC

    p = build_translate_prompt("k", XLSCC)
    for needle in ("__xls_channel", "#pragma hls_top", "run_xlscc.sh", "-I/workspace/run_area/xls_emu_include", "-I/workspace/run_area/ac_types_include"):
        assert needle in p, needle


def test_check_missing_output_dir(tmp_path: Path):
    r = check_translated_design(tmp_path / "nothing", CATAPULT)
    assert r["passed"] is False if "passed" in r else True
    assert r["checks"]["output_exists"] is False
