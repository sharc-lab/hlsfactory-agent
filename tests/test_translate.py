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
    assert c.startswith("#include <stdint.h>\n#include <ac_std_float.h>\n#include \"ac_float_trig.h\"\n#include \"ac_float_exp.h\"\n#include \"cat_abs.h\"\n")
    assert "ac_sqrt" in (tmp_path / "out" / "ac_float_trig.h").read_text(encoding="utf-8") and (tmp_path / "out" / "cat_abs.h").exists()
    assert "ac_exp_cordic" in (tmp_path / "out" / "ac_float_exp.h").read_text(encoding="utf-8")
    float_res = next(r for r in log["residue"] if "feeding a ac_ieee_float32" in r["reason"])
    assert "ac_ieee_float32" in c.splitlines()[float_res["line"] - 1]  # residue lines point at the output file after insertions
    assert (tmp_path / "out" / "testbench.cpp").read_text(encoding="utf-8").count("std::complex<double>") == 1
    reasons = " ".join(r["reason"] for r in log["residue"])
    assert "float math in Siemens Catapult HLS" not in reasons  # cos and exp both have shims
    # the shims define cos/sin/exp and must not be asked to include themselves
    static = check_translated_design(tmp_path / "out", CATAPULT)
    assert not [l for l in static["leftovers"] if l["file"] in ("ac_float_trig.h", "ac_float_exp.h")]
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
    # a pointer port sized from `new T[n]` or `malloc(sizeof(T) * N)`, in the definition only
    (d / "k.cpp").write_text('void top(int *keys,  // in\n         int *out, int n) { for (int i = 0; i < n; i++) out[i] = keys[i]; }\n', encoding="utf-8")
    (d / "testbench.cpp").write_text('#define N 10*10\nint main() { int num = 1000; int *keys = new int[num]; int *b; b = (int*)malloc(sizeof(int)*N); top(keys, b, 5); return 0; }\n', encoding="utf-8")
    shutil.rmtree(tmp_path / "out")
    rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    assert "void top(int keys[1000],  // in\n         int out[100], int n)" in (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")


def test_prepass_and_check_mechanical_leftovers(tmp_path: Path):
    from hlsfactory_agent.translate import insert_required_includes

    d = tmp_path / "design"
    d.mkdir()
    (d / "ap_fixed.h").write_text("template <int _AP_W, int _AP_I> struct ap_fixed {};\n", encoding="utf-8")  # a copied Vitis library header
    (d / "k.h").write_text('#include "hls_math.h"\n#include "hls_stream.h"\ntemplate <typename T> struct Pipe {\n    hls::stream<T> ch;\n};\n#define DEF_Q( z ) hls::stream<int> q_##z; \\\n    int unused_##z;\n', encoding="utf-8")
    (d / "k.cpp").write_text('#include "k.h"\nvoid top(hls::stream<int> &o) {\n    hls::stream<int> q;\n    q.write(1); o.write(q.read());\n}\n', encoding="utf-8")
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    out = tmp_path / "out"
    h, c = (out / "k.h").read_text(encoding="utf-8"), (out / "k.cpp").read_text(encoding="utf-8")
    assert not (out / "ap_fixed.h").exists() and "#include <cmath>" in h and "hls_math" not in h
    assert "    ac_channel<T> ch;" in h and "    static ac_channel<int> q;" in c  # a member stays, a local becomes static
    assert "#define DEF_Q( z ) static ac_channel<int> q_##z; \\" in h  # a channel declared by a macro too
    assert not any("ac_channel" in r["reason"] for r in log["residue"])
    # a local declared through a channel alias is static too; a member under #ifdef/#else is not
    (d / "a.h").write_text('#include "hls_stream.h"\n#define stream_t(x) hls::stream<x>\ntemplate <typename T> class S {\n#ifdef SYN\n  void w(T v) {\n#else\n  void w(T v, int n) {\n#endif\n    s_.write(v);\n  }\n#ifndef SYN\n  int n_;\n#else\n  hls::stream<T> s_;\n#endif\n};\n', encoding="utf-8")
    (d / "a.cpp").write_text('#include "a.h"\nvoid f(stream_t(int) &o) {\n    stream_t(int) buf[2];\n    buf[0].write(1); o.write(buf[0].read());\n}\n', encoding="utf-8")
    shutil.rmtree(out)
    rewrite_design(d, out, CATAPULT, top="top")
    assert "    static stream_t(int) buf[2];" in (out / "a.cpp").read_text(encoding="utf-8")
    assert "\n  ac_channel<T> s_;" in (out / "a.h").read_text(encoding="utf-8")
    for f in ("a.h", "a.cpp"):
        (d / f).unlink()
    # byte-level idioms become the typed cat_bytes.h helpers; a void* parameter becomes a template parameter
    (d / "pk.h").write_text("#include <cstring>\ntemplate <typename T, int N> struct pk {\n    T data[N];\n"
                            "    pk(ap_uint<32 * N> u) { *reinterpret_cast<ap_uint<32 * N>*>(data) = u; }\n"
                            "    inline void copy_from( const void* src ) { memcpy( data, src, sizeof(data) ); }\n"
                            "    inline void set_zero() { memset( data, 0, sizeof(data) ); }\n};\n", encoding="utf-8")
    (d / "k.cpp").write_text('#include "k.h"\n#include "pk.h"\n' + (d / "k.cpp").read_text(encoding="utf-8").split("\n", 1)[1], encoding="utf-8")
    shutil.rmtree(out)
    rewrite_design(d, out, CATAPULT, top="top")
    pk = (out / "pk.h").read_text(encoding="utf-8")
    assert pk.startswith('#include "cat_bytes.h"') and (out / "cat_bytes.h").exists()
    assert "cat_store<ac_int<32 * N, false>>(data, u);" in pk and "template <class P0> inline void copy_from( const P0 * src )" in pk
    assert "cat_memcpy( data, src, sizeof(data) );" in pk and "cat_memset0(data, sizeof(data) );" in pk
    # a header the agent writes with float types gets its include; native float in the DUT is flagged
    (out / "helper.h").write_text("inline ac_ieee_float32 twice(ac_ieee_float32 x) { return x + x; } // exp(x) in a comment\n", encoding="utf-8")
    (out / "k.cpp").write_text(c + "float f(ac_ieee_float32 a) { return a.to_float() * 2; }\n", encoding="utf-8")
    assert insert_required_includes(out, CATAPULT) == ["helper.h: #include <ac_std_float.h>", "k.cpp: #include <ac_std_float.h>"]
    static = check_translated_design(out, CATAPULT)
    assert [l["text"] for l in static["leftovers"]] == ["float f(ac_ieee_float32 a) { return a.to_float() * 2; }"]
    (out / "k.cpp").write_text("#include <ac_std_float.h>\n" + c + "void g(ac_ieee_float32 &a, int w) { a += float(w * 2); }\n", encoding="utf-8")
    assert [l["text"] for l in check_translated_design(out, CATAPULT)["leftovers"]] == ["void g(ac_ieee_float32 &a, int w) { a += float(w * 2); }"]
    # a double literal as a ternary branch of a float-typed declaration takes the float type (CRD-42)
    shutil.rmtree(out)
    (d / "t.h").write_text("using fp_t = float;\ninline fp_t pick(int i, fp_t x) {\n    fp_t t = i ? i * x : 0.0;\n    return t;\n}\n", encoding="utf-8")
    rewrite_design(d, out, CATAPULT, top="top")
    assert "    fp_t t = i ? i * x : fp_t(0.0);" in (out / "t.h").read_text(encoding="utf-8")


def test_prepass_names_anonymous_namespaces_in_headers_that_hold_only_types(tmp_path: Path):
    d = tmp_path / "design"
    d.mkdir()
    (d / "flat.h").write_text("namespace lib {\nnamespace {\ntemplate <int N>\nstruct Impl {\n  int v{N};\n};\n} // anonymous\ntemplate <int N> class Flat { Impl<N> impl_{}; };\n}\n", encoding="utf-8")
    (d / "vars.h").write_text("namespace {\n  int acc = 0;\n}\n", encoding="utf-8")  # one object per unit: naming it would define it twice
    (d / "k.cpp").write_text('#include "flat.h"\n#include "vars.h"\nvoid top(int &o) { o = acc; }\n', encoding="utf-8")
    rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    assert "\ninline namespace anon_flat {\n" in (tmp_path / "out" / "flat.h").read_text(encoding="utf-8")
    assert (tmp_path / "out" / "vars.h").read_text(encoding="utf-8").startswith("namespace {\n")


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



def test_normalize_vitis_script_sets_variables_include_paths_and_drops_removed_options(tmp_path: Path):
    from hlsfactory_agent.translate import normalize_vitis_script

    d = tmp_path / "accum"
    (d / "lib").mkdir(parents=True)
    (d / "lib" / "common.h").write_text("#pragma once\n", encoding="utf-8")
    (d / "accum.cpp").write_text('#include "common.h"\nvoid accum() {}\n', encoding="utf-8")
    (d / "synth.tcl").write_text("open_project -reset ${project_name}_prj\nset_top accum\nadd_files accum.cpp\nadd_files -tb tb.cpp\n"
                                 "config_interface -trim_dangling_width 0\ncsynth_design\n", encoding="utf-8")
    assert normalize_vitis_script(d) == {"variables_set": ["project_name"], "cflags": "-I[pwd]/lib",
                                         "options_dropped": ["config_interface -trim_dangling_width 0"]}
    tcl = (d / "synth.tcl").read_text(encoding="utf-8")
    assert tcl.startswith("set project_name accum\n") and 'add_files accum.cpp -cflags "-I[pwd]/lib"' in tcl and "add_files -tb tb.cpp\n" in tcl
    assert normalize_vitis_script(d) == {}  # idempotent


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



def test_prepass_finds_dataflow_behind_a_macro_and_stages_in_other_headers(tmp_path: Path):
    d = tmp_path / "design"
    d.mkdir()
    (d / "defs.h").write_text("#define PRAGMA(x) _Pragma(#x)\n#define DF_REGION PRAGMA( hls dataflow )\n", encoding="utf-8")
    (d / "stages.h").write_text(
        "#include <hls_stream.h>\nconstexpr int ceil_div(int a, int b) { return (a + b - 1) / b; }\n"
        "template <int N> void produce(hls::stream<int> &o) { o.write(N); }\n",
        encoding="utf-8",
    )
    (d / "k.cpp").write_text(
        '#include "defs.h"\n#include "stages.h"\n'
        "void top(hls::stream<int> &o) {\n    for (int i = 0; i < 2; ++i)\n    DF_REGION\n    {\n        produce<ceil_div(4, 2)>(o);\n    }\n}\n",
        encoding="utf-8",
    )
    log = rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    assert [b["function"] for b in log["blocks"]] == ["produce"]  # ceil_div returns a value
    assert "#pragma hls_design block\ntemplate <int N> void produce(" in (tmp_path / "out" / "stages.h").read_text(encoding="utf-8")

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
    # fence repair puts the fenced line back and keeps the licensed edit
    from hlsfactory_agent.translate import restore_fenced

    (out / "mac.cpp").write_text(kernel.replace('#include "ac_channel.h"\n', "").replace("out.write(in.read());", "out.write(in.read()); // fix"), encoding="utf-8")
    assert list(restore_fenced(out, pre, CATAPULT)) == ["mac.cpp"]
    fixed = (out / "mac.cpp").read_text(encoding="utf-8")
    assert '#include "ac_channel.h"' in fixed and "// fix" in fixed
    assert check_translated_design(out, CATAPULT, dir_prepass=pre)["checks"]["edits_within_residue"] is True
    # a header only the testbench includes is testbench code: not fenced
    from hlsfactory_agent.translate import testbench_files

    for d in (pre, out):
        (d / "tb_util.h").write_text("inline double gold(double x) { return x; }\n", encoding="utf-8")
        (d / "testbench.cpp").write_text('#include "tb_util.h"\n#include "mac.cpp"\nint main() { return 0; }\n', encoding="utf-8")
    assert testbench_files(out) == {"testbench.cpp", "tb_util.h"}
    (out / "tb_util.h").write_text("inline double gold(double x) { return x * 1.0; }\n", encoding="utf-8")
    assert check_translated_design(out, CATAPULT, dir_prepass=pre)["checks"]["edits_within_residue"] is True
    # a file in a folder the agent deleted entirely is restored, folder included
    (pre / "etc").mkdir()
    (pre / "etc" / "lib.h").write_text("#define LIB 1\n", encoding="utf-8")
    assert list(restore_fenced(out, pre, CATAPULT)) == ["etc/lib.h"]
    assert (out / "etc" / "lib.h").read_text(encoding="utf-8") == "#define LIB 1\n"
    # the repair can put back one hunk at a time, and a line-0 residue item licenses a file the agent removed
    from hlsfactory_agent.translate import _fenced_hunks, _hunk_key

    keys = lambda: {_hunk_key(r, o) for r, _, _, o in _fenced_hunks(out, pre, CATAPULT)}
    (out / "mac.cpp").write_text(kernel.replace('#include "ac_channel.h"\n', "").replace("typedef", "typedef  "), encoding="utf-8")
    (out / "etc" / "lib.h").unlink()
    ks = keys()
    assert len(ks) == 3 and ("etc/lib.h", None, None) in ks
    inc = next(k for k in ks if k[0] == "mac.cpp" and k[1] == kernel.splitlines().index('#include "ac_channel.h"'))
    assert list(restore_fenced(out, pre, CATAPULT, only={inc})) == ["mac.cpp"]
    assert keys() == ks - {inc} and '#include "ac_channel.h"' in (out / "mac.cpp").read_text(encoding="utf-8")
    log = json.loads((pre / "rewrite_log.json").read_text(encoding="utf-8"))
    log["residue"].append({"file": "etc/lib.h", "line": 0, "reason": "licensed"})
    (pre / "rewrite_log.json").write_text(json.dumps(log), encoding="utf-8")
    assert {k[0] for k in keys()} == {"mac.cpp"}


def test_prepass_resolves_flattened_includes_and_drops_host_libraries(tmp_path: Path):
    from hlsfactory_agent.translate import parse_compile_errors

    d = tmp_path / "design"
    d.mkdir()
    (d / "pe.h").write_text("#define PE 1\n", encoding="utf-8")  # extraction flattened ../include/pe.h next to the source
    (d / "k.cpp").write_text('#include "../include/pe.h"\n#include <gmp.h>\n#include <ap_axi_sdata.h>\nvoid top(int &o) { o = PE; }\n', encoding="utf-8")
    rewrite_design(d, tmp_path / "out", CATAPULT, top="top")
    k = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert '#include "pe.h"\n// #include <gmp.h>  // host-only library, not synthesized\n#include <ap_axi_sdata.h>\n' in k
    assert "struct ap_axis" in (tmp_path / "out" / "ap_axi_sdata.h").read_text(encoding="utf-8")  # the shim makes the include resolve
    # a missing header is a fatal error; it is licensed like any other compile error
    assert [e["line"] for e in parse_compile_errors("k.cpp:10:10: fatal error: 'x.h' file not found\nk.cpp:12:3: error: no\n")] == [10, 12]


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
    (failed / "catapult.log").write_text("# Failed Loop: II=4  Usable clock=4.0000ns  total feedback in loop=3Cy  Maximum allowed budget=48.0000ns  "
                                         "Sequential Delay of loop(ns)=55.9754ns  Delay exceeds budget by=7.9754ns (SCHD-85)\n", encoding="utf-8")
    assert catapult_degrade_step(failed, tmp_path / "d1b") == "relax_ii:5"  # ceil(4 * 55.98 / 48)
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
    (failed / "k.cpp").write_text("static int data_rev[4][256];\n", encoding="utf-8")  # a 2-D array interleaves like any other
    assert catapult_degrade_step(failed, tmp_path / "d2c") == "interleave:data_rev:2"
    (failed / "catapult.log").write_text(  # a scoped name: the array is the last segment, not the class
        "# Error: insufficient resources 'x' to schedule '/T'. 2 are needed, but only 1 instances are available (SCHD-4)\n"
        "# Error: please merge accesses to 'Align::Fixed::ChunkCompute:dp_mem.data:rsc' to reduce the number of required resources (SCHD-39)\n",
        encoding="utf-8",
    )
    assert catapult_degrade_step(failed, tmp_path / "d2s") == "interleave:dp_mem:2"
    (failed / "catapult.log").write_text(
        "# Error: insufficient resources 'x' to schedule '/T'. 4 are needed, but only 2 instances are available (SCHD-4)\n"
        "# Error: please merge accesses to 'stage:data_rev._r.d:rsc' to reduce the number of required resources (SCHD-39)\n",
        encoding="utf-8",
    )
    # already split and still short: the pipelined loop that indexes it gets the longer II instead
    shutil.copytree(tmp_path / "d2c", tmp_path / "d2d_in", ignore=shutil.ignore_patterns("k.c"))
    (tmp_path / "d2d_in" / "k.cpp").write_text("#pragma hls_pipeline_init_interval 1\nfor (int i = 0; i < 8; i++) { s += data_rev[i][0]; }\n"
                                               "#pragma hls_pipeline_init_interval 1\nfor (;;) { t++; }\n", encoding="utf-8")
    (tmp_path / "d2d_in" / "catapult.log").write_text((failed / "catapult.log").read_text(encoding="utf-8"), encoding="utf-8")
    assert catapult_degrade_step(tmp_path / "d2d_in", tmp_path / "d2d") == "relax_ii_for:data_rev:x2:1"
    assert (tmp_path / "d2d" / "k.cpp").read_text(encoding="utf-8").count("init_interval 2") == 1
    (failed / "run.tcl").write_text("go assembly\ngo architect\n", encoding="utf-8")
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


def test_bambu_target_keeps_float_and_uses_its_own_pragma_forms(tmp_path: Path):
    from hlsfactory_agent.tools import bambu_installation
    from hlsfactory_agent.targets.bambu import BAMBU, parse_bambu_report, render_bambu_driver

    assert bambu_installation(tmp_path / "nowhere") is None
    (tmp_path / "app" / "usr" / "bin").mkdir(parents=True)
    (tmp_path / "app" / "AppRun").touch()
    (tmp_path / "app" / "usr" / "bin" / "bambu").touch()
    inst = bambu_installation(tmp_path / "app")
    assert inst.volumes[str(tmp_path / "app")]["mode"] == "ro" and inst.environment["BAMBU"] == "/opt/panda/bambu/AppRun"
    assert "--top-fname=mac" in render_bambu_driver("mac", ["mac.cpp"]) and "-lm" in render_bambu_driver("mac", ["mac.cpp"])

    src = tmp_path / "in"
    src.mkdir()
    (src / "k.cpp").write_text(
        "#include <ap_int.h>\n#include <complex>\n#include <math.h>\n"
        "void k(ap_int<8> a[4], float *o) {\n    std::complex<float> z(1.0f, 2.0f);\n"
        "    for (int i = 0; i < 4; i++) {\n#pragma HLS UNROLL factor=2\n#pragma HLS PIPELINE II=1\n        o[i] = cosf(a[i]) + fabs(z.real());\n    }\n}\n",
        encoding="utf-8",
    )
    (src / "testbench.cpp").write_text("int main() { return 0; }\n", encoding="utf-8")
    log = rewrite_design(src, tmp_path / "out", BAMBU, "k")
    text = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "#pragma HLS unroll factor=2\n    for" in text and "pipeline" not in text.lower()
    assert "ac_complex<float>" in text and "float *o" in text and "cosf(" in text and "cat_abs" not in text
    assert [d["kind"] for d in log["pragmas_dropped"]] == ["pipeline"] and not log["top_marker"]["inserted"] and not log["residue"]

    (src / "k.h").write_text("#include <ap_int.h>\ntypedef ap_int<13> dout_t;\ndout_t k(ap_int<8> A[4], int w);\n", encoding="utf-8")
    (src / "k.cpp").write_text(
        "#include \"k.h\"\ndout_t k(ap_int<8> A[4], int w) {\n    dout_t acc = 0;\n    L: for (int i = 0; i < 4; i++) {\n#pragma HLS UNROLL\n        if (i < w) return acc;\n        acc += A[i];\n    }\n    return acc + 1;\n}\n",
        encoding="utf-8",
    )
    (src / "testbench.cpp").write_text("#include \"k.h\"\nint main() {\n    ap_int<8> a[4];\n    dout_t r = k(a, 2);\n    return r != 3;\n}\n", encoding="utf-8")
    log = rewrite_design(src, tmp_path / "out2", BAMBU, "k")
    text = (tmp_path / "out2" / "k.cpp").read_text(encoding="utf-8")
    assert "void k(ac_int<8, true> A[4], int w, dout_t &ret_out) {" in text and "{ ret_out = acc + 1; return; }" in text and "{ ret_out = acc; return; }" in text
    assert "    L:\n    #pragma HLS unroll\n    for (int i" in text  # Bambu wants the loop right after the pragma, the label above
    assert "void k(ac_int<8, true> A[4], int w, dout_t &ret_out);" in (tmp_path / "out2" / "k.h").read_text(encoding="utf-8")
    assert [r["line"] for r in log["residue"] if r["file"] == "testbench.cpp"] == [4]

    (tmp_path / "synth_stdout.log").write_text("mac.cpp:4:13: error: Missing interface mode attribute\nerror -> Front-end compiler returns an error during compilation 2\n", encoding="utf-8")
    r = parse_bambu_report(tmp_path)
    assert (r.status, r.failure_class) == ("FAILED", "missing-interface") and r.first_error.startswith("mac.cpp:4:13")
    (tmp_path / "synth_stdout.log").write_text("    Total estimated area: 1421\n    Estimated number of DSPs: 8\n  Total number of flip-flops in function _Z3mac: 871\n", encoding="utf-8")
    assert parse_bambu_report(tmp_path).status == "FAILED"  # no RTL written
    (tmp_path / "mac.v").touch()
    r = parse_bambu_report(tmp_path)
    assert (r.status, r.area, r.resources) == ("PASS", 1421.0, {"DSP": 8, "FF": 871})

    from hlsfactory_agent.targets.bambu import bambu_degrade_step

    (tmp_path / "run_bambu.sh").write_text(render_bambu_driver("mac", ["mac.cpp"]), encoding="utf-8")
    assert bambu_degrade_step(tmp_path, tmp_path / "d1") is None
    (tmp_path / "synth_stdout.log").write_text("    Parameter @500069 a\nerror -> unexpected pattern\n", encoding="utf-8")
    assert bambu_degrade_step(tmp_path, tmp_path / "d1") == "interface:minimal"
    assert "--generate-interface=MINIMAL" in (tmp_path / "d1" / "run_bambu.sh").read_text(encoding="utf-8") and not (tmp_path / "d1" / "mac.v").exists()


def test_retry_stats_and_stage_timer():
    from hlsfactory_agent.stats import StageTimer, retry_stats

    t = StageTimer()
    with t("oracle"):
        pass
    assert set(t.seconds) == {"oracle"}
    rows = [
        {"design": "a", "attempts": [{"attempt": 1, "passed": False, "failures": ["3 edit(s) outside the residue list"]}, {"attempt": 2, "passed": True, "failures": []}]},
        {"design": "b", "attempts": [{"attempt": 1, "passed": False, "failures": ["1 line(s) still use the source tool's dialect"]}, {"attempt": 2, "passed": False, "failures": ["1 line(s) still use the source tool's dialect"]}]},
        {"design": "c", "attempts": [{"attempt": 1, "passed": True, "failures": []}]},
    ]
    rs = retry_stats(rows)
    assert rs["first_pass"] == {1: 1, 2: 1} and rs["never"] == ["b"]
    assert [x["design"] for x in rs["thrashing"]] == ["b"] and rs["thrashing"][0]["category"] == "dialect"
    assert rs["transitions"] == {"dialect->same": 1, "fence->pass": 1}


def test_dynamatic_target_rewrites_to_c_types_and_parses_its_cosim_report(tmp_path: Path):
    from hlsfactory_agent.tools import dynamatic_installation
    from hlsfactory_agent.targets.dynamatic import DYNAMATIC, dynamatic_degrade_step, parse_dynamatic_report, render_dynamatic_driver
    from hlsfactory_agent.translate import compile_commands, feasibility

    assert dynamatic_installation("hlsfactory/no-such-image:none") is None
    drv = render_dynamatic_driver("atax", ["atax.c"])
    assert "set-src /work/atax.c" in drv and "simulate --simulator ghdl" in drv and "write-hdl --hdl vhdl" in drv
    assert "compile --buffer-algorithm fpga20 --milp-solver cbc" in drv
    assert compile_commands(DYNAMATIC) == ("clang -std=c17", "clang++ -std=c++17 -x c++")
    assert not DYNAMATIC.fence and "testbench.cpp" not in DYNAMATIC.required_files

    src = tmp_path / "in"
    src.mkdir()
    (src / "k.cpp").write_text(
        "#include <ap_int.h>\n#include \"hls_math.h\"\nvoid k(ap_int<32> a[8], ap_uint<12> b[8]) {\n"
        "  for (int i = 0; i < 8; i++) {\n#pragma HLS PIPELINE II=1\n#pragma HLS UNROLL factor=2\n    b[i] = a[i].range(11, 0);\n  }\n}\n",
        encoding="utf-8",
    )
    (src / "testbench.cpp").write_text("int main() { printf(\"%d\\n\", 1); std::cout << 2; fflush(stdout); return 0; }\n", encoding="utf-8")
    (src / "synth.tcl").write_text("set_top k\n", encoding="utf-8")
    log = rewrite_design(src, tmp_path / "out", DYNAMATIC, "k")
    tb = (tmp_path / "out" / "testbench.cpp").read_text(encoding="utf-8")
    assert "fprintf(stderr, \"%d" in tb and "std::cerr << 2" in tb and "fflush(stderr)" in tb and len(log["stdout"]) == 1
    text = (tmp_path / "out" / "k.cpp").read_text(encoding="utf-8")
    assert "int32_t a[8]" in text and "unsigned _BitInt(12) b[8]" in text and "#include <stdint.h>" in text and "<math.h>" in text
    assert "#pragma clang loop unroll_count(2)" in text and "PIPELINE" not in text and "ac_int" not in text
    assert any("pipeline" == e["kind"] for e in log["pragmas_dropped"]) and any(".range" in r["reason"] for r in log["residue"])
    scan = {"types": [{"kind": "hls::stream", "file": "k.cpp", "line": 1}, {"kind": "ap_fixed", "file": "testbench.cpp", "line": 1}]}
    assert [b["kind"] for b in feasibility(scan, DYNAMATIC)] == ["hls::stream"]
    (src / "k.cpp").write_text("void k(const unsigned char in[], int out[8], int n) {}\n", encoding="utf-8")
    (src / "testbench.cpp").write_text("#define N 16\nint main() { unsigned char a[N]; int o[8]; k(a, o, N); return 0; }\n", encoding="utf-8")
    rewrite_design(src, tmp_path / "out3", DYNAMATIC, "k")
    text = (tmp_path / "out3" / "k.cpp").read_text(encoding="utf-8")
    assert "in[16]" in text, text  # sized from the testbench call, as for Catapult
    assert check_translated_design(tmp_path / "out3", DYNAMATIC)["leftovers"] == []
    (tmp_path / "out3" / "k.cpp").write_text("void k(const unsigned char in[], int out[8], int n) {}\n", encoding="utf-8")
    assert any("in[]" in l["text"] for l in check_translated_design(tmp_path / "out3", DYNAMATIC)["leftovers"])
    kc = tmp_path / "out3" / "k.c"
    kc.write_text("void k(int a[4]) {}\nint main() { int a[4]; CALL_KERNEL(k, a); CALL_KERNEL(k, a); return 0; }\n", encoding="utf-8")
    assert any("CALL_KERNEL appears 2 times" in f for f in check_translated_design(tmp_path / "out3", DYNAMATIC)["failures"])
    kc.write_text("void k(int a[4]) {}\nint main() { int a[4]; // CALL_KERNEL(k, a);\n  CALL_KERNEL(kk, a); return 0; }\n", encoding="utf-8")
    assert any("named after the kernel" in f for f in check_translated_design(tmp_path / "out3", DYNAMATIC)["failures"])
    kc.write_text("void k(int a[4]) {}\nint main() { int a[4]; CALL_KERNEL(k, a); return 0; }\n", encoding="utf-8")
    assert check_translated_design(tmp_path / "out3", DYNAMATIC)["target_check_failures"] == []
    kc.write_text("#include <stdio.h>\nvoid k(int a[4]) {}\nint main() { int a[4]; CALL_KERNEL(k, a); printf(\"ok\"); fprintf(stderr, \"ok\"); return 0; }\n", encoding="utf-8")
    lo = check_translated_design(tmp_path / "out3", DYNAMATIC)["leftovers"]
    assert [l["text"] for l in lo if "printf" in l["text"]] and all("stdio.h" not in l["text"] for l in lo)  # stdout forbidden; the file includes stdio.h

    (tmp_path / "synth_stdout.log").write_text("[INFO] Placed simple buffers\n[FATAL] Failed to convert to std dialect\n", encoding="utf-8")
    r = parse_dynamatic_report(tmp_path)
    assert r.status == "FAILED" and r.failure_class == "unsupported-construct" and "std dialect" in r.first_error
    (tmp_path / "out" / "hdl").mkdir(parents=True)
    (tmp_path / "out" / "hdl" / "k.vhd").touch()
    (tmp_path / "out" / "sim").mkdir()
    (tmp_path / "synth_stdout.log").write_text("[INFO] HDL generation succeeded\n[FATAL] Simulation failed\n", encoding="utf-8")
    (tmp_path / "out" / "sim" / "report.txt").write_text("Comparison of [b] : Fail\n[ERROR HLS_VERIFIER] C and VHDL outputs do not match\n", encoding="utf-8")
    assert parse_dynamatic_report(tmp_path).failure_class == "cosim-mismatch"
    (tmp_path / "out" / "sim" / "report.txt").write_text(
        "tb_k.vhd:307:9:@24018ns:(assertion note): Simulation done! Latency = 6002 cycles\n[INFO  HLS_VERIFIER] C and VHDL outputs match\n", encoding="utf-8")
    r = parse_dynamatic_report(tmp_path)
    assert r.status == "PASS" and r.latency == 6002
    (tmp_path / "run_dynamatic.dyn").write_text(drv, encoding="utf-8")
    assert dynamatic_degrade_step(tmp_path, tmp_path / "d1") is None  # the log never named the LSQ
    (tmp_path / "synth_stdout.log").write_text("[FATAL] Failed to place smart buffers\n", encoding="utf-8")
    assert dynamatic_degrade_step(tmp_path, tmp_path / "d0") == "buffers:on-merges"
    assert "compile --buffer-algorithm on-merges --milp-solver cbc" in (tmp_path / "d0" / "run_dynamatic.dyn").read_text(encoding="utf-8")
    assert dynamatic_degrade_step(tmp_path / "d0", tmp_path / "d00") is None  # already on-merges, no LSQ in the log
    (tmp_path / "synth_stdout.log").write_text("[FATAL] Failed to lower to HW: LSQ generation failed\n", encoding="utf-8")
    assert dynamatic_degrade_step(tmp_path, tmp_path / "d1") == "lsq:off"
    assert "compile --disable-lsq --buffer-algorithm" in (tmp_path / "d1" / "run_dynamatic.dyn").read_text(encoding="utf-8") and not (tmp_path / "d1" / "out").exists()
