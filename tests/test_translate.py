import json
import shutil
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
        "dims": meta.get("dims"),
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
        "    int grid[4][256];\n"
        "#pragma HLS array_partition variable=grid complete dim=1\n"
        "}\n",
        encoding="utf-8",
    )
    scan = scan_design(d, top="top")
    by_var = {r["args"]["variable"]: r for r in scan["resources"]}
    assert by_var["coef"]["scope"] == "argument" and by_var["coef"]["size"] == 8
    assert by_var["buf"]["scope"] == "local" and by_var["buf"]["size"] == 16
    assert by_var["grid"]["dims"] == [4, 256] and by_var["grid"]["size"] == 1024
    assert [(a["var"], a["size"], a["scope"]) for a in scan["arrays"]] == [("coef", 8, "argument"), ("buf", 16, "local"), ("grid", 1024, "local")]


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


def test_prepass_catapult_dialect(tmp_path: Path):
    d = tmp_path / "design"
    d.mkdir()
    (d / "k.h").write_text(
        "#include <complex>\n#include <hls_stream.h>\n#include \"hls_vector.h\"\ntypedef float dt;\ntypedef float w_dt;\n"
        "void top(hls::stream<std::complex<float>> &in, hls::stream<complex<float>> &out);\n",
        encoding="utf-8",
    )
    (d / "k.cpp").write_text(
        '#include "k.h"\n#define UF 2\n#define TYPE double\nstatic const float TW[4] = {1.0f, 0.0f, -1.0f, 0.0f};\nfloat C[2] = {1.0f, 0.5f};\nconstexpr double two_pi { 6.283185307 };\n'
        "void top(hls::stream<std::complex<float>> &in, hls::stream<complex<float>> &out) {\n"
        "    hls::stream<complex<float>> mid;\n    hls::stream<int, 4 * N> q;\n    hls::stream<int> named(\"named\");\n    hls::stream<int> bank[4];\n    uint16_t idx = 0;\n    double a = std::cos(1.0); double b = exp(a);\n"
        "    double ang = -2 * PI * idx / N;\n    float f = 2 * a;\n    r.range(i, i) = v.range(N-1-i, N-1-i);\n"
        "    L: for (int i = 0; i < 4; i++) {\n#pragma HLS PIPELINE II=1\n        out.write(in.read());\n    }\n"
        "    for (int j = 0; j < 4; j++) {\n#pragma HLS PIPELINE II=1\n#pragma HLS UNROLL\n        idx++;\n    }\n"
        "    for (int k = 0; k < 8; k++) {\n#pragma HLS UNROLL factor=UF\n        idx--;\n    }\n"
        "    int q = abs(idx); float z = 0; double s = sqrt(a); const w_dt vth = 1.0;\n#pragma SDS data zero_copy(idx)\n}\n"
        "void node1(\n  int v0[4],\n  int v2\n) {\t// L90\n#pragma HLS inline\n  v0[0] = v2;\n}\n",
        encoding="utf-8",
    )
    (d / "testbench.cpp").write_text('#include "k.h"\nint main() { std::complex<double> g(0, 0); return 0; }\n', encoding="utf-8")
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    h = (tmp_path / "out" / "k.h").read_text(encoding="utf-8")
    c = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "#include <ac_complex.h>" in h and "typedef ac_ieee_float32 dt;" in h and "<complex>" not in h
    assert "struct vector" in (tmp_path / "out" / "hls_vector.h").read_text(encoding="utf-8")
    assert c.startswith("#include <stdint.h>\n#include <ac_std_float.h>\n#include \"ac_float_trig.h\"\n")
    assert "ac_cos_cordic" in (tmp_path / "out" / "ac_float_trig.h").read_text(encoding="utf-8")
    assert "ac_channel<ac_complex<ac_ieee_float32>> &in" in c and "static ac_channel<ac_complex<ac_ieee_float32>> mid;" in c
    assert "static const float TW[4]" in c and "float C[2] = {1.0f, 0.5f};" in c  # literal tables stay native, const or not
    assert "constexpr double two_pi" in c  # ac_ieee_float is not a literal type
    (d / "old_test.cpp").write_text("int main() { return 0; }\n", encoding="utf-8")  # a second test program is not copied
    log2 = rewrite_design(d, tmp_path / "out2", CATAPULT, top="top")
    assert not (tmp_path / "out2" / "old_test.cpp").exists() and any(e.get("kind") == "file" for e in log2["pragmas_dropped"])
    assert "#define TYPE ac_ieee_float64" in c  # a type behind a macro is still a type
    assert "static ac_channel<int> q;" in c  # hls::stream<T, depth>: ac_channel has no depth argument
    assert "static ac_channel<int> named;" in c and "static ac_channel<int> bank[4];" in c  # name argument dropped, arrays too
    assert "ac_ieee_float64 a = cos(1.0); ac_ieee_float64 b = exp(a);" in c  # std::cos unqualified for the shim overload
    assert "ac_ieee_float64 ang = -ac_ieee_float64(2) * ac_ieee_float64(PI) * ac_ieee_float64(idx) / ac_ieee_float64(N);" in c
    assert "ac_ieee_float32 f = 2 * a;" in c and "r[i] = v[N-1-i];" in c
    # labelled loop: the pipeline pragma lands before `L: for`, not inside the body
    assert c.index("#pragma hls_pipeline_init_interval 1") < c.index("L: for")
    # a fully unrolled loop keeps its unroll and loses the pipeline pragma (Catapult refuses the pair)
    assert "#pragma hls_unroll yes\n    for (int j" in c and c.count("hls_pipeline_init_interval") == 1
    assert any("fully unrolled" in d.get("reason", "") for d in log["pragmas_dropped"])
    assert "#pragma hls_unroll 2\n    for (int k" in c and not any("integer literal" in r["reason"] for r in log["residue"])  # UF resolved from its #define
    assert not any(".range" in r["reason"] for r in log["residue"])  # the single-bit range was rewritten, not left as residue
    assert "int q = cat_abs(idx); ac_ieee_float32 z(0); ac_ieee_float64 s = sqrt(a); const w_dt vth(1.0);" in c
    assert "#pragma SDS" not in c and any(d.get("kind") == "sds" for d in log["pragmas_dropped"])
    assert "#pragma hls_design inline\nvoid node1(\n" in c  # multi-line signature with a trailing comment: pragma before its first line
    assert c.startswith("#include <stdint.h>\n#include <ac_std_float.h>\n#include \"ac_float_trig.h\"\n#include \"cat_abs.h\"\n")
    assert "ac_sqrt" in (tmp_path / "out" / "ac_float_trig.h").read_text(encoding="utf-8") and (tmp_path / "out" / "cat_abs.h").exists()
    exp_res = next(r for r in log["residue"] if "exp(" in r["reason"])
    assert "exp(" in c.splitlines()[exp_res["line"] - 1]  # residue lines point at the output file after insertions
    assert (tmp_path / "out" / "testbench.cpp").read_text(encoding="utf-8").count("std::complex<double>") == 1
    reasons = " ".join(r["reason"] for r in log["residue"])
    assert "float math in Siemens Catapult HLS: exp(" in reasons and "float math in Siemens Catapult HLS: cos(" not in reasons
    # the shim defines cos/sin and must not be asked to include itself
    static = check_translated_design(tmp_path / "out", CATAPULT)
    assert not [l for l in static["leftovers"] if l["file"] == "ac_float_trig.h"]
    assert "feeding a ac_ieee_float32" in reasons
    assert "testbench" in reasons


