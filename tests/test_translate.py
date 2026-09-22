import json
from pathlib import Path

from hlsfactory_agent.rewrite import rewrite_design
from hlsfactory_agent.scan import resolve_int, scan_design
from hlsfactory_agent.translate import (
    CATAPULT,
    OUTPUT_DIR_NAME,
    check_translated_design,
    directive_template,
    load_directive_rules,
    render_catapult_run_tcl,
    render_directives,
)

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = ROOT / "exp" / "run_translate" / "designs" / "vitis_mac"
EXPECTED = ROOT / "exp" / "run_translate" / "expected" / "catapult_mac"

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


def make_good_design(root: Path) -> Path:
    out = root / OUTPUT_DIR_NAME
    out.mkdir()
    (out / "mac.cpp").write_text(GOOD_KERNEL, encoding="utf-8")
    (out / "testbench.cpp").write_text('#include "ac_fixed.h"\nint main() { return 0; }\n', encoding="utf-8")
    (out / "run.tcl").write_text(render_catapult_run_tcl("mac", ["mac.cpp"], "testbench.cpp"), encoding="utf-8")
    (out / "translation_report.md").write_text("# report\n", encoding="utf-8")
    return out


def _res(kind, **args):
    meta = {k[1:]: args.pop(k) for k in list(args) if k.startswith("_")}
    return {
        "kind": kind,
        "args": args,
        "factor": meta.get("factor"),
        "unresolved": meta.get("unresolved"),
        "scope": meta.get("scope", "unknown"),
        "size": meta.get("size"),
    }


def _norm(p: Path) -> str:
    return "\n".join(" ".join(ln.split()) for ln in p.read_text(encoding="utf-8").splitlines() if ln.strip())


# --- scanner ---------------------------------------------------------------------------


def test_resolve_int_substitutes_defines_and_rejects_non_arithmetic():
    d = {"UF": "4", "N": "UF*2", "HALF": "(N/2)"}
    assert resolve_int("UF*2", d) == 8
    assert resolve_int("HALF+1", d) == 5
    assert resolve_int("UNKNOWN*2", d) is None
    assert resolve_int("__import__('os')", d) is None


def test_scan_collects_resources_with_resolved_factors(tmp_path: Path):
    d = tmp_path / "design"
    d.mkdir()
    (d / "k.h").write_text("#define UF 4\n", encoding="utf-8")
    (d / "k.cpp").write_text(
        '#include "k.h"\n'
        "void top(int a[16]) {\n"
        "#pragma HLS array_partition variable=a type=cyclic factor=UF*2 dim=1\n"
        "#pragma HLS array_partition variable=b type=block factor=MISSING dim=1\n"
        "#pragma HLS ARRAY_PARTITION variable=c complete dim=1\n"
        "}\n",
        encoding="utf-8",
    )
    by_var = {r["args"]["variable"]: r for r in scan_design(d)["resources"]}
    assert by_var["a"]["factor"] == 8 and by_var["a"]["unresolved"] is None
    assert by_var["b"]["factor"] is None and by_var["b"]["unresolved"] == "MISSING"
    assert by_var["c"]["args"]["type"] == "complete"


def test_scan_marks_argument_vs_local_arrays_and_sizes(tmp_path: Path):
    d = tmp_path / "design"
    d.mkdir()
    (d / "k.cpp").write_text(
        "void top(int coef[8]) {\n"
        "#pragma HLS array_partition variable=coef complete dim=1\n"
        "    int buf[16];\n"
        "#pragma HLS array_partition variable=buf type=cyclic factor=4 dim=1\n"
        "}\n",
        encoding="utf-8",
    )
    by_var = {r["args"]["variable"]: r for r in scan_design(d, top="top")["resources"]}
    assert by_var["coef"]["scope"] == "argument" and by_var["coef"]["size"] == 8
    assert by_var["buf"]["scope"] == "local" and by_var["buf"]["size"] == 16


# --- mechanical pre-pass ---------------------------------------------------------------


def test_prepass_reproduces_the_hand_translation_of_the_fixture(tmp_path: Path):
    log = rewrite_design(FIXTURE, tmp_path / "out", CATAPULT, top="mac")
    for name in ("mac.h", "mac.cpp", "testbench.cpp"):
        assert _norm(tmp_path / "out" / name) == _norm(EXPECTED / name), name
    assert log["residue"] == []
    assert {d["kind"] for d in log["pragmas_dropped"]} == {"interface", "array_partition"}


def test_prepass_maps_every_mode_ac_fixed_supports(tmp_path: Path):
    d = tmp_path / "in"
    d.mkdir()
    (d / "k.cpp").write_text(
        '#include "ap_fixed.h"\n'
        "typedef ap_fixed<8, 4, AP_RND_ZERO, AP_SAT_SYM> a_t;\n"
        "typedef ap_fixed<8, 4, AP_TRN_ZERO, AP_SAT_ZERO> b_t;\n"
        "typedef ap_fixed<8, 4, AP_RND_CONV, AP_WRAP_SM> c_t;\n"
        "void top(a_t x) {}\n",
        encoding="utf-8",
    )
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    text = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "ac_fixed<8, 4, true, AC_RND_ZERO, AC_SAT_SYM> a_t" in text
    assert "ac_fixed<8, 4, true, AC_TRN_ZERO, AC_SAT_ZERO> b_t" in text
    # AP_WRAP_SM has no ac_fixed counterpart; it must be reported, never approximated.
    assert any("AP_WRAP_SM" in r["reason"] for r in log["residue"])