def test_prepass_sizes_unsized_top_parameters_from_the_testbench(tmp_path: Path):
    d = tmp_path / "design"
    d.mkdir()
    (d / "k.h").write_text("void top(const int in[], int out[], int n);\n", encoding="utf-8")
    (d / "k.cpp").write_text('#include "k.h"\nvoid top(const int in[/*n*/], int out[], int n) { for (int i = 0; i < n; i++) out[i] = in[i]; }\n', encoding="utf-8")
    (d / "testbench.cpp").write_text('#include "k.h"\n#define MAX 2000\nint main() { int a[MAX]; int len = 64; int* b = (int*)malloc(len * sizeof(int)); top(a, b, 5); return 0; }\n', encoding="utf-8")
    rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    assert "void top(const int in[2000], int out[64], int n)" in (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")  # static array, then malloc(len * sizeof)
    assert "int out[64]" in (tmp_path / "out" / "k.h").read_text(encoding="utf-8")


def test_compile_errors_in_the_prepass_output_become_residue():
    from hlsfactory_agent.translate import parse_compile_errors

    out = ("/workspace/run_area/output_design/dct.cpp:7:29: error: no viable conversion from 'float' to 'ac_ieee_float32'\n"
           "dct.cpp:7:40: error: no viable conversion from 'float' to 'ac_ieee_float32'\n"
           "/workspace/run_area/output_design/dct.cpp:10:12: error: no viable overloaded '='\n"
           "2 errors generated.\n")
    assert [(e["file"], e["line"]) for e in parse_compile_errors(out)] == [("dct.cpp", 7), ("dct.cpp", 10)]


def test_repair_top_uses_the_function_the_testbench_calls(tmp_path: Path):
    from hlsfactory_agent.translate import repair_top

    d = tmp_path / "aes_cipher"
    d.mkdir()
    (d / "aes_cipher.cpp").write_text("static void helper(int x) {}\nvoid aes_encrypt(const int in[16], int out[16]) { helper(1); }\n", encoding="utf-8")
    (d / "testbench.cpp").write_text("int main() { int a[16], b[16]; aes_encrypt(a, b); return 0; }\n", encoding="utf-8")
    (d / "synth.tcl").write_text("open_project p\nset_top aes_cipher\nadd_files aes_cipher.cpp\ncsynth_design\n", encoding="utf-8")
    assert repair_top(d) == {"top": "aes_encrypt", "repaired": True, "was": "aes_cipher"}
    assert "set_top aes_encrypt" in (d / "synth.tcl").read_text(encoding="utf-8")
    assert repair_top(d) == {"top": "aes_encrypt", "repaired": False}  # already right: untouched
    (d / "testbench.cpp").write_text("int main() { return 0; }\n", encoding="utf-8")  # stub testbench: name-prefix fallback
    (d / "synth.tcl").write_text("set_top aes\n", encoding="utf-8")
    assert repair_top(d)["top"] == "aes_encrypt"


def test_prepass_places_dataflow_blocks_on_channel_only_stages(tmp_path: Path):
    d = tmp_path / "design"
    d.mkdir()
    (d / "k.cpp").write_text(
        "#include <hls_stream.h>\n"
        "void produce(hls::stream<int> &o) { o.write(1); }\n"
        "template <int N> void consume(hls::stream<int> &i, int buf[4]) { buf[0] = i.read(); }\n"
        "void helper(hls::stream<int> &o) {\n#pragma HLS inline\n    o.write(2);\n}\n"
        "void top(int buf[4]) {\n#pragma HLS dataflow\n    hls::stream<int> s;\n    produce(s);\n    helper(s);\n    consume<4>(s, buf);\n}\n",
        encoding="utf-8",
    )
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    c = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "#pragma hls_design block\nvoid produce(" in c
    assert c.count("#pragma hls_design block") == 1  # consume takes an array, helper is inlined, top is the top
    assert [b["function"] for b in log["blocks"]] == ["produce"]


# --- Catapult directives (forms confirmed on Catapult 2026.2, 2026-09-16) ---------------


def test_render_directives_uses_the_confirmed_catapult_forms():
    rules = load_directive_rules("catapult")
    lines, left = render_directives(
        [
            _res("array_partition", variable="a", type="complete", _size=4, _scope="local"),
            _res("array_partition", variable="b", type="cyclic", _factor=8, _scope="local"),
            _res("array_partition", variable="c", type="block", _factor=4, _size=32, _scope="local"),
            _res("array_partition", variable="d", type="complete", _scope="argument"),
            # Catapult flattens row-major: splitting the outer dim is BLOCK_SIZE, the innermost is INTERLEAVE
            _res("array_partition", variable="e", type="complete", dim="1", _dims=[4, 256], _scope="local"),
            _res("array_partition", variable="f", type="cyclic", dim="2", _factor=2, _dims=[4, 256], _scope="local"),
            _res("bind_storage", variable="g", type="RAM_2P", impl="bram", _scope="local"),
            _res("stream", variable="h", depth="4", _scope="local"),
        ],
        "fft",
        rules,
        # Vitis gives these dual-port BRAM by default; Catapult's default has one read port
        arrays=[{"var": "big", "size": 64}, {"var": "small", "size": 16}, {"var": "rom", "size": 512, "const": True},
                {"var": "a", "size": 4}, {"var": "g", "size": 1024}, {"var": "unsized", "size": None}],
    )
    assert left == []
    assert lines == [
        "resource_directive a -MAP_TO_MODULE {[Register]}",
        "resource_directive b -INTERLEAVE 8",
        # Vitis factor counts partitions; Catapult BLOCK_SIZE counts elements per block.
        "resource_directive c -BLOCK_SIZE 8",
        # On an interface array -MAP_TO_MODULE {[Register]} is rejected with MEM-31.
        "resource_directive d -BLOCK_SIZE 1",
        "resource_directive e -BLOCK_SIZE 256",
        "resource_directive f -INTERLEAVE 2",
        "resource_directive g -MAP_TO_MODULE {ccs_sample_mem.ccs_ram_sync_dualport}",
        "channel_directive h -FIFO_DEPTH 4",
        "resource_directive big -MAP_TO_MODULE {ccs_sample_mem.ccs_ram_sync_dualport}",
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
            # 1024 registers is over Catapult's REGISTER_THRESHOLD (MEM-8); the outer dim of a 2-D array is not
            _res("array_partition", variable="e", type="complete", _size=1024, _scope="local"),
            _res("array_partition", variable="g", type="cyclic", dim="1", _factor=2, _dims=[4, 256], _scope="local"),
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
        "1024 elements exceed Catapult REGISTER_THRESHOLD 256",
        "cyclic partition of dim 1 of a 4x256 array has no flat equivalent",
    ]
    # an unresolved factor must still resolve to a template so the agent gets a hint line
    assert directive_template(unresolved, rules) is not None


def test_fixture_partition_reaches_the_catapult_script():
    resources = scan_design(FIXTURE, top="mac")["resources"]
    rendered, unrendered = render_directives(resources, "mac", CATAPULT.directive_rules)
    assert rendered == ["resource_directive coef -BLOCK_SIZE 1"]
    assert [u["resource"]["kind"] for u in unrendered] == ["interface", "interface"]


def test_run_tcl_looks_up_resource_paths_after_assembly():
    # predicted paths like /fft/a:rsc never exist: resources are nested per block and split per struct field
    line = "resource_directive a -INTERLEAVE 8"
    lines = render_catapult_run_tcl("fft", ["fft.cpp"], "testbench.cpp", directives=[line]).splitlines()
    proc = lines.index("proc directive_on {suffix var args} {")
    assert lines.index("go assembly") < proc < lines.index(line) < lines.index("go architect")
    assert "    set pat /fft" in lines
    # -DESIGN_HIERARCHY overrides every hls_design block with inline (CIN-393)
    assert not any("DESIGN_HIERARCHY" in l for l in lines)


# --- output checks ---------------------------------------------------------------------


def test_check_flags_leftover_vitis_dialect(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "mac.cpp").write_text(GOOD_KERNEL.replace("ac_fixed<16, 8, true>", "ap_fixed<16, 8>"), encoding="utf-8")
    r = check_translated_design(out, CATAPULT)
    assert not r["passed"] and r["leftovers"][0]["file"] == "mac.cpp"
    # DUT-only: float math and non-static local channels fail in the kernel, not in the testbench
    (out / "mac.cpp").write_text(GOOD_KERNEL.replace("    for", "    ac_channel<data_t> mid;\n    double a = cosf(1.0);\n    std::complex<float> z; ac_complex<float> ok; uint16_t k;\n    for"), encoding="utf-8")
    (out / "testbench.cpp").write_text('#include "ac_fixed.h"\n#include <cmath>\nint main() { return cos(0.0) > 2; }\n', encoding="utf-8")
    r = check_translated_design(out, CATAPULT)
    # a used uint16_t without <stdint.h> is reported at line 1 (the pre-pass inserts it; the agent must not remove it);
    # cosf is fine once ac_float_trig.h is included, so line 8 is not a leftover but the missing include is
    assert [(l["file"], l["line"]) for l in r["leftovers"]] == [("mac.cpp", 1), ("mac.cpp", 1), ("mac.cpp", 7), ("mac.cpp", 9)]


def test_compare_outputs_tolerates_low_bit_float_differences():
    from hlsfactory_agent.translate import compare_outputs

    r = compare_outputs("v (0.0655044,8.15184)\nPASS\n", "v (0.0655021,8.15185)\nPASS\n")
    assert r["identical"] and r["close_within_tolerance"] == 1 and 0 < r["max_relative_difference"] < 1e-4
    assert not compare_outputs("v 1.0000\n", "v 1.0100\n")["identical"]


def test_check_flags_directives_missing_or_unresolved_in_the_script(tmp_path: Path):
    out = make_good_design(tmp_path)
    line = "resource_directive coef -MAP_TO_MODULE {[Register]}"
    (out / "directives.json").write_text(json.dumps({"rendered": [line], "unrendered": []}), encoding="utf-8")
    r = check_translated_design(out, CATAPULT)
    assert r["checks"]["directives_in_script"] is False

    (out / "run.tcl").write_text(
        render_catapult_run_tcl("mac", ["mac.cpp"], "testbench.cpp", directives=[line]), encoding="utf-8"
    )
    assert check_translated_design(out, CATAPULT)["checks"]["directives_in_script"] is True


def test_check_fences_agent_edits_to_the_residue(tmp_path: Path):
    out = make_good_design(tmp_path)
    kernel = GOOD_KERNEL.replace("typedef", "// pad\n" * 20 + "L_pad:\ntypedef")  # the loop sits ~28 lines below the includes
    (out / "mac.cpp").write_text(kernel, encoding="utf-8")
    pre = tmp_path / "prepass"
    shutil.copytree(out, pre)
    (pre / "rewrite_log.json").write_text(json.dumps({"residue": [{"file": "mac.cpp", "line": 28, "reason": "x"}]}), encoding="utf-8")
    # an insertion anywhere, an edit near the residue line, and deleting comment lines are fine
    # (and so is deleting a bare loop label)
    (out / "mac.cpp").write_text(kernel.replace("typedef", "struct v { int d[2]; };\ntypedef").replace("out.write(in.read());", "out.write(in.read()); // residue fix").replace("// pad\n" * 3, "").replace("L_pad:\n", ""), encoding="utf-8")
    assert check_translated_design(out, CATAPULT, dir_prepass=pre)["checks"]["edits_within_residue"] is True
    # deleting the include the pre-pass added is not
    (out / "mac.cpp").write_text(kernel.replace('#include "ac_channel.h"\n', ""), encoding="utf-8")
    r = check_translated_design(out, CATAPULT, dir_prepass=pre)
    assert r["checks"]["edits_within_residue"] is False and r["fenced_edits"][0]["prepass"] == '#include "ac_channel.h"'


def test_check_flags_pragma_kind_missing_from_the_report(tmp_path: Path):
    out = make_good_design(tmp_path)
    (out / "translation_report.md").write_text("only pipeline is mentioned\n", encoding="utf-8")
    r = check_translated_design(out, CATAPULT, scan={"counts": {"pragmas": {"pipeline": 1, "dataflow": 2}}})
    assert not r["passed"] and r["unaccounted_pragma_kinds"] == ["dataflow"]


def test_parse_vitis_report_and_synth_cache(tmp_path, monkeypatch):
    from hlsfactory_agent import synth as synth_mod
    from hlsfactory_agent.spec import SynthResult
    from hlsfactory_agent.targets.vitis import VITIS, parse_vitis_report

    rpt = tmp_path / "prj" / "solution" / "syn" / "report" / "csynth.rpt"
    rpt.parent.mkdir(parents=True)
    rpt.write_text(
        "|+ FFT_TOP* | -| 0.07| 5879| 2.352e+04| -| 512| -| dataflow| 96 (5%)| 96 (3%)| 43381 (7%)| 40743 (14%)| -|\n",
        encoding="utf-8",
    )
    r = parse_vitis_report(tmp_path)
    assert (r.status, r.latency, r.throughput, r.resources) == ("PASS", 5879, 512, {"BRAM": 96, "DSP": 96, "FF": 43381, "LUT": 40743})

    design = tmp_path / "design"
    design.mkdir()
    (design / "k.cpp").write_text("int x;\n", encoding="utf-8")
    calls = []
    monkeypatch.setattr(synth_mod, "run_synth", lambda target, d: calls.append(d) or SynthResult("PASS", latency=1))
    a = synth_mod.run_synth_cached(VITIS, design, tmp_path / "work", tmp_path / "cache")
    b = synth_mod.run_synth_cached(VITIS, design, tmp_path / "work", tmp_path / "cache")
    assert a.latency == b.latency == 1 and len(calls) == 1  # second call served from the cache
    (design / "k.cpp").write_text("int y;\n", encoding="utf-8")
    synth_mod.run_synth_cached(VITIS, design, tmp_path / "work", tmp_path / "cache")
    assert len(calls) == 2  # content changed, so it ran again


def test_catapult_degrade_step_reads_the_number_from_the_log(tmp_path: Path):
    from hlsfactory_agent.targets.catapult import catapult_degrade_step

    failed = tmp_path / "failed"
    failed.mkdir()
    (failed / "k.c").write_text("#pragma hls_pipeline_init_interval 1\nfor(;;){}\n#pragma hls_pipeline_init_interval 8\n", encoding="utf-8")  # a .c kernel too
    (failed / "run.tcl").write_text("go assembly\ngo architect\n", encoding="utf-8")
    (failed / "catapult.log").write_text("#  Minimum II of '4' is required to schedule the design (SCHD-87)\n", encoding="utf-8")
    assert catapult_degrade_step(failed, tmp_path / "d1") == "relax_ii:4"
    assert (tmp_path / "d1" / "k.c").read_text(encoding="utf-8").count("init_interval 4") == 1  # the 8 stays
    (failed / "catapult.log").write_text(
        "# Error: insufficient resources 'x' to schedule '/T'. 4 are needed, but only 2 instances are available (SCHD-4)\n"
        "# Error: please merge accesses to 'stage:data_rev._r.d:rsc' to reduce the number of required resources (SCHD-39)\n",
        encoding="utf-8",
    )
    assert catapult_degrade_step(failed, tmp_path / "d2") == "interleave:data_rev:2"
    assert "resource_directive data_rev -INTERLEAVE 2\ngo architect" in (tmp_path / "d2" / "run.tcl").read_text(encoding="utf-8")
    (tmp_path / "d2" / "catapult.log").write_text((failed / "catapult.log").read_text(encoding="utf-8"), encoding="utf-8")  # same verdict again
    assert catapult_degrade_step(tmp_path / "d2", tmp_path / "d2b") == "interleave:data_rev:4"  # a repeat round compounds
    assert (tmp_path / "d2b" / "run.tcl").read_text(encoding="utf-8").count("INTERLEAVE") == 1
    (failed / "k.cpp").write_text("static int data_rev[4][256];\n", encoding="utf-8")  # rows in parallel: one bank per row
    assert catapult_degrade_step(failed, tmp_path / "d2c") == "block:data_rev:256"
    (failed / "catapult.log").write_text("# Error: something else\n", encoding="utf-8")
    assert catapult_degrade_step(failed, tmp_path / "d3") is None
    (failed / "k.cpp").write_text("    L:\n    #pragma hls_pipeline_init_interval 1\n    #pragma hls_unroll 4\n    for(;;){}\n", encoding="utf-8")
    (failed / "catapult.log").write_text("# Error: $PROJECT_HOME/k.cpp(4): Cannot pipeline a fully unrolled loop. Attempting (SCHD-x)\n", encoding="utf-8")
    assert catapult_degrade_step(failed, tmp_path / "d4") == "unpipeline:1"
    assert "init_interval" not in (tmp_path / "d4" / "k.cpp").read_text(encoding="utf-8")


def test_tool_installations_resolve_to_read_only_mounts(tmp_path: Path):
    from hlsfactory_agent.tools import catapult_installation, vitis_installation

    assert catapult_installation(tmp_path / "nowhere") is None and vitis_installation(tmp_path / "nowhere") is None
    (tmp_path / "Mgc_home" / "bin").mkdir(parents=True)
    (tmp_path / "Mgc_home" / "bin" / "catapult").touch()
    cat = catapult_installation(tmp_path / "Mgc_home")
    assert cat.volumes[str(tmp_path / "Mgc_home")] == {"bind": "/opt/siemens/catapult", "mode": "ro"}
    assert cat.environment["MGC_HOME"] == "/opt/siemens/catapult" and "@" in cat.environment["SALT_LICENSE_FILE"]
    hls = tmp_path / "Vitis_HLS" / "2024.1"
    (hls / "bin").mkdir(parents=True)
    (hls / "bin" / "vitis_hls").touch()
    assert vitis_installation(hls) is None  # csynth needs the matching Vivado part database
    vivado = tmp_path / "Vivado" / "2024.1"
    (vivado / "bin").mkdir(parents=True)
    (vivado / "bin" / "vivado").touch()
    (vivado / "data" / "parts").mkdir(parents=True)
    vit = vitis_installation(hls)
    assert {v["bind"] for v in vit.volumes.values()} == {"/opt/amd/hls", "/opt/amd/vivado"} and all(v["mode"] == "ro" for v in vit.volumes.values())
    assert vit.environment["XILINX_VIVADO"] == "/opt/amd/vivado" and vit.environment["PATH"].startswith("/opt/amd/hls/bin")


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
    (tmp_path / "catapult.log").write_text("# DIRECTIVE coef hits=2\n# DIRECTIVE tw hits=0\n", encoding="utf-8")
    r = parse_catapult_report(tmp_path)
    assert (r.status, r.latency, r.throughput, r.area) == ("PASS", 8, 10, 2220.6)
    assert r.directives == {"coef": 2, "tw": 0}