# --- Catapult directives (forms confirmed on Catapult 2026.2, 2026-09-16) ---------------


def test_render_directives_uses_the_confirmed_catapult_forms():
    rules = load_directive_rules("catapult")
    lines, left = render_directives(
        [
            _res("array_partition", variable="a", type="complete", _scope="local"),
            _res("array_partition", variable="b", type="cyclic", _factor=8, _scope="local"),
            _res("array_partition", variable="c", type="block", _factor=4, _size=32, _scope="local"),
            _res("array_partition", variable="d", type="complete", _scope="argument"),
        ],
        "fft",
        rules,
    )
    assert left == []
    assert lines == [
        "directive set /fft/a:rsc -MAP_TO_MODULE {[Register]}",
        "directive set /fft/b:rsc -INTERLEAVE 8",
        # Vitis factor counts partitions; Catapult BLOCK_SIZE counts elements per block.
        "directive set /fft/c:rsc -BLOCK_SIZE 8",
        # On an interface array -MAP_TO_MODULE {[Register]} is rejected with MEM-31.
        "directive set /fft/d:rsc -BLOCK_SIZE 1",
    ]


def test_render_directives_omits_forms_catapult_ignores_or_breaks_on():
    rules = load_directive_rules("catapult")
    unresolved = _res("array_partition", variable="d", type="cyclic", _unresolved="UF*2", _scope="local")
    lines, left = render_directives(
        [
            # -INTERLEAVE on an interface array is accepted and silently ignored.
            _res("array_partition", variable="a", type="cyclic", _factor=2, _scope="argument"),
            # mapping an array to a RAM can fail scheduling, so no rule is provided.
            _res("bind_storage", variable="b", impl="bram", _scope="local"),
            _res("array_partition", variable="c", type="block", _factor=2, _scope="local"),
            unresolved,
        ],
        "fft",
        rules,
    )
    assert lines == []
    assert [e["reason"] for e in left] == [
        "no directive rule",
        "no directive rule",
        "block partition needs both array size and factor",
        "unresolved factor UF*2",
    ]
    # an unresolved factor must still resolve to a template so the agent gets a hint line
    assert directive_template(unresolved, rules) is not None


def test_fixture_partition_reaches_the_catapult_script():
    resources = scan_design(FIXTURE, top="mac")["resources"]
    rendered, unrendered = render_directives(resources, "mac", CATAPULT.directive_rules)
    assert rendered == ["directive set /mac/coef:rsc -BLOCK_SIZE 1"]
    assert [u["resource"]["kind"] for u in unrendered] == ["interface", "interface"]


def test_run_tcl_places_directives_between_libraries_and_assembly():
    line = "directive set /fft/a:rsc -INTERLEAVE 8"
    lines = render_catapult_run_tcl("fft", ["fft.cpp"], "testbench.cpp", directives=[line]).splitlines()
    assert lines.index("go libraries") < lines.index(line) < lines.index("go assembly")


# --- output checks ---------------------------------------------------------------------


def test_check_flags_leftover_vitis_dialect(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "mac.cpp").write_text(GOOD_KERNEL.replace("ac_fixed<16, 8, true>", "ap_fixed<16, 8>"), encoding="utf-8")
    r = check_translated_design(out, CATAPULT)
    assert not r["passed"] and r["leftovers"][0]["file"] == "mac.cpp"


def test_check_flags_directives_missing_or_unresolved_in_the_script(tmp_path: Path):
    out = make_good_design(tmp_path)
    line = "directive set /mac/coef:rsc -MAP_TO_MODULE {[Register]}"
    (out / "directives.json").write_text(json.dumps({"rendered": [line], "unrendered": []}), encoding="utf-8")
    r = check_translated_design(out, CATAPULT)
    assert r["checks"]["directives_in_script"] is False

    (out / "run.tcl").write_text(
        render_catapult_run_tcl("mac", ["mac.cpp"], "testbench.cpp", directives=[line]), encoding="utf-8"
    )
    assert check_translated_design(out, CATAPULT)["checks"]["directives_in_script"] is True


def test_check_flags_pragma_kind_missing_from_the_report(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "translation_report.md").write_text("only pipeline is mentioned\n", encoding="utf-8")
    r = check_translated_design(out, CATAPULT, scan={"counts": {"pragmas": {"pipeline": 1, "dataflow": 2}}})
    assert not r["passed"] and r["unaccounted_pragma_kinds"] == ["dataflow"]


def test_parse_catapult_report(tmp_path):
    from hlsfactory_agent.targets.catapult import parse_catapult_report

    (tmp_path / "catapult.log").write_text(
        "# Error: $PROJECT_HOME/FFT.cpp(1): Logic mixed with interconnect in hierarchical function '/top/f'\n"
        "# SYNTH_ERROR: go schedule: Failed schedule\n",
        encoding="utf-8",
    )
    r = parse_catapult_report(tmp_path)
    assert (r.status, r.failure_class) == ("FAILED", "logic-in-interconnect")
    rpt = tmp_path / "Catapult" / "mac.v1" / "rtl.rpt"
    rpt.parent.mkdir(parents=True)
    rpt.write_text("  Design Total:   8   8   10   0  0\n  Total Area Score:   1940.6   2261.2   2220.6\n", encoding="utf-8")
    (tmp_path / "catapult.log").write_text("# done\n", encoding="utf-8")
    r = parse_catapult_report(tmp_path)
    assert (r.status, r.latency, r.throughput, r.area) == ("PASS", 8, 10, 2220.6)
